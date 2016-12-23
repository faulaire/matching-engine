#pragma once

#include <memory>
#include <boost/asio.hpp>

#include <Gateway_Session.h>

using boost::asio::ip::tcp;

namespace exchange
{
    namespace gateway
    {
        class TCPServer
        {
        public:
            TCPServer(engine::MatchingEngine<> & rMatchinEngine, boost::asio::io_service& io_service, std::uint32_t service)
                    : m_acceptor(io_service, tcp::endpoint(tcp::v4(), service)),
                      m_socket(io_service), m_matching_engine(rMatchinEngine)
            {}

            void start()
            {
                do_accept();
            }

        private:
            void do_accept()
            {
                m_acceptor.async_accept(m_socket,
                                       [this](boost::system::error_code ec)
                                       {
                                           if (!ec)
                                           {
                                               std::make_shared<Session>(std::move(m_socket), m_matching_engine)->start();
                                           }

                                           do_accept();
                                       });
            }

        private:
            tcp::acceptor              m_acceptor;
            tcp::socket                m_socket;
            engine::MatchingEngine<> & m_matching_engine;
        };
    }
}
