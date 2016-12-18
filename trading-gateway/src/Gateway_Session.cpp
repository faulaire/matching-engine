#include <Gateway_Session.h>

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
                                     [self](boost::system::error_code ec, std::size_t /*length*/){});
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
                                                std::cerr << "Protobuf decoding error, closing socket" << std::endl;
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
                        const auto & new_order_msg = rMsg.new_order_msg();
                    }
                    else
                    {
                        std::cerr << "Message is flagged as new order but no new order message available" << std::endl;
                    }
                    break;
                case protocol::OneMessage_Type_MOD_ORDER:
                    std::cout << "Mod Order Message received" << std::endl;
                    break;
                case protocol::OneMessage_Type_CAN_ORDER:
                case protocol::OneMessage_Type_LOGON:
                    if( rMsg.has_logon_msg())
                    {
                        process_logon_message(rMsg.logon_msg());
                    }
                    else
                    {
                        std::cerr << "Message is flagged as logon but no logon message available" << std::endl;
                    }
                    break;
                case protocol::OneMessage_Type_HEARTBEAT:
                    break;
                default:
                    std::cerr << "Unhandled message type : " << rMsg.type() << std::endl;
                    break;
            }
        }

        void Session::process_logon_message(const protocol::Logon & rLogon)
        {
            std::cout << "Logon Message" << std::endl;
            std::cout << rLogon.user_name() << ":"  << rLogon.password() << std::endl;
            write_message();
        }
    }
}


