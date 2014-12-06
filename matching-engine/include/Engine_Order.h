/*
* Copyright (C) 2014, Fabien Aulaire
* All rights reserved.
*/

#pragma once

#include <iosfwd>

#include <Types.h>

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

                using price_type = UInt32;
                using qty_type   = UInt32;
            
            protected:

                #pragma pack(1)
                struct Layout
                {
                    Layout(){}
                    Layout(OrderWay iWay, qty_type iQty, price_type iPrice, UInt32 iOrderID, UInt32 iClientID)
                        :m_Way(iWay), m_Qty(iQty), m_Price(iPrice), m_OrderID(iOrderID), m_ClientID(iClientID)
                    {}

                    OrderWay   m_Way      = OrderWay::MAX_WAY;
                    qty_type   m_Qty      = 0;
                    price_type m_Price    = 0;
                    UInt32     m_OrderID  = 0;
                    UInt32     m_ClientID = 0;
                };
                #pragma pack()

            public:

                Order(){}
                Order(OrderWay iWay, qty_type iQty, price_type iPrice, UInt32 iOrderID, UInt32 iClientID)
                    :m_Layout(iWay, iQty, iPrice, iOrderID, iClientID)
                {}

                inline OrderWay   GetWay()      const;
                inline qty_type   GetQuantity() const;
                inline price_type GetPrice()    const;
                inline UInt32     GetOrderID()  const;
                inline UInt32     GetClientID() const;

                inline void       SetQuantity(qty_type iQty);
                inline void       SetOrderID(UInt32 iOrderID);
                inline void       SetPrice(price_type iPrice);
                                  
                inline void       RemoveQuantity(qty_type iDelta);

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

        inline UInt32 Order::GetOrderID()  const
        {
            return m_Layout.m_OrderID;
        }

        inline UInt32 Order::GetClientID() const
        {
            return m_Layout.m_ClientID;
        }
        
        inline void Order::SetQuantity(qty_type iQty)
        {
            m_Layout.m_Qty = iQty;
        }

        inline void Order::SetOrderID(UInt32 iOrderID)
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
        *  This class is used to modify an order on the order book.
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
                        Layout(OrderWay iWay, qty_type iQty, price_type iPrice, UInt32 iExistingOrderID, UInt32 iReplacedID, UInt32 iClientID)
                            :m_Way(iWay), m_Qty(iQty), m_Price(iPrice), m_ExistingOrderID(iExistingOrderID),
                             m_ReplacedOrderID(iReplacedID), m_ClientID(iClientID)
                        {}
                
                        OrderWay   m_Way             = OrderWay::MAX_WAY;
                        qty_type   m_Qty             = 0;
                        price_type m_Price           = 0;
                        UInt32     m_ExistingOrderID = 0;
                        UInt32     m_ReplacedOrderID = 0;
                        UInt32     m_ClientID        = 0;
                    };
                #pragma pack()

            public:

                OrderReplace(){}
                OrderReplace(OrderWay iWay, qty_type iQty, price_type iPrice, UInt32 iExistingOrderID, UInt32 iReplacedID, UInt32 iClientID)
                    :m_Layout(iWay, iQty, iPrice, iExistingOrderID, iReplacedID, iClientID)
                {}

                inline qty_type      GetQuantity() const;
                inline void          SetQuantity(qty_type iQty);

                inline price_type    GetPrice() const;
                inline void          SetPrice(price_type iPrice);

                inline UInt32        GetReplacedOrderID() const;
                inline void          SetReplacedOrderID(UInt32 iReplacedID);

                /*
                    Read only informations. Use to retreive the existing order
                    All this attributes cannot be modified
                */
                inline UInt32    GetExistingOrderID() const;
                inline void      SetExistingOrderID(UInt32 iExistingID);

                inline UInt32    GetClientID() const;
                inline void      SetClientID(UInt32 iClientID);

                inline OrderWay  GetWay() const;
                inline void      SetWay(OrderWay iWay);

                inline void RemoveQuantity(qty_type iDelta);


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

        inline UInt32 OrderReplace::GetReplacedOrderID() const
        {
            return m_Layout.m_ReplacedOrderID;
        }

        inline void OrderReplace::SetReplacedOrderID(UInt32 iReplacedID)
        {
            m_Layout.m_ReplacedOrderID = iReplacedID;
        }

        /*
            Read only information
        */
        inline UInt32 OrderReplace::GetExistingOrderID() const
        {
            return m_Layout.m_ExistingOrderID;
        }

        inline void OrderReplace::SetExistingOrderID(UInt32 iExistingID)
        {
            m_Layout.m_ExistingOrderID = iExistingID;
        }

        inline UInt32 OrderReplace::GetClientID() const
        {
            return m_Layout.m_ClientID;
        }

        inline void OrderReplace::SetClientID(UInt32 iClientID)
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
        template <void(Order::*Operator)(UInt32)>
        class OrderUpdaterSingle
        {
            public:

                OrderUpdaterSingle(UInt32 NewValue) :m_new_value(NewValue){}

                void operator()(Order& iOrder) const
                {
                    (iOrder.*Operator)(m_new_value);
                }

            private:

                /*!< New value to set */
                UInt32 m_new_value;
        };

        /*!
        *  \brief Functor to update an order
        *
        *  To update more than one field of an order this object must be construct with an OrderReplace message and this instance
        *  will be the second argument of Index::modify method.
        */
        class OrderUpdater
        {
            public:

                OrderUpdater(const OrderReplace & iOrderReplace) :
                    m_OrderReplace(iOrderReplace)
                {}

                void operator()(Order& iOrder)
                {
                    iOrder.SetOrderID(m_OrderReplace.GetReplacedOrderID());
                    iOrder.SetQuantity(m_OrderReplace.GetQuantity());
                    iOrder.SetPrice(m_OrderReplace.GetPrice());
                }

            private:
                const OrderReplace & m_OrderReplace;
        };

    }
}
