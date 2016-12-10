#pragma once

#include "quickfix/Application.h"
#include "quickfix/MessageCracker.h"
#include "quickfix/Values.h"
#include "quickfix/Utility.h"
#include "quickfix/Mutex.h"

#include "quickfix/fix42/NewOrderSingle.h"
#include "quickfix/fix42/OrderCancelRequest.h"
#include "quickfix/fix42/OrderCancelReplaceRequest.h"
#include "quickfix/fix42/OrderStatusRequest.h"

namespace exchange
{
    namespace gateway
    {

        class Application
                : public FIX::Application,
                  public FIX::MessageCracker {
            // Application overloads

            void onCreate(const FIX::SessionID &) { }

            void onLogon(const FIX::SessionID &sessionID);
            void onLogout(const FIX::SessionID &sessionID);

            void toAdmin(FIX::Message &, const FIX::SessionID &) { }

            void toApp(FIX::Message &, const FIX::SessionID &)
            throw(FIX::DoNotSend) { }

            void fromAdmin(const FIX::Message &, const FIX::SessionID &)
            throw(FIX::FieldNotFound, FIX::IncorrectDataFormat, FIX::IncorrectTagValue, FIX::RejectLogon) { }

            void fromApp(const FIX::Message &message, const FIX::SessionID &sessionID)
                    throw(FIX::FieldNotFound, FIX::IncorrectDataFormat, FIX::IncorrectTagValue, FIX::UnsupportedMessageType);

            // MessageCracker overloads
            void onMessage(const FIX42::NewOrderSingle &, const FIX::SessionID &);
            void onMessage(const FIX42::OrderCancelRequest &, const FIX::SessionID &);
            void onMessage(const FIX42::OrderCancelReplaceRequest &, const FIX::SessionID &);
            void onMessage(const FIX42::OrderStatusRequest &, const FIX::SessionID &);
        };
    }
}
