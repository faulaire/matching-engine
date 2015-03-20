/*
* Copyright (C) 2015, Fabien Aulaire
* All rights reserved.
*/

#pragma once

#include <array>
#include <cstring>
#include <cassert>
#include <chrono>
#include <iosfwd>
#include <utility>
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

                using price_type = Order::price_type;
                using qty_type   = Order::qty_type;

            public:

                Deal(price_type iPrice, qty_type iQty, std::uint32_t iBuyerClientID, std::uint32_t iBuyerOrderID, std::uint32_t iSellerClientID, std::uint32_t iSellerOrderID);

                Deal() = delete;

                bool operator==(const Deal & rhs) const;

            public:

                inline price_type            GetPrice() const;
                inline void                  SetPrice(price_type iPrice);
                                             
                inline qty_type              GetQuantity() const;
                inline void                  SetQuantity(qty_type iQty);
                                             
                inline std::uint32_t         GetBuyerClientID() const;
                inline void                  SetBuyerClientID(std::uint32_t iId);
                                             
                inline std::uint32_t         GetSellerClientID() const;
                inline void                  SetSellerClientID(std::uint32_t iId);
                                             
                inline std::uint32_t         GetBuyerOrderID() const;
                inline void                  SetBuyerOrderID(std::uint32_t iId);
                                             
                inline std::uint32_t         GetSellerOrderID() const;
                inline void                  SetSellerOrderID(std::uint32_t iId);
                
                inline TimestampType         GetTimeStamp() const;

                inline const std::string &   GetReference() const;
                inline void                  SetReference(std::string);

            protected:
                std::string          m_Reference;
                price_type           m_Price;
                qty_type             m_Qty;
                TimestampType        m_Timestamp;
                std::uint32_t        m_BuyerClientID;
                std::uint32_t        m_BuyerOrderID;
                std::uint32_t        m_SellerClientID;
                std::uint32_t        m_SellerOrderID;
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

        inline std::uint32_t Deal::GetBuyerClientID() const
        {
            return m_BuyerClientID;
        }

        inline void   Deal::SetBuyerClientID(std::uint32_t iId)
        {
            m_BuyerClientID = std::move(iId);
        }

        inline std::uint32_t Deal::GetSellerClientID() const
        {
            return m_SellerClientID;
        }

        inline void   Deal::SetSellerClientID(std::uint32_t iId)
        {
            m_SellerClientID = std::move(iId);
        }

        inline std::uint32_t Deal::GetBuyerOrderID() const
        {
            return m_BuyerOrderID;
        }

        inline void Deal::SetBuyerOrderID(std::uint32_t iId)
        {
            m_BuyerOrderID = std::move(iId);
        }

        inline std::uint32_t Deal::GetSellerOrderID() const
        {
            return m_SellerOrderID;
        }

        inline void Deal::SetSellerOrderID(std::uint32_t iId)
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
