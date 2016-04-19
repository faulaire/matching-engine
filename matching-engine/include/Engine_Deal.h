/*
* Copyright (C) 2016, Fabien Aulaire
* All rights reserved.
*/

#pragma once

#include <array>
#include <cstring>
#include <cassert>
#include <chrono>
#include <iosfwd>
#include <utility>
#include <iostream>

#include <boost/noncopyable.hpp>

#include <Engine_Order.h>

namespace exchange
{
    namespace engine
    {

        /*!
        *  \brief Deal
        */
        class Deal : boost::noncopyable
        {

            friend std::ostream& operator<<(std::ostream& o, const Deal & x);

            public:

                using TimestampType = std::chrono::system_clock::time_point;

                using price_type          = Order::price_type;
                using qty_type            = Order::qty_type;
                using client_orderid_type = Order::client_orderid_type;
                using client_id_type      = Order::client_id_type;

            public:

                Deal(price_type iPrice, qty_type iQty, client_id_type iBuyerClientID, client_orderid_type iBuyerOrderID, client_id_type iSellerClientID, client_orderid_type iSellerOrderID);

                Deal() = delete;

                bool operator==(const Deal & rhs) const;

            public:

                inline price_type            GetPrice() const;
                inline void                  SetPrice(price_type iPrice);
                                             
                inline qty_type              GetQuantity() const;
                inline void                  SetQuantity(qty_type iQty);
                                             
                inline client_id_type        GetBuyerClientID() const;
                inline void                  SetBuyerClientID(client_id_type iId);
                                             
                inline client_id_type        GetSellerClientID() const;
                inline void                  SetSellerClientID(client_id_type iId);
                                             
                inline client_orderid_type   GetBuyerOrderID() const;
                inline void                  SetBuyerOrderID(client_orderid_type iId);
                                             
                inline client_orderid_type   GetSellerOrderID() const;
                inline void                  SetSellerOrderID(client_orderid_type iId);
                
                inline TimestampType         GetTimeStamp() const;

                inline const std::string &   GetReference() const;
                inline void                  SetReference(std::string);

            protected:
                std::string           m_Reference;
                price_type            m_Price;
                qty_type              m_Qty;
                TimestampType         m_Timestamp;
                client_id_type        m_BuyerClientID;
                client_orderid_type   m_BuyerOrderID;
                client_id_type        m_SellerClientID;
                client_orderid_type   m_SellerOrderID;
        };

        std::ostream& operator<<(std::ostream& o, const Deal & x);

        inline Deal::price_type Deal::GetPrice() const
        {
            return m_Price;
        }

        inline void Deal::SetPrice(price_type iPrice)
        {
            m_Price = std::move(iPrice);
        }

        inline Deal::qty_type Deal::GetQuantity() const
        {
            return m_Qty;
        }

        inline void Deal::SetQuantity(qty_type iQty)
        {
            m_Qty = std::move(iQty);
        }

        inline Deal::client_id_type Deal::GetBuyerClientID() const
        {
            return m_BuyerClientID;
        }

        inline void   Deal::SetBuyerClientID(client_id_type iId)
        {
            m_BuyerClientID = std::move(iId);
        }

        inline Deal::client_id_type Deal::GetSellerClientID() const
        {
            return m_SellerClientID;
        }

        inline void   Deal::SetSellerClientID(client_id_type iId)
        {
            m_SellerClientID = std::move(iId);
        }

        inline Deal::client_orderid_type Deal::GetBuyerOrderID() const
        {
            return m_BuyerOrderID;
        }

        inline void Deal::SetBuyerOrderID(client_orderid_type iId)
        {
            m_BuyerOrderID = std::move(iId);
        }

        inline Deal::client_orderid_type Deal::GetSellerOrderID() const
        {
            return m_SellerOrderID;
        }

        inline void Deal::SetSellerOrderID(client_orderid_type iId)
        {
            m_SellerOrderID = std::move(iId);
        }

        inline const std::string & Deal::GetReference() const
        {
            return m_Reference;
        }

        inline void Deal::SetReference(std::string iRef)
        {
            m_Reference = std::move(iRef);
        }

        inline Deal::TimestampType Deal::GetTimeStamp() const
        {
            return m_Timestamp;
        }
    }
}
