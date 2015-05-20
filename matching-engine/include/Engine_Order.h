/*
* Copyright (C) 2015, Fabien Aulaire
* All rights reserved.
*/

#pragma once

#include <iosfwd>
#include <Engine_Types.h>

namespace exchange
{
    namespace engine
    {

        /*!
        *   OrderWay
        */
        enum class OrderWay
        {
            BUY = 0,
            SELL,
            MAX_WAY
        };

        const char * OrderWayToString(OrderWay iPhase);

        /*!
        *  \brief Order
        *
        *  This class is used to enter a new order on the order book.
        */
        class Order
        {

            friend std::ostream& operator<<(std::ostream& o, const Order & x);

            public:

                using price_type   = Price;
                using qty_type     = Quantity;
                using volume_type  = Volume;
                using nominal_type = Nominal;
            
            protected:

                #pragma pack(1)
                struct Layout
                {
                    Layout(){}
                    Layout(OrderWay iWay, qty_type iQty, price_type iPrice, std::uint32_t iOrderID, std::uint32_t iClientID)
                        :m_Way(iWay), m_Qty(iQty), m_Price(iPrice), m_OrderID(iOrderID), m_ClientID(iClientID)
                    {}

                    OrderWay          m_Way      = OrderWay::MAX_WAY;
                    qty_type          m_Qty      = qty_type(0);
                    price_type        m_Price    = price_type(0);
                    std::uint32_t     m_OrderID  = 0;
                    std::uint32_t     m_ClientID = 0;
                };
                #pragma pack()

            public:

                Order(){}
                Order(OrderWay iWay, qty_type iQty, price_type iPrice, std::uint32_t iOrderID, std::uint32_t iClientID)
                    :m_Layout(iWay, iQty, iPrice, iOrderID, iClientID)
                {}

                Order(const Order & rhs) = delete;
                Order& operator=(const Order & rhs) = delete;

                inline OrderWay          GetWay()      const;
                inline qty_type          GetQuantity() const;
                inline price_type        GetPrice()    const;
                inline std::uint32_t     GetOrderID()  const;
                inline std::uint32_t     GetClientID() const;

                inline void              SetQuantity(qty_type iQty);
                inline void              SetOrderID(std::uint32_t iOrderID);
                inline void              SetPrice(price_type iPrice);
                                  
                inline void              RemoveQuantity(qty_type iDelta);

                inline bool operator==(const Order & rhs) const;

            protected:

                Layout m_Layout;
        };

        std::ostream& operator<<(std::ostream& o, const Order & x);

        inline OrderWay Order::GetWay() const
        {
            return m_Layout.m_Way;
        }

        inline Order::qty_type Order::GetQuantity() const
        {
            return m_Layout.m_Qty;
        }

        inline Order::price_type Order::GetPrice() const
        {
            return m_Layout.m_Price;
        }

        inline std::uint32_t Order::GetOrderID()  const
        {
            return m_Layout.m_OrderID;
        }

        inline std::uint32_t Order::GetClientID() const
        {
            return m_Layout.m_ClientID;
        }
        
        inline void Order::SetQuantity(qty_type iQty)
        {
            m_Layout.m_Qty = iQty;
        }

        inline void Order::SetOrderID(std::uint32_t iOrderID)
        {
            m_Layout.m_OrderID = iOrderID;
        }

        inline void Order::SetPrice(price_type iPrice)
        {
            m_Layout.m_Price = iPrice;
        }

        inline void Order::RemoveQuantity(qty_type iDelta)
        {
            m_Layout.m_Qty -= iDelta;
        }


        inline bool Order::operator==(const Order & rhs) const
        {
            if(&rhs != this)
            {
                return ( GetPrice() == rhs.GetPrice() )       &&
                       ( GetQuantity() == rhs.GetQuantity() ) &&
                       ( GetWay() == rhs.GetWay() )           &&
                       ( GetClientID() == rhs.GetClientID())  &&
                       ( GetOrderID() == rhs.GetOrderID());
            }
            return true;
        }

        /*!
        *  \brief OrderReplace
        *
        *  This class is used to modify an auto o = CREATE_ORDERn the order book.
        */
        class OrderReplace
        {
            public:

                typedef typename Order::price_type  price_type;
                typedef typename Order::qty_type    qty_type;

            protected:

                #pragma pack(1)
                    struct Layout
                    {
                        Layout(){}
                        Layout(OrderWay iWay, qty_type iQty, price_type iPrice, std::uint32_t iExistingOrderID, std::uint32_t iReplacedID, std::uint32_t iClientID)
                            :m_Way(iWay), m_Qty(iQty), m_Price(iPrice), m_ExistingOrderID(iExistingOrderID),
                             m_ReplacedOrderID(iReplacedID), m_ClientID(iClientID)
                        {}
                
                        OrderWay          m_Way             = OrderWay::MAX_WAY;
                        qty_type          m_Qty             = qty_type(0);
                        price_type        m_Price           = price_type(0);
                        std::uint32_t     m_ExistingOrderID = 0;
                        std::uint32_t     m_ReplacedOrderID = 0;
                        std::uint32_t     m_ClientID        = 0;
                    };
                #pragma pack()

            public:

                OrderReplace(){}
                OrderReplace(OrderWay iWay, qty_type iQty, price_type iPrice, std::uint32_t iExistingOrderID, std::uint32_t iReplacedID, std::uint32_t iClientID)
                    :m_Layout(iWay, iQty, iPrice, iExistingOrderID, iReplacedID, iClientID)
                {}

                inline qty_type      GetQuantity() const;
                inline void          SetQuantity(qty_type iQty);

                inline price_type    GetPrice() const;
                inline void          SetPrice(price_type iPrice);

                inline std::uint32_t GetReplacedOrderID() const;
                inline void          SetReplacedOrderID(std::uint32_t iReplacedID);

                /*
                    Read only informations. Use to retreive the existing order
                    All this attributes cannot be modified
                */
                inline std::uint32_t    GetExistingOrderID() const;
                inline void             SetExistingOrderID(std::uint32_t iExistingID);

                inline std::uint32_t    GetClientID() const;
                inline void             SetClientID(std::uint32_t iClientID);

                inline OrderWay         GetWay() const;
                inline void             SetWay(OrderWay iWay);

                inline void             RemoveQuantity(qty_type iDelta);


            protected:

                Layout m_Layout;
        };

        inline Order::qty_type OrderReplace::GetQuantity() const
        {
            return m_Layout.m_Qty;
        }

        inline void OrderReplace::SetQuantity(qty_type iQty)
        {
            m_Layout.m_Qty = iQty;
        }

        inline Order::price_type OrderReplace::GetPrice() const
        {
            return m_Layout.m_Price;
        }

        inline void OrderReplace::SetPrice(price_type iPrice)
        {
            m_Layout.m_Price = iPrice;
        }

        inline std::uint32_t OrderReplace::GetReplacedOrderID() const
        {
            return m_Layout.m_ReplacedOrderID;
        }

        inline void OrderReplace::SetReplacedOrderID(std::uint32_t iReplacedID)
        {
            m_Layout.m_ReplacedOrderID = iReplacedID;
        }

        /*
            Read only information
        */
        inline std::uint32_t OrderReplace::GetExistingOrderID() const
        {
            return m_Layout.m_ExistingOrderID;
        }

        inline void OrderReplace::SetExistingOrderID(std::uint32_t iExistingID)
        {
            m_Layout.m_ExistingOrderID = iExistingID;
        }

        inline std::uint32_t OrderReplace::GetClientID() const
        {
            return m_Layout.m_ClientID;
        }

        inline void OrderReplace::SetClientID(std::uint32_t iClientID)
        {
            m_Layout.m_ClientID = iClientID;
        }

        inline OrderWay OrderReplace::GetWay() const
        {
            return m_Layout.m_Way;
        }
        
        inline void OrderReplace::SetWay(OrderWay iWay)
        {
            m_Layout.m_Way = iWay;
        }

        inline void OrderReplace::RemoveQuantity(qty_type iDelta)
        {
            m_Layout.m_Qty -= iDelta;
        }

        /*!
        *  \brief Functor to update a field of an order
        *
        *  This functor must be provided to Index::modify method to update a specific field of an order
        *
        *  \tparam Operator : Accessor to call
        *
        */
        
        template <typename Order>
        class QuantityUpdater
        {
            public:

                using OrderType = typename std::remove_pointer<Order>::type;
                using qty_type  = typename OrderType::qty_type;

            public:

                QuantityUpdater(qty_type NewValue) :m_new_value(NewValue){}

                void operator()(Order& iOrder) const
                {
                    iOrder->SetQuantity(m_new_value);
                }

            private:

                /*!< New value to set */
                qty_type m_new_value;
        };

    }
}
