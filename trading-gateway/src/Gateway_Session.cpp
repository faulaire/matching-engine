#include <Gateway_Session.h>
#include <Logger.h>

namespace exchange
{
    namespace gateway
    {
        void Session::write_message()
        {
            protocol::OneMessage msg;
            msg.set_type(protocol::OneMessage::LOGON_REPLY);

            auto * logon_reply = new protocol::LogonReply();
            msg.set_allocated_logon_reply_msg(logon_reply);

            logon_reply->set_reject_code(0);

            std::string sBuffer;
            msg.SerializeToString(&sBuffer);

            auto self(shared_from_this());

            Message::Header rHeader;
            rHeader.m_BodyLength = sBuffer.length();

            boost::asio::async_write(m_socket, boost::asio::buffer(rHeader.m_Buffer, 4),
                                     [self](boost::system::error_code ec, std::size_t /*length*/){});

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
                case protocol::OneMessage_Type_HEARTBEAT:
                    if( rMsg.has_heartbeat_msg())
                    {
                        process_heartbeat_message(rMsg.heartbeat_msg());
                    }
                    else
                    {
                        EXERR("Session::process_message : Message type is LOGON "
                                      << "but no logon message available");
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
            write_message();
        }

        void Session::process_new_order_message(const protocol::NewOrder & rNewOrder)
        {
            EXINFO("New Order Message received");
            EXINFO("InstrumentID : " << rNewOrder.instrument_id() << " ; Side : " << rNewOrder.side()
                        << " ; " << rNewOrder.order_quantity() << "@" << rNewOrder.limit_price());
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
        }
    }
}


