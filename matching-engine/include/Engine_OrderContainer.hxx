/*
* Copyright (C) 2013, Fabien Aulaire
* All rights reserved.
*/

#include <Engine_OrderContainer.h>
#include <Engine_Deal.h>
#include <Engine_Tools.h>

#include <iomanip>

namespace exchange
{
    namespace engine
    {

        template <typename TOrder, typename TDealHandler>
        void OrderContainer<TOrder, TDealHandler>::Reset()
        {
            m_AskOrders.clear();
            m_BidOrders.clear();
        }

        template <typename TOrder, typename SortingPredicate>
        struct ExecutableQtyPredHelper
        {
            typedef std::greater_equal<typename TOrder::price_type>    type;
        };

        template <typename TOrder>
        struct ExecutableQtyPredHelper<TOrder, std::less<typename TOrder::price_type> >
        {
            typedef std::less_equal<typename TOrder::price_type>       type;
        };

        template <typename TOrder, typename TDealHandler>
        template <typename Container>
        UInt64 OrderContainer<TOrder, TDealHandler>::GetExecutableQuantity(const Container & Orders, price_type iPrice) const
        {
            auto & Index = bmi::get<price_tag>(Orders);
            UInt64 Qty = 0;

            typedef decltype(Index.key_comp())                                          SortingPredicate;
            typedef typename ExecutableQtyPredHelper<TOrder, SortingPredicate>::type   Predicate;

            for (auto & order : Index)
            {
                if (Predicate()(order.GetPrice(), iPrice))
                {
                    Qty += order.GetQuantity();
                }
            }
            return Qty;
        }

        template <typename TOrder, typename TDealHandler>
        template <typename Msg>
        UInt64 OrderContainer<TOrder, TDealHandler>::GetExecutableQuantity(const Msg & iMsg, OrderWay iWay) const
        {
            switch (iWay)
            {
                case OrderWay::BUY:
                {
                    UInt64 MaxQty = GetExecutableQuantity(m_AskOrders, iMsg.GetPrice());
                    return (std::min)(MaxQty, static_cast<UInt64>(iMsg.GetQuantity()));
                }
                case OrderWay::SELL:
                {
                    UInt64 MaxQty = GetExecutableQuantity(m_BidOrders, iMsg.GetPrice());
                    return (std::min)(MaxQty, static_cast<UInt64>(iMsg.GetQuantity()));
                }
                default:
                    return 0;
            }
        }

        template <typename Msg, bool bIsReplaceOrder>
        struct AggressorIDHelper
        {
            static UInt32 get(Msg & t) { return t.GetOrderID(); }
        };

        template <typename Msg>
        struct AggressorIDHelper<Msg, true>
        {
            static UInt32 get(Msg & t) { return t.GetReplacedOrderID(); }
        };

        template <typename Msg>
        UInt32 GetAggressorID(Msg & t)
        {
            return AggressorIDHelper<Msg, is_replaced_order<Msg>::value >::get(t);
        }

        template <typename TOrder, typename TDealHandler>
        template <typename Container, typename Msg>
        void OrderContainer<TOrder, TDealHandler>::ProcessDeals(Container & Orders, Msg & iMsg, UInt64 iMatchQty)
        {
            auto & Index = bmi::get<price_tag>(Orders);

            while (iMatchQty > 0)
            {
                auto OrderToHit = Index.begin();

                // Get the exec price and the exec quantity
                auto ExecQty = (std::min)(OrderToHit->GetQuantity(), iMsg.GetQuantity());
                auto ExecPrice = (std::min)(OrderToHit->GetPrice(), iMsg.GetPrice());

                // Update quantity of both orders
                iMsg.RemoveQuantity(ExecQty);
                Index.modify(OrderToHit, OrderUpdaterSingle<&Order::SetQuantity>(OrderToHit->GetQuantity() - ExecQty));

                // Decrease the matching quantity
                iMatchQty -= ExecQty;

                // Generate the deal
                Deal * pDeal = nullptr;

                if (OrderToHit->GetWay() == OrderWay::BUY)
                {
                    pDeal = new Deal(ExecPrice, ExecQty, OrderToHit->GetClientID(), OrderToHit->GetOrderID(), iMsg.GetClientID(), GetAggressorID(iMsg));
                }
                else
                {
                    pDeal = new Deal(ExecPrice, ExecQty, iMsg.GetClientID(), GetAggressorID(iMsg), OrderToHit->GetClientID(), OrderToHit->GetOrderID());
                }

                m_DealHandler.OnDeal(pDeal);

                if (0 == OrderToHit->GetQuantity())
                {
                    Index.erase(OrderToHit);
                }
            }
        }

        template <typename TOrder, typename TDealHandler>
        template <typename Msg>
        void OrderContainer<TOrder, TDealHandler>::ProcessDeals(Msg & iMsg, OrderWay iWay, UInt64 iMatchQty)
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
                    return;
            }
        }

        template <typename TOrder, typename TDealHandler>
        bool OrderContainer<TOrder, TDealHandler>::Insert(TOrder & iOrder, bool Match)
        {
            if (Match)
            {
                UInt64 MatchQty = (std::min)(GetExecutableQuantity(iOrder, iOrder.GetWay()), static_cast<UInt64>(iOrder.GetQuantity()));

                if (MatchQty)
                {
                    ProcessDeals(iOrder, iOrder.GetWay(), MatchQty);

                    if (iOrder.GetQuantity())
                    {
                        return AuctionInsert(iOrder);
                    }
                    return true;
                }
                else
                {
                    return AuctionInsert(iOrder);
                }
            }
            else
            {
                return AuctionInsert(iOrder);
            }
        }

        template <typename TOrder, typename TDealHandler>
        bool OrderContainer<TOrder, TDealHandler>::AuctionInsert(const TOrder & iOrder)
        {
            /*
            *  insert return a pair, the second element indicate
            *  if the insertion fail or not.
            */
            switch (iOrder.GetWay())
            {
                case OrderWay::BUY:
                    return m_BidOrders.insert(iOrder).second;
                case OrderWay::SELL:
                    return m_AskOrders.insert(iOrder).second;
                default:
                    return false;
            };
        }

        /**
        * Erase an order from order book
        */
        template <typename TOrder, typename TDealHandler>
        bool OrderContainer<TOrder, TDealHandler>::Delete(const UInt32 iOrderId, const UInt32 iClientId, OrderWay iWay)
        {    
            /*
            Erase return the number of element erased, so if no element
            is erased in Bid container, try on Ask.
            */
            switch (iWay)
            {
                case OrderWay::BUY:
                    return (1 == m_BidOrders.erase(OrderIDGenerator<TOrder>()(iClientId, iOrderId)));
                case OrderWay::SELL:
                    return (1 == m_AskOrders.erase(OrderIDGenerator<TOrder>()(iClientId, iOrderId)));
                default:
                    assert(false);
                    return false;
            };
        }

        template <typename TOrder, typename TDealHandler>
        template <typename TOrderReplace>
        bool OrderContainer<TOrder, TDealHandler>::Modify(TOrderReplace & iOrderReplace, bool Match)
        {
            auto ProcessModify = [&](hashed_index_iterator iOrder, bool bMatch)
            {
                if (bMatch)
                {
                    UInt64 MaxExecQty = GetExecutableQuantity(iOrderReplace, iOrder->GetWay());
                    UInt64 MatchQty = (std::min)(MaxExecQty, static_cast<UInt64>(iOrderReplace.GetQuantity()));

                    if (MatchQty)
                    {
                        ProcessDeals(iOrderReplace, iOrder->GetWay(), MatchQty);

                        if (0 == iOrderReplace.GetQuantity())
                        {
                            return false;
                        }
                    }
                }
                return true;
            };

            auto OrderID = OrderIDGenerator<TOrder>()(iOrderReplace.GetClientID(), iOrderReplace.GetExistingOrderID());

            switch (iOrderReplace.GetWay())
            {
                case OrderWay::BUY:
                    {
                        auto BidOrder = m_BidOrders.find(OrderID);
                        if (BidOrder != m_BidOrders.end())
                        {
                            if (ProcessModify(BidOrder, Match))
                            {
                                OrderUpdater aUpdater(iOrderReplace);
                                return m_BidOrders.modify(BidOrder, aUpdater);
                            }
                            else
                            {
                                return (0 != m_BidOrders.erase(iOrderReplace.GetExistingOrderID()));
                            }
                        }
                    }
                    break;
                case OrderWay::SELL:
                    {
                        auto AskOrder = m_AskOrders.find(OrderID);
                        if (AskOrder != m_AskOrders.end())
                        {
                            if (ProcessModify(AskOrder, Match))
                            {
                                OrderUpdater aUpdater(iOrderReplace);
                                return m_AskOrders.modify(AskOrder, aUpdater);
                            }
                            else
                            {
                                return (0 != m_AskOrders.erase(iOrderReplace.GetExistingOrderID()));
                            }
                        }
                    }
                    break;
                default:
                    return false;
            }
            return false;
        }

        /*
        TODO : Check if the following rule is applied
        Where there are two maximum executable volumes (for two different prices), the following rules are applied:
        the opening price cannot be higher than the best ask or lower than the best bid immediately after the auction phase.
        */
        template <typename TOrder, typename TDealHandler>
        std::tuple<UInt32, UInt64> OrderContainer<TOrder, TDealHandler>::GetTheoriticalAuctionInformations() const
        {
            UInt64 MaxQty = 0;
            UInt32 OpenPrice = 0;

            for (auto & order : m_AskOrders)
            {
                UInt64 BidQty = GetExecutableQuantity(m_BidOrders, order.GetPrice());
                UInt64 AskQty = GetExecutableQuantity(m_AskOrders, order.GetPrice());

                UInt64 CurrentQty = (std::min)(BidQty, AskQty);

                if (CurrentQty > MaxQty)
                {
                    MaxQty = CurrentQty;
                    OpenPrice = order.GetPrice();
                }
            }
            return std::make_tuple(OpenPrice, MaxQty);
        }


        /*
        Post auction phase
        */
        template <typename TOrder, typename TDealHandler>
        void  OrderContainer<TOrder, TDealHandler>::MatchOrders()
        {
            auto OpeningInformation = GetTheoriticalAuctionInformations();

            UInt32 MatchingPrice = std::get<0>(OpeningInformation);
            UInt64 MatchingQty = std::get<1>(OpeningInformation);

            bid_index_type & BidIndex = GetBidIndex();
            ask_index_type & AskIndex = GetAskIndex();

            while (MatchingQty > 0)
            {
                price_index_iterator BidOrder = BidIndex.begin();

                while (BidOrder->GetQuantity() > 0 && MatchingQty > 0)
                {
                    price_index_iterator AskOrder = AskIndex.begin();

                    auto ExecutedQty = (std::min)(AskOrder->GetQuantity(), BidOrder->GetQuantity());

                    AskIndex.modify(AskOrder, OrderUpdaterSingle<&Order::SetQuantity>(AskOrder->GetQuantity() - ExecutedQty));
                    BidIndex.modify(BidOrder, OrderUpdaterSingle<&Order::SetQuantity>(BidOrder->GetQuantity() - ExecutedQty));

                    Deal * pDeal = new Deal(MatchingPrice, ExecutedQty, BidOrder->GetClientID(), BidOrder->GetOrderID(), AskOrder->GetClientID(), AskOrder->GetOrderID());
                    m_DealHandler.OnDeal(pDeal);

                    MatchingQty -= ExecutedQty;

                    if (0 == AskOrder->GetQuantity())
                    {
                        AskIndex.erase(AskOrder);
                    }
                }

                if (0 == BidOrder->GetQuantity())
                {
                    GetBidIndex().erase(BidOrder);
                }
            }
        }

        template <typename TOrder, typename TDealHandler>
        void OrderContainer<TOrder, TDealHandler>::ByOrderView(std::vector<TOrder> & BidContainer, std::vector<TOrder> & AskContainer) const
        {
            BidContainer.reserve( GetBidIndex().size() );
            AskContainer.reserve( GetAskIndex().size() );

            std::copy(GetAskIndex().begin(), GetAskIndex().end(), std::back_inserter(AskContainer));
            std::copy(GetBidIndex().begin(), GetBidIndex().end(), std::back_inserter(BidContainer));
        }

        template <typename TOrder, typename TDealHandler>
        void OrderContainer<TOrder, TDealHandler>::AggregatedView(LimitContainer & BidContainer, LimitContainer & AskContainer) const
        {
            auto AggregatedViewHelper = [&](price_index_iterator begin, price_index_iterator end, LimitContainer & Container)
            { 
                price_type CurrentPrice = std::numeric_limits<price_type>::max();
                size_t     ContainerIndex = 0;

                for (; begin != end; ++begin)
                {
                    price_type Price = begin->GetPrice();
                    if (Price != CurrentPrice)
                    {
                        ++ContainerIndex;
                        CurrentPrice = Price;
                        Container.emplace_back(0, 0, Price);
                    }

                    LimitType & Limit = Container[ContainerIndex - 1];
                    ++std::get<0>(Limit);
                    std::get<1>(Limit) += begin->GetQuantity();
                }
            };

            AggregatedViewHelper(GetBidIndex().begin(), GetBidIndex().end(), BidContainer);
            AggregatedViewHelper(GetAskIndex().begin(), GetAskIndex().end(), AskContainer);
        }

        template <typename TOrder, typename TDealHandler>
        std::ostream& operator<<(std::ostream& oss, const OrderContainer<TOrder, TDealHandler> & iOrders)
        {
            switch (iOrders.GetViewMode())
            {
                case OrderContainer<TOrder, TDealHandler>::ViewMode::VM_BY_ORDER:
                    iOrders.StreamByOrder(oss);
                    break;
                case OrderContainer<TOrder, TDealHandler>::ViewMode::VM_BY_PRICE:
                    iOrders.StreamByPrice(oss);
                    break;
                case OrderContainer<TOrder, TDealHandler>::ViewMode::VM_UNKNOWN:
                    assert(false);
                    break;
            };
            return oss;
        }

        template <typename TOrder, typename TDealHandler>
        void OrderContainer<TOrder, TDealHandler>::StreamByOrder(std::ostream& oss) const
        {
            auto MaxIndex = (std::max)(m_BidOrders.size(), m_AskOrders.size());

            auto MakeString = [](UInt32 Qty, UInt64 Price)
            {
                std::ostringstream oss("");
                oss << Qty << "@" << Price;
                return oss.str();
            };

            auto StreamEntry = [&](price_index_iterator & Entry, price_index_iterator end, const std::string & end_string)
            {
                if (Entry != end)
                {
                    oss << "|" << std::setw(13) << MakeString(Entry->GetQuantity(), Entry->GetPrice()) << end_string;
                    ++Entry;
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

            for (UInt64 Index = 0; Index < MaxIndex; ++Index)
            {
                StreamEntry(BidIterator, GetBidIndex().end(), "       ");
                StreamEntry(AskIterator, GetAskIndex().end(), "      |");
                oss << std::endl;
            }
        }

        template <typename TOrder, typename TDealHandler>
        void OrderContainer<TOrder, TDealHandler>::StreamByPrice(std::ostream& oss) const
        {
            typedef typename OrderContainer<TOrder, TDealHandler>::LimitContainer LimitContainer;
            typedef typename OrderContainer<TOrder, TDealHandler>::LimitType      LimitType;

            LimitContainer BidContainer;
            LimitContainer AskContainer;

            auto MakeString = [](UInt32 NbOrder, UInt32 Qty, UInt64 Price)
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

            for (UInt64 Index = 0; Index < MaxIndex; ++Index)
            {
                StreamEntry(Index, BidContainer, "       ");
                StreamEntry(Index, AskContainer, "      |");
                oss << std::endl;
            }
        }

    }
}
