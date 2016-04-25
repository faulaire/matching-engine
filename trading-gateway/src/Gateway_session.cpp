#include <fix8/f8includes.hpp>

#include <Gateway_types.hpp>
#include <Gateway_router.hpp>
#include <Gateway_classes.hpp>
#include <Gateway_session.hpp>

bool Gateway_session_server::handle_admin(const unsigned seqnum, const FIX8::Message *msg)
{
    return enforce(seqnum, msg) || msg->process(_router);
}

bool Gateway_session_server::handle_application (const unsigned seqnum, const FIX8::Message *&msg)
{
    return enforce(seqnum, msg) || msg->process(_router);
}

bool Gateway_router_server::operator() (const FIX8::GateWay::Logon *msg) const
{
    std::cout << "Logon Message received" << std::endl;
    return true;
}

bool Gateway_router_server::operator() (const FIX8::GateWay::NewOrderSingle *msg) const
{
    std::cout << "NewOrderSingle message received" << std::endl;
    return true;
}

bool Gateway_router_server::operator() (const FIX8::GateWay::OrderCancelRequest *msg) const
{
    std::cout << "OrderCancelRequest message received" << std::endl;
    return true;
}

bool Gateway_router_server::operator() (const FIX8::GateWay::OrderCancelReplaceRequest *msg) const
{
    std::cout << "OrderCancelReplaceRequest message received" << std::endl;
    return true;
}

bool Gateway_router_server::operator() (const FIX8::GateWay::OrderStatusRequest *msg) const
{
    std::cout << "OrderStatusRequest message received" << std::endl;
    return true;
}