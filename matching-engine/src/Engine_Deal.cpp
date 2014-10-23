/*
* Copyright (C) 2014, Fabien Aulaire
* All rights reserved.
*/

#include <Engine_Deal.h>

namespace exchange
{
    namespace engine
    {

        Deal::Deal()
            :m_Reference{0}, m_Price(0), m_Qty(0), m_BuyerClientID(0), m_BuyerOrderID(0), m_SellerClientID(0), m_SellerOrderID(0)
        {
            m_Timestamp = std::chrono::system_clock::now();
        }

        Deal::Deal(price_type iPrice, qty_type iQty, UInt32 iBuyerClientID, UInt32 iBuyerOrderID, UInt32 iSellerClientID, UInt32 iSellerOrderID)
            : m_Reference{0}, m_Price(iPrice), m_Qty(iQty), m_BuyerClientID(iBuyerClientID),
            m_BuyerOrderID(iBuyerOrderID), m_SellerClientID(iSellerClientID), m_SellerOrderID(iSellerOrderID)
        {
            m_Timestamp = std::chrono::system_clock::now();
        }

        bool Deal::operator == (const Deal & rhs) const
        {
            return ( m_Price == rhs.GetPrice() ) && ( m_Qty == rhs.GetQuantity() ) &&
                   (m_BuyerClientID == rhs.GetBuyerClientID()) && (m_BuyerOrderID == rhs.GetBuyerOrderID()) &&
                   (m_SellerClientID == rhs.GetSellerClientID()) && (m_SellerOrderID == rhs.GetSellerOrderID());
        }

        std::ostream& operator<<(std::ostream& o, const Deal & x)
        {
            o << "Deal : Price[" << x.GetPrice() << "] ; Qty[" << x.GetQuantity() << "] ;"
                << " BuyerClientID[" << x.GetBuyerClientID() << "] ; BuyerOrderID[" << x.GetBuyerOrderID()
                << "] ; SellerClientID[" << x.GetSellerClientID() << "] ; SellerOrderID[" << x.GetSellerOrderID() << "] ;"
                << " Reference[" << x.GetReference().data() << "]";
            return o;
        }

    }
}