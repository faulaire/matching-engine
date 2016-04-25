#include <thread>
#include <vector>

#include <Logger.h>
#include <Engine_MatchingEngine.h>

// f8 headers
#include <fix8/f8includes.hpp>
#include <fix8/usage.hpp>

#include "Gateway_types.hpp"
#include "Gateway_router.hpp"
#include "Gateway_classes.hpp"
#include "Gateway_session.hpp"

bool term_received(false);
bool quiet(false);

void sig_handler(int sig)
{
    switch (sig)
    {
        case SIGTERM:
        case SIGINT:
            term_received = true;
            signal(sig, sig_handler);
            break;
        default:
            std::cerr << sig << std::endl;
            break;
    }
}

void server_process(FIX8::ServerSessionBase *srv, int scnt);

unsigned next_send(0), next_receive(0);

int main( int argc, char** argv )
{
    if (argc != 3)
    {
        std::cerr << "usage: " << argv[0]
        << " MATCHING_CONFIG."<< " GATEWAY_CONFIG" << std::endl;
        return 1;
    }

    std::string s_matching_config = argv[1];
    std::string s_gateway_config  = argv[2];

    auto & Logger = LoggerHolder::GetInstance();

    boost::property_tree::ptree aConfig;
    if (!boost::filesystem::exists(s_matching_config))
    {
        std::cerr << "File " << s_matching_config << " doesn't exists" << std::endl;
        return 2;
    }

    if (!boost::filesystem::exists(s_gateway_config))
    {
        std::cerr << "File " << s_gateway_config << " doesn't exists" << std::endl;
        return 2;
    }

    exchange::engine::MatchingEngine<> _MatchingEngine;

    boost::property_tree::ini_parser::read_ini(s_matching_config, aConfig);
    Logger.Init(aConfig);

    if( !_MatchingEngine.Configure(aConfig) )
    {
        std::cerr << "Could not configure matching engine" << std::endl;
        return 3;
    }

    signal(SIGTERM, sig_handler);
    signal(SIGINT, sig_handler);

    try
    {
        FIX8::f8_atomic<unsigned> scnt(0);

        std::unique_ptr<FIX8::ServerManager> sm(new FIX8::ServerManager);
        sm->add(new FIX8::ServerSession<Gateway_session_server>(FIX8::GateWay::ctx(), s_gateway_config, "TEX1"));

        std::vector<std::thread> thrds;
        while (!term_received)
        {
            FIX8::ServerSessionBase *srv(sm->select());
            if (srv)
            {
                thrds.push_back(std::thread ([&]() { server_process(srv, ++scnt); }));
                FIX8::hypersleep<FIX8::h_milliseconds>(10);
            }
            else
            {
                _MatchingEngine.EngineListen();
            }
        }
        for_each(thrds.begin(), thrds.end(), [](std::thread& tt) { if (tt.joinable()) tt.join(); });

    }
    catch ( std::exception & e )
    {
        std::cout << e.what() << std::endl;
        return 4;
    }
}

void server_process(FIX8::ServerSessionBase *srv, int scnt)
{
    std::unique_ptr<FIX8::SessionInstanceBase> inst(srv->create_server_instance());
    if (!quiet)
        inst->session_ptr()->control() |= FIX8::Session::print;
    glout_info << "client(" << scnt << ") connection established.";
    const FIX8::ProcessModel pm(srv->get_process_model(srv->_ses));
    std::cout << (pm == FIX8::pm_pipeline ? "Pipelined" : "Threaded") << " mode." << std::endl;
    inst->start(pm == FIX8::pm_pipeline, next_send, next_receive);
    if (pm != FIX8::pm_pipeline)
        while (!inst->session_ptr()->is_shutdown())
            FIX8::hypersleep<FIX8::h_milliseconds>(10);
    std::cout << "Session(" << scnt << ") finished." << std::endl;
    inst->stop();
}

