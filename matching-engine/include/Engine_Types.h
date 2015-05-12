#pragma once

#include <cstdint>

namespace exchange
{
    namespace engine
    {
        template <typename Underlying, typename Daughter>
        class Numeric
        {
        public:

            using underlying_type = Underlying;

        public:

            constexpr Numeric() noexcept = default;

            constexpr explicit Numeric(Underlying quantity) noexcept
                :m_value(quantity)
            {}

            constexpr explicit operator Underlying() noexcept
            {
                return m_value;
            }

            Underlying  AsScalar() const
            {
                return m_value;
            }

            template<class Archive>
            void serialize(Archive & ar, const unsigned int /* version */)
            {
                ar & m_value;
            }

            bool operator==(const Numeric & rhs) const  { return m_value == rhs.m_value; }
            bool operator!=(const Numeric & rhs) const  { return m_value != rhs.m_value; }
            bool operator<(const Numeric & rhs)  const  { return m_value < rhs.m_value; }
            bool operator>(const Numeric & rhs)  const  { return m_value > rhs.m_value; }
            bool operator>=(const Numeric & rhs) const  { return m_value >= rhs.m_value; }
            bool operator<=(const Numeric & rhs) const  { return m_value <= rhs.m_value; }

            Daughter operator +(const Underlying & rhs) const { return Daughter(m_value + rhs); }
            Daughter operator -(const Underlying & rhs) const { return Daughter(m_value - rhs); }
            Daughter operator +(const Numeric & rhs) const { return Daughter(m_value + rhs.m_value); }
            Daughter operator -(const Numeric & rhs) const { return Daughter(m_value - rhs.m_value); }

            Daughter operator *(double rhs) const { return Daughter(m_value * rhs); }

            Daughter& that()
            {
                return static_cast<Daughter&>(*this);
            }

            Daughter& operator -=(const Numeric & rhs)
            {
                m_value -= rhs.m_value;
                return that();
            }

            Daughter& operator +=(const Numeric & rhs)
            {
                m_value += rhs.m_value;
                return that();
            }

        private:
            Underlying m_value;
        };

        class Price : public Numeric <std::uint32_t, Price>
        {
        public:
            using Numeric<std::uint32_t, Price>::Numeric;
        };

        class Quantity : public Numeric <std::uint32_t, Quantity>
        {
        public:
            using Numeric<std::uint32_t, Quantity>::Numeric;
        };

        constexpr inline Price operator"" _price(unsigned long long int n)
        {
            return Price(n);
        }

        constexpr inline Quantity operator"" _qty(unsigned long long int n)
        {
            return Quantity(n);
        }

    }
}

