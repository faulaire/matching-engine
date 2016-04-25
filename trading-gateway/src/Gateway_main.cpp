#include <Logger.h>

// f8 headers
#include <fix8/f8includes.hpp>
#include <fix8/usage.hpp>

#include "Gateway_types.hpp"
#include "Gateway_router.hpp"
#include "Gateway_classes.hpp"
#include "Gateway_session.hpp"

bool term_received(false);
bool quiet(true);

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

    boost::property_tree::ini_parser::read_ini(s_matching_config, aConfig);
    Logger.Init(aConfig);

    signal(SIGTERM, sig_handler);
    signal(SIGINT, sig_handler);

    unsigned next_send(0), next_receive(0);

    try
    {
        std::unique_ptr<FIX8::ServerSessionBase> ms(new FIX8::ServerSession<Gateway_session_server>(FIX8::GateWay::ctx(), s_gateway_config, "TEX1"));
        for (unsigned scnt(0); !term_received; )
        {
            if (!ms->poll())
                continue;
            std::unique_ptr<FIX8::SessionInstanceBase> inst(ms->create_server_instance());
            if (!quiet)
                inst->session_ptr()->control() |= FIX8::Session::print;
            std::ostringstream sostr;
            sostr << "client(" << ++scnt << ") connection established.";
            FIX8::GlobalLogger::log(sostr.str());
            const FIX8::ProcessModel pm(ms->get_process_model(ms->_ses));
            inst->start(pm == FIX8::pm_pipeline, next_send, next_receive);
            std::cout << (pm == FIX8::pm_pipeline ? "Pipelined" : "Threaded") << " mode." << std::endl;
            if (inst->session_ptr()->get_connection()->is_secure())
                std::cout << "Session is secure (SSL)" << std::endl;
            if (pm != FIX8::pm_pipeline)
                while (!inst->session_ptr()->is_shutdown())
                    FIX8::hypersleep<FIX8::h_milliseconds>(100);
            std::cout << "Session(" << scnt << ") finished." << std::endl;
            inst->stop();
#if defined FIX8_CODECTIMING
            FIX8::Message::report_codec_timings("server");
#endif
        }
    }
    catch ( std::exception & e )
    {
        std::cout << e.what() << std::endl;
        return 4;
    }
}