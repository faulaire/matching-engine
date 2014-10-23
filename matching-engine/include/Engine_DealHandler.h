/*
* Copyright (C) 2014, Fabien Aulaire
* All rights reserved.
*/

#ifndef ENGINE_DEAL_HANDLER_INCLUDE
#define ENGINE_DEAL_HANDLER_INCLUDE

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

        class DealHandler
        {
            protected:

                struct deal_id_tag{};
                struct seller_id_tag{};
                struct buyer_id_tag{};
                
                
                using DealContainerType = boost::multi_index_container
                                    <
                                        Deal*,
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
                void OnDeal(Deal * ipDeal);

            public:

                inline UInt32 GetInstrumentID() const;
                inline UInt64 GetDealCounter() const;

            
            protected:
                DealContainerType m_DealContainer;
                UInt32            m_InstrumentID;
                UInt64            m_DealCounter;
        };

        inline UInt32 DealHandler::GetInstrumentID() const
        {
            return m_InstrumentID;
        }

        inline UInt64 DealHandler::GetDealCounter() const
        {
            return m_DealCounter;
        }
    }
}

#endif
