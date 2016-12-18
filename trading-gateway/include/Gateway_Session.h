#pragma once

#include <iostream>
#include <memory>
#include <boost/asio.hpp>

#include <Gateway_Message.h>
#include <protocol.pb.h>


using boost::asio::ip::tcp;

namespace exchange
{
    namespace gateway
    {
        class Session : public std::enable_shared_from_this<Session>
        {
        public:
            Session(tcp::socket socket):
                    m_socket(std::move(socket))
            {}

            void start()
            {
                do_read_header();
            }

        private:

            void do_read_header();
            void do_read_body();

            void write_message();
            void process_message(const protocol::OneMessage & rMsg);

            void process_logon_message(const protocol::Logon & rLogon);

        private:
            tcp::socket m_socket;
            Message     m_read_msg;
        };
    }
}
