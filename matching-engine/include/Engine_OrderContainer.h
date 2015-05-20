/*
* Copyright (C) 2015, Fabien Aulaire
* All rights reserved.
*/

#pragma once

#include <logger/Logger.h>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/mem_fun.hpp>

#include <Engine_Status.h>

#include <unordered_set>
#include <memory>

namespace exchange
{
    namespace engine
    {
        namespace bmi = boost::multi_index;

        struct order_id_tag{};
        struct client_id_tag{};
        struct price_tag{};

        template <typename TOrder>
        struct OrderIDGenerator
        {
            using result_type = std::uint64_t;

            result_type get_key(std::uint32_t iClientID, std::uint32_t iOrderID) const
            {
                result_type Hash = (result_type)iClientID << 32;
                Hash += iOrderID;
                return Hash;
            }

            result_type operator()(std::uint32_t iClientID, std::uint32_t iOrderID) const
            {
                return get_key(iClientID, iOrderID);
            }

            result_type operator()(const TOrder & Order) const
            {
                return get_key(Order->GetClientID(), Order->GetOrderID());
            }
        };

        template <typename TOrder, typename SortingPredicate>
        struct OrderStorage
        {

            typedef boost::multi_index_container<
                TOrder*,
                bmi::indexed_by<
                
                    bmi::hashed_unique<
                    bmi::tag<order_id_tag>, OrderIDGenerator<TOrder*> >,

                    bmi::hashed_non_unique<
                    bmi::tag<client_id_tag>, bmi::const_mem_fun<TOrder, std::uint32_t, &TOrder::GetClientID> >,

                    bmi::ordered_non_unique<
                    bmi::tag<price_tag>, bmi::const_mem_fun<TOrder, typename TOrder::price_type, &TOrder::GetPrice>, SortingPredicate>
                >
            > order_set;
        };

        template<typename TOrder, typename TEventHandler> class OrderContainer;
        template<typename TOrder, typename TEventHandler> std::ostream& operator<< (std::ostream& o, const OrderContainer<TOrder, TEventHandler> & x);

        template <typename TOrder, typename TEventHandler>
        class OrderContainer
        {

            friend std::ostream& operator<< <> (std::ostream& o, const OrderContainer<TOrder, TEventHandler> & x);

            public:

                enum class ViewMode
                {
                    VM_BY_PRICE = 0,
                    VM_BY_ORDER,
                    VM_UNKNOWN
                };

            public:

                using price_type          = typename TOrder::price_type;
                using qty_type            = typename TOrder::qty_type;
                using nominal_type        = typename TOrder::nominal_type;
                using volume_type         = typename TOrder::volume_type;

                /* Types used to store the aggregated view of orders */
                /* NbOrder Qty Price*/
                using LimitType           = std::tuple<std::uint32_t, qty_type, price_type>;
                using LimitContainer      = std::vector<LimitType>;

                using OpenInformationType = std::tuple<price_type, volume_type>;

                using OrderIDContainer    = std::unordered_set < typename OrderIDGenerator<TOrder>::result_type >;
                using GlobalOrderContainer = std::unordered_set < std::unique_ptr<TOrder> > ;

            protected:

                using OrderPtrType = TOrder*;

                typedef std::greater<price_type>                                           BidPredicate;
                typedef typename OrderStorage<TOrder, BidPredicate>::order_set             BidStorage;

                typedef std::less<price_type>                                              AskPredicate;
                typedef typename OrderStorage<TOrder, AskPredicate>::order_set             AskStorage;

                typedef typename boost::multi_index::index<BidStorage, price_tag>::type    bid_index_type;
                typedef typename boost::multi_index::index<AskStorage, price_tag>::type    ask_index_type;

                typedef typename bid_index_type::iterator                                  price_index_iterator;
                typedef typename BidStorage::iterator                                      hashed_index_iterator;

            public:

                /**
                */
                OrderContainer(TEventHandler & iEventHandler):
                    m_EventHandler(iEventHandler), m_ViewMode(ViewMode::VM_BY_ORDER)
                {}

                /**
                */
                Status Insert(std::unique_ptr<TOrder> ipOrder, bool Match = false);

                /**
                */
                template <typename TOrderReplace>
                Status Modify(std::unique_ptr<TOrderReplace> iOrderReplace, bool Match = false);

                /**
                */
                Status Delete(const std::uint32_t iOrderId, const std::uint32_t ClientId, OrderWay iWay);

                /**
                */
                void MatchOrders();

                /**
                */
                void CancelAllOrders();

                /**
                */
                void AggregatedView(LimitContainer & BidContainer, LimitContainer & AskContainer) const;

                /**
                */
                void ByOrderView(std::vector<TOrder*> & BidContainer, std::vector<TOrder*> & AskContainer) const;

                /**
                */
                OpenInformationType GetTheoriticalAuctionInformations() const;
            
            public:

                inline void SetViewMode(ViewMode iView) { m_ViewMode = iView; }
                inline ViewMode GetViewMode() const { return m_ViewMode; }

            protected:

                bool AuctionInsert(std::unique_ptr<TOrder> & ipOrde);

                template <typename Container>
                volume_type GetExecutableQuantity(const Container & Orders, price_type iPrice) const;

                template <typename Msg>
                volume_type GetExecutableQuantity(const std::unique_ptr<Msg> & ipMsg, OrderWay iWay) const;

                template <typename Container, typename Msg>
                void ProcessDeals(Container & Orders, Msg & iMsg, volume_type iMatchQty);

                template <typename Msg>
                void ProcessDeals(Msg & iMsg, OrderWay iWay, volume_type iMatchQty);

                bid_index_type & GetBidIndex() { return bmi::get<price_tag>(m_BidOrders); }
                ask_index_type & GetAskIndex() { return bmi::get<price_tag>(m_AskOrders); }

                const bid_index_type & GetBidIndex() const { return bmi::get<price_tag>(m_BidOrders); }
                const ask_index_type & GetAskIndex() const { return bmi::get<price_tag>(m_AskOrders); }

            private:

                /**
                */
                void StreamByOrder(std::ostream& oss) const;
                /**
                */
                void StreamByPrice(std::ostream& oss) const;

            private:

                /* No copy constructor and assigment operator */
                OrderContainer(const OrderContainer & other);
                OrderContainer & operator= (const OrderContainer & other);

            protected:
                /* */
                BidStorage       m_BidOrders;
                /* */
                AskStorage       m_AskOrders;
                /* */
                TEventHandler&   m_EventHandler;
                /* */
                ViewMode         m_ViewMode;
                /* */
                OrderIDContainer m_InsertedOrderIDs;
                /* */
                GlobalOrderContainer m_InsertedOrders;
        };

    }
}

#include <Engine_OrderContainer.hxx>
