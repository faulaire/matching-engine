/*
* Copyright (C) 2014, Fabien Aulaire
* All rights reserved.
*/

#pragma once

#include <array>
#include <cstring>
#include <cassert>
#include <chrono>

#include <Types.h>

#include <Engine_Order.h>

namespace exchange
{
    namespace engine
    {

        /*!
        *  \brief Deal
        */
        class Deal
        {

            friend std::ostream& operator<<(std::ostream& o, const Deal & x);

            public:

                using TimestampType = std::chrono::system_clock::time_point;

                using price_type = Order::price_type;
                using qty_type   = Order::qty_type;

            public:

                Deal(price_type iPrice, qty_type iQty, UInt32 iBuyerClientID, UInt32 iBuyerOrderID, UInt32 iSellerClientID, UInt32 iSellerOrderID);

                Deal() = delete;
                Deal(const Deal & rhs) = delete;

                bool operator==(const Deal & rhs) const;

            public:

                inline price_type            GetPrice() const;
                inline void                  SetPrice(price_type iPrice);
                                             
                inline qty_type              GetQuantity() const;
                inline void                  SetQuantity(qty_type iQty);
                                             
                inline UInt32                GetBuyerClientID() const;
                inline void                  SetBuyerClientID(UInt32 iId);
                                             
                inline UInt32                GetSellerClientID() const;
                inline void                  SetSellerClientID(UInt32 iId);
                                             
                inline UInt32                GetBuyerOrderID() const;
                inline void                  SetBuyerOrderID(UInt32 iId);
                                             
                inline UInt32                GetSellerOrderID() const;
                inline void                  SetSellerOrderID(UInt32 iId);
                
                inline TimestampType         GetTimeStamp() const;

                inline const std::string &   GetReference() const;
                inline void                  SetReference(const std::string &);

            protected:
                std::string   m_Reference;
                price_type    m_Price;
                qty_type      m_Qty;
                TimestampType m_Timestamp;
                UInt32        m_BuyerClientID;
                UInt32        m_BuyerOrderID;
                UInt32        m_SellerClientID;
                UInt32        m_SellerOrderID;
        };

        std::ostream& operator<<(std::ostream& o, const Deal & x);

        inline Deal::price_type Deal::GetPrice() const
        {
            return m_Price;
        }

        inline void Deal::SetPrice(price_type iPrice)
        {
            m_Price = iPrice;
        }

        inline Deal::qty_type Deal::GetQuantity() const
        {
            return m_Qty;
        }

        inline void Deal::SetQuantity(qty_type iQty)
        {
            m_Qty = iQty;
        }

        inline UInt32 Deal::GetBuyerClientID() const
        {
            return m_BuyerClientID;
        }

        inline void   Deal::SetBuyerClientID(UInt32 iId)
        {
            m_BuyerClientID = iId;
        }

        inline UInt32 Deal::GetSellerClientID() const
        {
            return m_SellerClientID;
        }

        inline void   Deal::SetSellerClientID(UInt32 iId)
        {
            m_SellerClientID = iId;
        }

        inline UInt32 Deal::GetBuyerOrderID() const
        {
            return m_BuyerOrderID;
        }

        inline void Deal::SetBuyerOrderID(UInt32 iId)
        {
            m_BuyerOrderID = iId;
        }

        inline UInt32 Deal::GetSellerOrderID() const
        {
            return m_SellerOrderID;
        }

        inline void Deal::SetSellerOrderID(UInt32 iId)
        {
            m_SellerOrderID = iId;
        }

        inline const std::string & Deal::GetReference() const
        {
            return m_Reference;
        }

        inline void Deal::SetReference(const std::string & iRef)
        {
            m_Reference = iRef;
        }

        inline Deal::TimestampType Deal::GetTimeStamp() const
        {
            return m_Timestamp;
        }
    }
}
