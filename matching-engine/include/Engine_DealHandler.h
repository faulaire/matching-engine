/*
* Copyright (C) 2014, Fabien Aulaire
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
                                                bmi::tag<buyer_id_tag>, bmi::const_mem_fun<Deal, UInt32, &Deal::GetBuyerClientID> >,

                                                bmi::hashed_non_unique<
                                                bmi::tag<seller_id_tag>, bmi::const_mem_fun<Deal, UInt32, &Deal::GetSellerClientID> >
                                            >
                                    >;

            public:

                DealHandler(UInt32 iInstrumentID);
                virtual ~DealHandler();

            public:

                /**/
                void OnDeal(std::unique_ptr<Deal> ipDeal);

            public:

                inline UInt32 GetInstrumentID() const;
                inline UInt64 GetDealCounter() const;
            
            protected:
                DealContainerType m_DealContainer;
                UInt32            m_InstrumentID;
        };


        template <typename TDealProcessor>
        DealHandler<TDealProcessor>::DealHandler(UInt32 iInstrumentID):
            m_InstrumentID(iInstrumentID)
        {}

        template <typename TDealProcessor>
        DealHandler<TDealProcessor>::~DealHandler()
        {}

        template <typename TDealProcessor>
        inline UInt32 DealHandler<TDealProcessor>::GetInstrumentID() const
        {
            return m_InstrumentID;
        }

        template <typename TDealProcessor>
        inline UInt64 DealHandler<TDealProcessor>::GetDealCounter() const
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
                static_cast<TDealProcessor*>(this)->ProcessDeal((*insertion.first).get());
            }
            else
            {
                EXERR("DealHandler : Failed to insert and process deal [" << (*insertion.first).get());
                assert(false);
            }
        }
    }
}
