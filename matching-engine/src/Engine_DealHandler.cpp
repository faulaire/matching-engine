#include <Engine_DealHandler.h>

#include <sstream>

namespace exchange
{
    namespace engine
    {

        DealHandler::DealHandler(UInt32 iInstrumentID):
            m_InstrumentID(iInstrumentID), m_DealCounter(0)
        {}

        DealHandler::~DealHandler()
        {}

        void DealHandler::OnDeal(Deal * ipDeal)
        {
            m_DealCounter++;

            std::ostringstream  oss("");
            oss << m_InstrumentID << "_" << ipDeal->GetTimeStamp().time_since_epoch().count();
            oss << "_" << m_DealCounter;
            ipDeal->SetReference(oss.str());

            m_DealContainer.insert(ipDeal);
        }

    }
}