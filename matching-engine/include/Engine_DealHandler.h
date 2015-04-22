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

        template <typename TDealProcessor>
        class DealHandler
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

                DealHandler(std::uint32_t iInstrumentID);
                virtual ~DealHandler();

            public:

                /**/
                void OnDeal(std::unique_ptr<Deal> ipDeal);

            public:

                inline std::uint32_t      GetInstrumentID() const;
                inline std::uint64_t      GetDealCounter() const;

            
            protected:
                DealContainerType   m_DealContainer;
                std::uint32_t       m_InstrumentID;
        };


        template <typename TDealProcessor>
        DealHandler<TDealProcessor>::DealHandler(std::uint32_t iInstrumentID):
            m_InstrumentID(iInstrumentID)
        {}

        template <typename TDealProcessor>
        DealHandler<TDealProcessor>::~DealHandler()
        {}

        template <typename TDealProcessor>
        inline std::uint32_t DealHandler<TDealProcessor>::GetInstrumentID() const
        {
            return m_InstrumentID;
        }

        template <typename TDealProcessor>
        inline std::uint64_t DealHandler<TDealProcessor>::GetDealCounter() const
        {
            return m_DealContainer.size();
        }

        template <typename TDealProcessor>
        void DealHandler<TDealProcessor>::OnDeal(std::unique_ptr<Deal> ipDeal)
        {
            std::ostringstream  oss("");
            oss << m_InstrumentID << "_" << ipDeal->GetTimeStamp().time_since_epoch().count();
            oss << "_" << GetDealCounter()+1;
            ipDeal->SetReference(oss.str());

            auto insertion = m_DealContainer.insert(std::move(ipDeal));

            if (insertion.second)
            {
                Deal* pDeal = insertion.first->get();
                static_cast<TDealProcessor*>(this)->ProcessDeal(pDeal);
            }
            else
            {
                EXERR("DealHandler : Failed to insert and process deal [" << (*insertion.first).get());
                assert(false);
            }
        }
    }
}
