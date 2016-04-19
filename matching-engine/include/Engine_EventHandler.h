/*
* Copyright (C) 2016, Fabien Aulaire
* All rights reserved.
*/

#pragma once

#include <Engine_Deal.h>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/mem_fun.hpp>

#include <iostream>
#include <MemoryPool.h>

namespace exchange
{
    namespace engine
    {
        namespace bmi = boost::multi_index;

        template <typename TPool>
        struct pool_delete
        {
            pool_delete() = default;

            pool_delete(TPool * pool)
                    :_pool(pool)
            { }

            template <class U>
            void operator()(U* ptr) const
            {
                _pool->deleteElement(ptr);
            }

            TPool * _pool = nullptr;
        };


        template <typename TEventProcessor>
        class EventHandler
        {
            public:

                using pool_type = MemoryPool<Deal, 65536>;
                using pool_delete_type = pool_delete<pool_type>;
                using deal_ptr_type = std::unique_ptr<Deal, pool_delete_type>;

            protected:

                struct deal_id_tag{};
                struct seller_id_tag{};
                struct buyer_id_tag{};
                
                
                using client_id_type = typename Deal::client_id_type;



                using DealContainerType = boost::multi_index_container
                                    <
                                        deal_ptr_type,
                                        bmi::indexed_by
                                            <
                                                bmi::hashed_unique<
                                                bmi::tag<deal_id_tag>, bmi::const_mem_fun<Deal, const std::string &, &Deal::GetReference> >,

                                                bmi::hashed_non_unique<
                                                bmi::tag<buyer_id_tag>, bmi::const_mem_fun<Deal, client_id_type, &Deal::GetBuyerClientID>, Hasher<client_id_type> >,

                                                bmi::hashed_non_unique<
                                                bmi::tag<seller_id_tag>, bmi::const_mem_fun<Deal, client_id_type, &Deal::GetSellerClientID>, Hasher<client_id_type> >
                                            >
                                    >;

            public:

                EventHandler(std::uint32_t iInstrumentID);
                virtual ~EventHandler();

            public:

                /**/
                template <typename... Args>
                deal_ptr_type CreateDeal(Args &&... args)
                {
                    return deal_ptr_type(m_pDealPool->newElement(std::forward<Args>(args)...), pool_delete_type(m_pDealPool.get()));
                }

                /**/
                void OnDeal(deal_ptr_type ipDeal);

                /**/
                void OnUnsolicitedCancelledOrder(const Order* order);

                /**/
                inline void RehashDealIndexes(size_t size);

            public:

                inline std::uint32_t   GetInstrumentID() const;
                inline size_t          GetDealCounter() const;

            
            protected:
                std::unique_ptr<pool_type>  m_pDealPool;
                DealContainerType           m_DealContainer;
                std::uint32_t               m_InstrumentID;
        };


        template <typename TEventProcessor>
        EventHandler<TEventProcessor>::EventHandler(std::uint32_t iInstrumentID):
            m_InstrumentID(iInstrumentID)
        {
            m_pDealPool = std::make_unique<pool_type>();
        }

        template <typename TEventProcessor>
        EventHandler<TEventProcessor>::~EventHandler()
        {}

        template <typename TEventProcessor>
        inline std::uint32_t EventHandler<TEventProcessor>::GetInstrumentID() const
        {
            return m_InstrumentID;
        }

        template <typename TEventProcessor>
        inline size_t EventHandler<TEventProcessor>::GetDealCounter() const
        {
            return m_DealContainer.size();
        }

        template <typename TEventProcessor>
        void EventHandler<TEventProcessor>::OnDeal(deal_ptr_type ipDeal)
        {
            std::ostringstream  oss("");
            oss << m_InstrumentID << "_" << ipDeal->GetTimeStamp().time_since_epoch().count();
            oss << "_" << GetDealCounter()+1;
            ipDeal->SetReference(oss.str());

            auto insertion = m_DealContainer.insert(std::move(ipDeal));

            if (insertion.second)
            {
                auto pDeal = insertion.first->get();
                static_cast<TEventProcessor*>(this)->ProcessDeal(pDeal);
            }
            else
            {
                EXERR("EventHandler : Failed to insert and process deal [" << *(insertion.first->get()) );
                assert(false);
            }
        }

        template <typename TEventProcessor>
        void EventHandler<TEventProcessor>::OnUnsolicitedCancelledOrder(const Order * order)
        {
            static_cast<TEventProcessor*>(this)->ProcessUnsolicitedCancelledOrder(order);
        }

        template <typename TEventProcessor>
        inline void EventHandler<TEventProcessor>::RehashDealIndexes(size_t size)
        {
            bmi::get<deal_id_tag>(m_DealContainer).rehash(size);
            bmi::get<seller_id_tag>(m_DealContainer).rehash(size);
            bmi::get<buyer_id_tag>(m_DealContainer).rehash(size);
        }
    }
}
