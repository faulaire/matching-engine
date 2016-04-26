#include <thread>

#include <Gateway_Application.h>

namespace exchange
{
    namespace gateway
    {

        void Application::onLogon( const FIX::SessionID& sessionID ) {}

        void Application::onLogout( const FIX::SessionID& sessionID ) {}

        void Application::fromApp( const FIX::Message& message,
                                   const FIX::SessionID& sessionID )
        throw( FIX::FieldNotFound, FIX::IncorrectDataFormat, FIX::IncorrectTagValue, FIX::UnsupportedMessageType )
        {
            crack( message, sessionID );
        }

        void Application::onMessage( const FIX42::NewOrderSingle& message, const FIX::SessionID& )
        {
            FIX::SenderCompID senderCompID;
            FIX::TargetCompID targetCompID;
            FIX::ClOrdID clOrdID;
            FIX::Symbol symbol;
            FIX::Side side;
            FIX::OrdType ordType;
            FIX::Price price;
            FIX::OrderQty orderQty;
            FIX::TimeInForce timeInForce(FIX::TimeInForce_DAY);
        }

        void Application::onMessage( const FIX42::OrderCancelRequest& message, const FIX::SessionID& )
        {
            FIX::OrigClOrdID origClOrdID;
            FIX::Symbol symbol;
            FIX::Side side;

            message.get( origClOrdID );
            message.get( symbol );
            message.get( side );
        }

        void Application::onMessage(const FIX42::OrderCancelReplaceRequest &, const FIX::SessionID &)
        {
            return;
        }

        void Application::onMessage(const FIX42::OrderStatusRequest &, const FIX::SessionID &)
        {
            return;
        }

    }
}
