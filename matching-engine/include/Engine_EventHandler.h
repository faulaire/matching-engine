/*
* Copyright (C) 2015, Fabien Aulaire
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

namespace exchange
{
    namespace engine
    {
        namespace bmi = boost::multi_index;

        template <typename TEventProcessor>
        class EventHandler
        {
            protected:

                struct deal_id_tag{};
                struct seller_id_tag{};
                struct buyer_id_tag{};
                
                
                using DealContainerType = boost::multi_index_container
                                    <
                                        std::unique_ptr<Deal>,
                                        bmi::indexed_by
                                            <
                                                bmi::hashed_unique<
                                                bmi::tag<deal_id_tag>, bmi::const_mem_fun<Deal, const std::string &, &Deal::GetReference> >,

                                                bmi::hashed_non_unique<
                                                bmi::tag<buyer_id_tag>, bmi::const_mem_fun<Deal, std::uint32_t, &Deal::GetBuyerClientID> >,

                                                bmi::hashed_non_unique<
                                                bmi::tag<seller_id_tag>, bmi::const_mem_fun<Deal, std::uint32_t, &Deal::GetSellerClientID> >
                                            >
                                    >;

            public:

                EventHandler(std::uint32_t iInstrumentID);
                virtual ~EventHandler();

            public:

                /**/
                void OnDeal(std::unique_ptr<Deal> ipDeal);

                /**/
                void OnUnsolicitedCancelledOrder(const Order & order);

            public:

                inline std::uint32_t   GetInstrumentID() const;
                inline size_t          GetDealCounter() const;

            
            protected:
                DealContainerType   m_DealContainer;
                std::uint32_t       m_InstrumentID;
        };


        template <typename TEventProcessor>
        EventHandler<TEventProcessor>::EventHandler(std::uint32_t iInstrumentID):
            m_InstrumentID(iInstrumentID)
        {}

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
        void EventHandler<TEventProcessor>::OnDeal(std::unique_ptr<Deal> ipDeal)
        {
            std::ostringstream  oss("");
            oss << m_InstrumentID << "_" << ipDeal->GetTimeStamp().time_since_epoch().count();
            oss << "_" << GetDealCounter()+1;
            ipDeal->SetReference(oss.str());

            auto insertion = m_DealContainer.insert(std::move(ipDeal));

            if (insertion.second)
            {
                auto pDeal = insertion.first->get();
                EXINFO("EventHandler::OnDeal : " << *pDeal);
                static_cast<TEventProcessor*>(this)->ProcessDeal(pDeal);
            }
            else
            {
                EXERR("EventHandler : Failed to insert and process deal [" << *(insertion.first->get()) );
                assert(false);
            }
        }

        template <typename TEventProcessor>
        void EventHandler<TEventProcessor>::OnUnsolicitedCancelledOrder(const Order & order)
        {
            static_cast<TEventProcessor*>(this)->ProcessUnsolicitedCancelledOrder(order);
        }
    }
}
