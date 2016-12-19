/*
* Copyright (C) 2016, Fabien Aulaire
* All rights reserved.
*/

#include <ScopedExit.h>

#include <Engine_OrderContainer.h>
#include <Engine_Deal.h>
#include <Engine_Tools.h>

#include <iomanip>

namespace exchange
{
    namespace engine
    {

        template <typename TOrder, typename TEventHandler>
        void OrderContainer<TOrder, TEventHandler>::CancelAllOrders()
        {
            auto DoCancelAllOrders = [this](auto & container)
            {
                while (!container.empty())
                {
                    auto OrderIt = container.begin();
                    m_EventHandler.OnUnsolicitedCancelledOrder(*OrderIt);
                    container.erase(OrderIt);
                }
            };

            DoCancelAllOrders(m_AskOrders);
            DoCancelAllOrders(m_BidOrders);
        }

        template <typename TOrder, typename SortingPredicate>
        struct ExecutableQtyPredHelper
        {
            typedef std::greater_equal<typename TOrder::price_type>    value;
        };

        template <typename TOrder>
        struct ExecutableQtyPredHelper<TOrder, std::less<typename TOrder::price_type> >
        {
            typedef std::less_equal<typename TOrder::price_type>       value;
        };

        template <typename TOrder, typename TEventHandler>
        template <typename Container>
        typename OrderContainer<TOrder, TEventHandler>::volume_type
        OrderContainer<TOrder, TEventHandler>::GetExecutableQuantity(const Container & Orders, price_type iPrice, volume_type iMaxVolume) const
        {
            const auto & Index = bmi::get<price_tag>(Orders);
            volume_type Qty = 0_volume;

            using SortingPredicate = decltype(Index.key_comp());
            using Predicate        = typename ExecutableQtyPredHelper<TOrder, SortingPredicate>::value;

            for (const auto & order : Index)
            {
                if (Predicate()(order->GetPrice(), iPrice))
                {
                    Qty += order->GetOpenQuantity();
                    if ( Qty >= iMaxVolume )
                    {
                        // An order can't be over executed, we can stop
                        return iMaxVolume;
                    }
                }
                else
                {
                    // Because index is sorted if the Predicate doesn't match we can stop
                    return Qty;
                }
            }
            return Qty;
        }

        template <typename TOrder, typename TEventHandler>
        template <typename Msg>
        typename OrderContainer<TOrder, TEventHandler>::volume_type
        OrderContainer<TOrder, TEventHandler>::GetExecutableQuantity(const Msg & ipMsg) const
        {
            switch (ipMsg->GetWay())
            {
                case OrderWay::BUY:
                {
                    return GetExecutableQuantity(m_AskOrders, ipMsg->GetPrice(), (volume_type)ipMsg->GetQuantity());

                }
                case OrderWay::SELL:
                {
                    return GetExecutableQuantity(m_BidOrders, ipMsg->GetPrice(), (volume_type)ipMsg->GetQuantity());
                }
                default:
                    assert(false && "Invalid order way");
                    return 0_volume;
            }
        }

        template <typename Msg, bool bIsReplaceOrder>
        struct AggressorIDHelper
        {
            // TODO : Add static assert to verify that Msg is a pointer type ( smart or raw pointer )
            static auto get(const Msg & t) { return t->GetOrderID(); }
        };

        template <typename Msg>
        struct AggressorIDHelper<Msg, true>
        {
            // TODO : Add static assert to verify that Msg is a pointer type ( smart or raw pointer )
            static auto get(const Msg & t) { return t->GetReplacedOrderID(); }
        };

        template <typename Msg>
        auto GetAggressorID(const Msg & t)
        {
            return AggressorIDHelper<Msg, is_replaced_order<Msg>::value >::get(t);
        }

        template <typename TOrder, typename TEventHandler>
        template <typename Container, typename Msg>
        void OrderContainer<TOrder, TEventHandler>::ProcessDeals(Container & Orders, Msg & iMsg, volume_type iMatchQty)
        {
            auto & Index = bmi::get<price_tag>(Orders);

            while (iMatchQty > 0_volume)
            {
                auto OrderToHitIt  = Index.begin();

                assert(OrderToHitIt != Index.end());

                auto * OrderToHit  = *(OrderToHitIt);

                // Get the exec price and the exec quantity
                auto ExecQty = (std::min)(OrderToHit->GetOpenQuantity(), iMsg->GetOpenQuantity());
                auto ExecPrice = OrderToHit->GetPrice();

                // Update quantity of both orders
                iMsg->AddExecutedQuantity(ExecQty);
                // TODO : Why the pointer on the order is not used ?
                Index.modify(OrderToHitIt, ExecQuantityUpdater<OrderPtrType>(ExecQty));

                // Decrease the matching quantity
                iMatchQty -= ExecQty;

                // Generate the deal
                typename TEventHandler::deal_ptr_type pDeal = nullptr;

                if (OrderToHit->GetWay() == OrderWay::BUY)
                {
                    pDeal = m_EventHandler.CreateDeal(ExecPrice, ExecQty, OrderToHit->GetClientID(), OrderToHit->GetOrderID(), iMsg->GetClientID(), GetAggressorID(iMsg));
                }
                else
                {
                    pDeal = m_EventHandler.CreateDeal(ExecPrice, ExecQty, iMsg->GetClientID(), GetAggressorID(iMsg), OrderToHit->GetClientID(), OrderToHit->GetOrderID());
                }

                m_EventHandler.OnDeal(std::move(pDeal));

                if (0_qty == OrderToHit->GetOpenQuantity())
                {
                    Index.erase(OrderToHitIt);
                }
            }
        }

        template <typename TOrder, typename TEventHandler>
        template <typename Msg>
        void OrderContainer<TOrder, TEventHandler>::ProcessDeals(Msg & iMsg, OrderWay iWay, volume_type iMatchQty)
        {
            switch (iWay)
            {
                case OrderWay::BUY:
                    ProcessDeals(m_AskOrders, iMsg, iMatchQty);
                    break;
                case OrderWay::SELL:
                    ProcessDeals(m_BidOrders, iMsg, iMatchQty);
                    break;
                default:
                    assert(false);
                    return;
            }
        }

        template <typename TOrder, typename TEventHandler>
        Status OrderContainer<TOrder, TEventHandler>::Insert(std::unique_ptr<TOrder> ipOrder, bool Match)
        {
            auto insert_at_exit = common::make_scope_exit([this,&ipOrder]()
                {
                    m_OrderContainer.emplace(ipOrder->GetClientID(), std::move(ipOrder));
                }
            );

            if (Match)
            {
                volume_type MatchQty = GetExecutableQuantity(ipOrder);

                if (MatchQty != 0_volume)
                {
                    ProcessDeals(ipOrder, ipOrder->GetWay(), MatchQty);
                }
            }

            if (ipOrder->GetOpenQuantity() != 0_qty)
            {
                if (AuctionInsert(ipOrder.get()) == false)
                {
                    return Status::InternalError;;
                }
            }

            return Status::Ok;;
        }

        template <typename TOrder, typename TEventHandler>
        bool OrderContainer<TOrder, TEventHandler>::AuctionInsert(TOrder * ipOrder)
        {
            assert(ipOrder && "Input order is null!");
            /*
            *  insert return a pair, the second element indicate
            *  if the insertion fail or not.
            */
            switch (ipOrder->GetWay())
            {
                case OrderWay::BUY:
                    return m_BidOrders.insert( ipOrder ).second;
                case OrderWay::SELL:
                    return m_AskOrders.insert( ipOrder ).second;
                default:
                    assert(false);
                    return false;
            };
        }

        /**
        * Erase an order from order book
        */
        template <typename TOrder, typename TEventHandler>
        Status OrderContainer<TOrder, TEventHandler>::Delete(const client_orderid_type iOrderId, const client_id_type iClientId, OrderWay iWay)
        {    
            bool Result = false;

            switch (iWay)
            {
                case OrderWay::BUY:
                    Result = (1 == m_BidOrders.erase(OrderIDGenerator<TOrder>()(iClientId, iOrderId)));
                    break;
                case OrderWay::SELL:
                    Result = (1 == m_AskOrders.erase(OrderIDGenerator<TOrder>()(iClientId, iOrderId)));
                    break;
                default:
                    assert(false);
                    return Status::InternalError;
            }
            return Result ? Status::Ok : Status::OrderNotFound;
        }

        template <typename TOrder, typename TEventHandler>
        template <typename TOrderReplace>
        Status OrderContainer<TOrder, TEventHandler>::Modify(std::unique_ptr<TOrderReplace> iOrderReplace, bool Match)
        {
            auto ProcessModify = [&](TOrder * iOrder)
            {
                if (Match)
                {
                    volume_type MatchQty = GetExecutableQuantity(iOrder);

                    if (MatchQty != 0_volume)
                    {
                        ProcessDeals(iOrder, iOrder->GetWay(), MatchQty);

                        if (0_qty == iOrder->GetOpenQuantity())
                        {
                            return false;
                        }
                    }
                }
                return true;
            };

            auto OrderID    = OrderIDGenerator<TOrder>()(iOrderReplace->GetClientID(), iOrderReplace->GetExistingOrderID());

            auto ApplyModify = [&](auto & Container)
            {
                auto Order = Container.find(OrderID);
                if (Order != Container.end())
                {
                    auto OrderPtr = const_cast<TOrder*>( *(Order) );

                    const auto min_new_qty = OrderPtr->GetCancelledQuantity() + OrderPtr->GetExecutedQuantity();

                    if( iOrderReplace->GetQuantity() <= min_new_qty )
                    {
                        return Status::InvalidQuantity;
                    }

                    Container.erase(Order);

                    OrderPtr->SetQuantity(iOrderReplace->GetQuantity() );
                    OrderPtr->SetPrice(iOrderReplace->GetPrice());
                    OrderPtr->SetOrderID(iOrderReplace->GetReplacedOrderID());

                    if (ProcessModify(OrderPtr))
                    {
                        // Order is not fully filled, re-queued the rest of the quantity
                        Container.insert(OrderPtr);
                    }
                    
                    return Status::Ok;
                }
                return Status::OrderNotFound;
            };

            switch (iOrderReplace->GetWay())
            {
                case OrderWay::BUY:
                    return ApplyModify(m_BidOrders);
                case OrderWay::SELL:
                    return ApplyModify(m_AskOrders);
                default:
                    assert(false);
                    return Status::InternalError;
            }
        }

        /*
            ENH_TODO : Check if the following rule is applied
            Where there are two maximum executable volumes (for two different prices), the following rules are applied:
            the opening price cannot be higher than the best ask or lower than the best bid immediately after the auction phase.
            This rules is clearly not applied, but because the matching engine does not support market order
            for now, this rules is not necessary
        */
        template <typename TOrder, typename TEventHandler>
        typename OrderContainer<TOrder, TEventHandler>::OpenInformationType
        OrderContainer<TOrder, TEventHandler>::GetTheoriticalAuctionInformations() const
        {
            volume_type MaxQty = 0_volume;
            Price  OpenPrice = 0_price;

            for (auto & order : m_AskOrders)
            {
                volume_type BidQty = GetExecutableQuantity(m_BidOrders, order->GetPrice(), volume_type::max());
                volume_type AskQty = GetExecutableQuantity(m_AskOrders, order->GetPrice(), volume_type::max());

                volume_type CurrentQty = (std::min)(BidQty, AskQty);

                if (CurrentQty > MaxQty)
                {
                    MaxQty = CurrentQty;
                    OpenPrice = order->GetPrice();
                }
            }
            return std::make_tuple(OpenPrice, MaxQty);
        }


        /*
        Post auction phase
        */
        template <typename TOrder, typename TEventHandler>
        void  OrderContainer<TOrder, TEventHandler>::MatchOrders()
        {
            auto OpeningInformation = GetTheoriticalAuctionInformations();

            price_type   MatchingPrice( std::get<0>(OpeningInformation) );
            volume_type  MatchingQty(std::get<1>(OpeningInformation));

            bid_index_type & BidIndex = GetBidIndex();
            ask_index_type & AskIndex = GetAskIndex();

            while (MatchingQty > 0_volume)
            {
                auto   BidOrderIt = BidIndex.begin();
                auto   BidOrder   = *(BidOrderIt);

                while (BidOrder->GetOpenQuantity() > 0_qty && MatchingQty > 0_volume)
                {
                    auto   AskOrderIt = AskIndex.begin();
                    auto   AskOrder   = *(AskOrderIt);

                    auto ExecutedQty = (std::min)(AskOrder->GetOpenQuantity(), BidOrder->GetOpenQuantity());

                    AskIndex.modify(AskOrderIt, ExecQuantityUpdater<OrderPtrType>(ExecutedQty));
                    BidIndex.modify(BidOrderIt, ExecQuantityUpdater<OrderPtrType>(ExecutedQty));

                    auto pDeal = m_EventHandler.CreateDeal(MatchingPrice, ExecutedQty, BidOrder->GetClientID(), BidOrder->GetOrderID(), AskOrder->GetClientID(), AskOrder->GetOrderID());

                    m_EventHandler.OnDeal(std::move(pDeal));

                    MatchingQty -= ExecutedQty;

                    if (0_qty == AskOrder->GetOpenQuantity())
                    {
                        AskIndex.erase(AskOrderIt);
                    }
                }

                if (0_qty == BidOrder->GetOpenQuantity())
                {
                    GetBidIndex().erase(BidOrderIt);
                }
            }
        }

        template <typename TOrder, typename TEventHandler>
        void OrderContainer<TOrder, TEventHandler>::RehashIndexes(size_t size)
        {
            bmi::get<order_id_tag>(m_BidOrders).rehash(size);
            bmi::get<order_id_tag>(m_AskOrders).rehash(size);
        }

        template <typename TOrder, typename TEventHandler>
        void OrderContainer<TOrder, TEventHandler>::ByOrderView(std::vector<TOrder*> & BidContainer, std::vector<TOrder*> & AskContainer) const
        {
            BidContainer.reserve( GetBidIndex().size() );
            AskContainer.reserve( GetAskIndex().size() );

            std::copy(GetAskIndex().begin(), GetAskIndex().end(), std::back_inserter(AskContainer));
            std::copy(GetBidIndex().begin(), GetBidIndex().end(), std::back_inserter(BidContainer));
        }

        template <typename TOrder, typename TEventHandler>
        void OrderContainer<TOrder, TEventHandler>::AggregatedView(LimitContainer & BidContainer, LimitContainer & AskContainer) const
        {
            auto AggregatedViewHelper = [&](price_index_iterator begin, price_index_iterator end, LimitContainer & Container)
            { 
                price_type CurrentPrice = std::numeric_limits<price_type>::max();
                size_t     ContainerIndex = 0;

                for (; begin != end; begin++)
                {
                    price_type Price = (*begin)->GetPrice();
                    if (Price != CurrentPrice)
                    {
                        ContainerIndex++;
                        CurrentPrice = Price;
                        Container.emplace_back(0, 0_qty, Price);
                    }

                    LimitType & Limit = Container[ContainerIndex - 1];
                    std::get<0>(Limit)++;
                    std::get<1>(Limit) += (*begin)->GetOpenQuantity();
                }
            };

            AggregatedViewHelper(GetBidIndex().begin(), GetBidIndex().end(), BidContainer);
            AggregatedViewHelper(GetAskIndex().begin(), GetAskIndex().end(), AskContainer);
        }

        template <typename TOrder, typename TEventHandler>
        std::ostream& operator<<(std::ostream& oss, const OrderContainer<TOrder, TEventHandler> & iOrders)
        {
            switch (iOrders.GetViewMode())
            {
                case OrderContainer<TOrder, TEventHandler>::ViewMode::VM_BY_ORDER:
                    iOrders.StreamByOrder(oss);
                    break;
                case OrderContainer<TOrder, TEventHandler>::ViewMode::VM_BY_PRICE:
                    iOrders.StreamByPrice(oss);
                    break;
                case OrderContainer<TOrder, TEventHandler>::ViewMode::VM_UNKNOWN:
                    assert(false);
                    break;
            };
            return oss;
        }

        template <typename TOrder, typename TEventHandler>
        void OrderContainer<TOrder, TEventHandler>::StreamByOrder(std::ostream& oss) const
        {
            auto MaxIndex = (std::max)(m_BidOrders.size(), m_AskOrders.size());

            auto MakeString = [](qty_type Qty, price_type Price)
            {
                std::ostringstream oss("");
                oss << Qty << "@" << Price;
                return oss.str();
            };

            auto StreamEntry = [&](price_index_iterator & Entry, price_index_iterator end, const std::string & end_string)
            {
                if (Entry != end)
                {
                    oss << "|" << std::setw(13) << MakeString((*Entry)->GetOpenQuantity(), (*Entry)->GetPrice()) << end_string;
                    Entry++;
                }
                else
                {
                    oss << "|" << std::setw(13) << "0" << end_string;
                }
            };

            oss << "|        BID         |        ASK        |" << std::endl;
            oss << "|                    |                   |" << std::endl;

            auto AskIterator = GetAskIndex().begin();
            auto BidIterator = GetBidIndex().begin();

            for (size_t Index = 0; Index < MaxIndex; Index++)
            {
                StreamEntry(BidIterator, GetBidIndex().end(), "       ");
                StreamEntry(AskIterator, GetAskIndex().end(), "      |");
                oss << std::endl;
            }
        }

        template <typename TOrder, typename TEventHandler>
        void OrderContainer<TOrder, TEventHandler>::StreamByPrice(std::ostream& oss) const
        {
            typedef typename OrderContainer<TOrder, TEventHandler>::LimitContainer LimitContainer;
            typedef typename OrderContainer<TOrder, TEventHandler>::LimitType      LimitType;

            LimitContainer BidContainer;
            LimitContainer AskContainer;

            auto MakeString = [](std::uint32_t NbOrder, qty_type Qty, price_type Price)
            {
                std::ostringstream oss("");
                oss << "  " << NbOrder << "   " << Qty << "@" << Price;
                return oss.str();
            };

            auto StreamEntry = [&](size_t Index, LimitContainer & Container, const std::string & end_string)
            {
                if (Index < Container.size())
                {
                    LimitType & Limit = Container[Index];
                    oss << "|" << std::setw(15) << MakeString(std::get<0>(Limit), std::get<1>(Limit), std::get<2>(Limit)) << end_string;
                }
                else
                {
                    oss << "|" << std::setw(15) << "0" << end_string;
                }
            };
            
            AggregatedView(BidContainer, AskContainer);

            auto MaxIndex = (std::max)(BidContainer.size(), AskContainer.size());

            oss << "|         BID          |         ASK         |" << std::endl;
            oss << "|                      |                     |" << std::endl;

            for (size_t Index = 0; Index < MaxIndex; Index++)
            {
                StreamEntry(Index, BidContainer, "       ");
                StreamEntry(Index, AskContainer, "      |");
                oss << std::endl;
            }
        }

    }
}
