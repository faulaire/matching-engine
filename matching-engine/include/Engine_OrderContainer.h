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

            result_type operator()(const TOrder & key) const
            {
                return get_key(key.GetClientID(), key.GetOrderID());
            }
        };

        template <typename TOrder, typename SortingPredicate>
        struct OrderStorage
        {
            typedef boost::multi_index_container<
                TOrder,
                bmi::indexed_by<
                
                    bmi::hashed_unique<
                    bmi::tag<order_id_tag>, OrderIDGenerator<TOrder> >,

                    bmi::hashed_non_unique<
                    bmi::tag<client_id_tag>, bmi::const_mem_fun<TOrder, std::uint32_t, &TOrder::GetClientID> >,

                    bmi::ordered_non_unique<
                    bmi::tag<price_tag>, bmi::const_mem_fun<TOrder, typename TOrder::price_type, &TOrder::GetPrice>, SortingPredicate>
                >
            > order_set;
        };

        template<typename TOrder, typename TDealHandler> class OrderContainer;
        template<typename TOrder, typename TDealHandler> std::ostream& operator<< (std::ostream& o, const OrderContainer<TOrder, TDealHandler> & x);

        template <typename TOrder, typename TDealHandler>
        class OrderContainer
        {

            friend std::ostream& operator<< <> (std::ostream& o, const OrderContainer<TOrder, TDealHandler> & x);

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

                /* Types used to store the aggregated view of orders */
                /* NbOrder Qty Price*/
                using LimitType           = std::tuple<std::uint32_t, qty_type, price_type>;
                using LimitContainer      = std::vector<LimitType>;

                using OpenInformationType = std::tuple<std::uint32_t, std::uint64_t>;

            protected:

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
                OrderContainer(TDealHandler & iDealHandler):
                    m_DealHandler(iDealHandler), m_ViewMode(ViewMode::VM_BY_ORDER)
                {}

                /**
                */
                bool Insert(TOrder & iOrder, bool Match = false);

                /**
                */
                template <typename TOrderReplace>
                bool Modify(TOrderReplace & iOrderReplace, bool Match = false);

                /**
                */
                bool Delete(const std::uint32_t iOrderId, const std::uint32_t ClientId, OrderWay iWay);

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
                void ByOrderView(std::vector<TOrder> & BidContainer, std::vector<TOrder> & AskContainer) const;

                /**
                */
                OpenInformationType GetTheoriticalAuctionInformations() const;
            
            public:

                inline void SetViewMode(ViewMode iView) { m_ViewMode = iView; }
                inline ViewMode GetViewMode() const { return m_ViewMode; }

            protected:

                bool AuctionInsert(const TOrder & iOrder);

                template <typename Container>
                std::uint64_t GetExecutableQuantity(const Container & Orders, price_type iPrice) const;

                template <typename Msg>
                std::uint64_t GetExecutableQuantity(const Msg & iOrder, OrderWay iWay) const;

                template <typename Container, typename Msg>
                void ProcessDeals(Container & Orders, Msg & iMsg, std::uint64_t iMatchQty);

                template <typename Msg>
                void ProcessDeals(Msg & iMsg, OrderWay iWay, std::uint64_t iMatchQty);

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
                BidStorage      m_BidOrders;
                /* */
                AskStorage      m_AskOrders;
                /* */
                TDealHandler&   m_DealHandler;
                /* */
                ViewMode        m_ViewMode;
        };

    }
}

#include <Engine_OrderContainer.hxx>
