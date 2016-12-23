#include <Gateway_Session.h>
#include <Logger.h>

namespace exchange
{
    namespace gateway
    {
        void Session::write_message(const protocol::OneMessage & msg)
        {
            std::string sBuffer;
            msg.SerializeToString(&sBuffer);

            Message::Header rHeader;
            rHeader.m_BodyLength = sBuffer.length();

            auto self(shared_from_this());

            boost::asio::async_write(m_socket, boost::asio::buffer(rHeader.m_Buffer, Message::header_length),
                                     [self](boost::system::error_code /* ec */, std::size_t /*length*/){});

            boost::asio::async_write(m_socket, boost::asio::buffer(sBuffer.c_str(), sBuffer.length()),
                                     [self](boost::system::error_code /* ec */, std::size_t /*length*/){});
        }

        void Session::do_read_header()
        {
            auto self(shared_from_this());
            boost::asio::async_read(m_socket,
                                    boost::asio::buffer(m_read_msg.header(), Message::header_length),
                                    [this, self](boost::system::error_code ec, std::size_t /*length*/)
                                    {
                                        if (!ec && m_read_msg.decode_header())
                                        {
                                            do_read_body();
                                        }
                                        else
                                        {
                                            m_socket.close();
                                        }
                                    });
        }

        void Session::do_read_body()
        {
            auto self(shared_from_this());
            boost::asio::async_read(m_socket,
                                    boost::asio::buffer(m_read_msg.body(), m_read_msg.body_length()),
                                    [this, self](boost::system::error_code ec, std::size_t /*length*/)
                                    {
                                        if (!ec)
                                        {
                                            protocol::OneMessage msg;
                                            if (msg.ParseFromArray(m_read_msg.body(), m_read_msg.body_length()))
                                            {
                                                self->process_message(msg);
                                            }
                                            else
                                            {
                                                EXERR("Session::do_read_body : Protobuf decoding error, closing socket");
                                                m_socket.close();
                                            }

                                            do_read_header();
                                        }
                                        else
                                        {
                                            m_socket.close();
                                        }
                                    });
        }

        void Session::process_message(const protocol::OneMessage & rMsg)
        {
            switch (rMsg.type())
            {
                case protocol::OneMessage_Type_NEW_ORDER:
                    if (rMsg.has_new_order_msg())
                    {
                        process_new_order_message(rMsg.new_order_msg());
                    }
                    else
                    {
                        EXERR("Session::process_message : Message type is NEW_ORDER "
                                      << "but no new_order message available");
                    }
                    break;
                case protocol::OneMessage_Type_MOD_ORDER:
                    if (rMsg.has_mod_order_msg())
                    {
                        process_mod_order_message(rMsg.mod_order_msg());
                    }
                    else
                    {
                        EXERR("Session::process_message : Message type is MOD_ORDER "
                                      << "but no mod_order message available");
                    }
                    break;
                case protocol::OneMessage_Type_CAN_ORDER:
                    if (rMsg.has_can_order_msg())
                    {
                        process_can_order_message(rMsg.can_order_msg());
                    }
                    else
                    {
                        EXERR("Session::process_message : Message type is CAN_ORDER "
                                      << "but no can_order message available");
                    }
                    break;
                case protocol::OneMessage_Type_LOGON:
                    if( rMsg.has_logon_msg())
                    {
                        process_logon_message(rMsg.logon_msg());
                    }
                    else
                    {
                        EXERR("Session::process_message : Message type is LOGON "
                                      << "but no logon message available");
                    }
                    break;
                case protocol::OneMessage_Type_LOGOUT:
                    if( rMsg.has_logout_msg())
                    {
                        process_logout_message(rMsg.logout_msg());
                    }
                    else
                    {
                        EXERR("Session::process_message : Message type is LOGOUT "
                                      << "but no logout message available");
                    }
                    break;
                case protocol::OneMessage_Type_HEARTBEAT:
                    if( rMsg.has_heartbeat_msg())
                    {
                        process_heartbeat_message(rMsg.heartbeat_msg());
                    }
                    else
                    {
                        EXERR("Session::process_message : Message type is HEARTBEAT "
                                      << "but no heartbeat message available");
                    }
                    break;
                default:
                    EXERR("Session::process_message : Unhandled message type : " << rMsg.type());
                    break;
            }
        }

        void Session::process_logon_message(const protocol::Logon & rLogon)
        {
            EXINFO("Logon Message received");
            EXINFO(rLogon.user_name() << ":"  << rLogon.password());

            protocol::OneMessage msg;
            msg.set_type(protocol::OneMessage::LOGON_REPLY);

            auto * logon_reply = new protocol::LogonReply();
            msg.set_allocated_logon_reply_msg(logon_reply);

            logon_reply->set_reject_code( ::google::protobuf::uint32(0) );

            write_message(msg);
        }

        void Session::process_logout_message(const protocol::Logout & rLogout)
        {
            EXINFO("Logout Message received");
        }


        void Session::process_new_order_message(const protocol::NewOrder & rNewOrder)
        {
            EXINFO("New Order Message received : InstrumentID : " << rNewOrder.instrument_id() << " ; Side : " << rNewOrder.side()
                        << " ; " << rNewOrder.order_quantity() << "@" << rNewOrder.limit_price());

            engine::OrderWay eWay = engine::OrderWay::MAX_WAY;
            if( !decode_order_way(rNewOrder, eWay) )
            {
                EXERR("Session::process_new_order : Invalid away : " << rNewOrder.side());
                return;
            }

            const auto nQuantity = engine::Order::qty_type(rNewOrder.order_quantity());
            const auto nPrice = engine::Order::price_type(rNewOrder.limit_price());
            const auto nClientOrderID = engine::Order::client_orderid_type(rNewOrder.client_order_id());
            const auto nClientID = engine::Order::client_id_type(0);

            auto pOrder = std::make_unique<engine::Order>(eWay, nQuantity, nPrice, nClientOrderID, nClientID);

            const auto result = m_matching_engine.Insert( std::move(pOrder), rNewOrder.instrument_id());
            if (result  == engine::Status::Ok )
            {
                // Ack the order
                EXINFO("Session::process_new_order : Order inserted");
            }
            else
            {
                EXERR("Session::process_new_order : Failed to insert order. Reason[ " << result << " ]");
            }
        }

        void Session::process_mod_order_message(const protocol::ModOrder & rModOrder)
        {
            EXINFO("Mod Order Message received");
        }

        void Session::process_can_order_message(const protocol::CanOrder & rCanOrder)
        {
            EXINFO("Can Order Message received");
        }

        void Session::process_heartbeat_message(const protocol::Hearbeat & rHeartBeat)
        {
            EXINFO("HeartBeat Message received");

            protocol::OneMessage msg;
            msg.set_type(protocol::OneMessage::HEARTBEAT);

            auto * heartbeat = new protocol::Hearbeat();
            msg.set_allocated_heartbeat_msg(heartbeat);

            write_message(msg);
        }

        bool Session::decode_order_way(const protocol::NewOrder & rNewOrder, engine::OrderWay & rWay)
        {
            switch(rNewOrder.side())
            {
                case protocol::NewOrder::BUY:
                    rWay = engine::OrderWay::BUY;
                    break;
                case protocol::NewOrder::SELL:
                    rWay = engine::OrderWay::SELL;
                    break;
                default:
                    rWay = engine::OrderWay::MAX_WAY;
            }
            return rWay != engine::OrderWay::MAX_WAY;
        }
    }
}


