/*
* Copyright (C) 2016, Fabien Aulaire
* All rights reserved.
*/

#pragma once

#include <ScopedExit.h>
#include <NoSqlStorage.h>

#include <Engine_Types.h>

#include <sstream>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

namespace exchange
{
    namespace engine
    {

        template <typename TOrder>
        class Instrument;

        template <typename TOrder>
        std::ostream& operator<< (std::ostream& o, const Instrument<TOrder> & x);


        template <typename TOrder>
        class Instrument
        {
            friend std::ostream& operator<< <> (std::ostream& o, const Instrument<TOrder> & x);
            
            using price_type = typename TOrder::price_type;

        public:

            Instrument() = default;

            Instrument(const std::string & name, const std::string & isin, const std::string & currency, int id, Price closeprice)
                :m_name(name), m_isin(isin), m_currency(currency), m_closeprice(closeprice), m_productid(id)
            {}

            inline bool operator==(const Instrument & rhs) const;

            const std::string & GetName() const { return m_name; }
            const std::string & GetCurrency() const { return m_currency; }
            const std::string & GetIsin() const { return m_isin; }
            price_type          GetClosePrice() const { return m_closeprice; }
            std::uint32_t       GetProductId() const { return m_productid; }

            void                SetClosePrice(price_type iprice) { m_closeprice = iprice; }

        private:

            friend class boost::serialization::access;

            template<class Archive>
            void serialize(Archive & ar, const unsigned int /* version */)
            {
                ar & m_name;
                ar & m_isin;
                ar & m_currency;
                ar & m_closeprice;
                ar & m_productid;
            }

        private:

            std::string        m_name;
            std::string        m_isin;
            std::string        m_currency;
            price_type         m_closeprice;
            std::uint32_t      m_productid;
                   
        };

        template <typename TOrder>
        inline bool Instrument<TOrder>::operator==(const Instrument & rhs) const
        {
            if (&rhs != this)
            {
                return rhs.m_name == m_name       &&
                    rhs.m_isin == m_isin       &&
                    rhs.m_currency == m_currency   &&
                    rhs.m_closeprice == m_closeprice &&
                    rhs.m_productid == m_productid;
            }
            return true;
        }

        template <typename TOrder>
        std::ostream & operator<<(std::ostream &os, const Instrument<TOrder> &gp)
        {
            return os << "Name[" << gp.GetName() << "] Isin[" << gp.GetIsin() << "] Currency[" << gp.GetCurrency() << "] "
                << "ClosePrice[" << gp.GetClosePrice() << "] ProductId[" << gp.GetProductId() << "]";
        }
        

        
        template <typename TOrder>
        class InstrumentManager : public common::NoSqlStorage< Instrument<TOrder>, common::LevelDBStorage >
        {
            private:

                using instrument_type = Instrument < TOrder > ;

            public:

                using common::NoSqlStorage< Instrument<TOrder>, common::LevelDBStorage >::NoSqlStorage;
        };


    }
}