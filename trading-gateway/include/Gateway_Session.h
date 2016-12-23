#pragma once

#include <iostream>
#include <memory>
#include <boost/asio.hpp>

#include <Gateway_Message.h>
#include <protocol.pb.h>

#include <Engine_MatchingEngine.h>

using boost::asio::ip::tcp;

namespace exchange
{
    namespace gateway
    {
        class Session : public std::enable_shared_from_this<Session>
        {
        public:
            Session(tcp::socket socket, engine::MatchingEngine<> & rMatchinEngine):
                    m_socket(std::move(socket)), m_matching_engine(rMatchinEngine)
            {}

            void start()
            {
                do_read_header();
            }

        private:

            void do_read_header();
            void do_read_body();

            void write_message(const protocol::OneMessage & msg);
            void process_message(const protocol::OneMessage & rMsg);

            void process_logon_message(const protocol::Logon & rLogon);
            void process_logout_message(const protocol::Logout & rLogout);
            void process_new_order_message(const protocol::NewOrder & rNewOrder);
            void process_mod_order_message(const protocol::ModOrder & rModOrder);
            void process_can_order_message(const protocol::CanOrder & rCanOrder);
            void process_heartbeat_message(const protocol::Hearbeat & rHeartBeat);

        private:

            bool decode_order_way(const protocol::NewOrder & rNewOrder, engine::OrderWay & rWay);

        private:
            tcp::socket                m_socket;
            Message                    m_read_msg;
            engine::MatchingEngine<> & m_matching_engine;
        };
    }
}
