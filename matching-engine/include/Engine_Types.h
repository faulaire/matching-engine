#pragma once

#include <cstdint>
#include <limits>
#include <iosfwd>

namespace exchange
{
    namespace engine
    {

        template <typename Underlying, typename Daughter>
        class Numeric;

        template <typename Underlying, typename Daughter>
        std::ostream& operator<< (std::ostream& o, const Numeric<Underlying, Daughter> & x);

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

            constexpr Underlying  AsScalar() const
            {
                return m_value;
            }

            template<class Archive>
            void serialize(Archive & ar, const unsigned int /* version */)
            {
                ar & m_value;
            }

            constexpr bool operator==(const Numeric & rhs) const  { return m_value == rhs.m_value; }
            constexpr bool operator!=(const Numeric & rhs) const  { return m_value != rhs.m_value; }
            constexpr bool operator<(const Numeric & rhs)  const  { return m_value < rhs.m_value; }
            constexpr bool operator>(const Numeric & rhs)  const  { return m_value > rhs.m_value; }
            constexpr bool operator>=(const Numeric & rhs) const  { return m_value >= rhs.m_value; }
            constexpr bool operator<=(const Numeric & rhs) const  { return m_value <= rhs.m_value; }

            constexpr Daughter operator +(const Numeric & rhs) const { return Daughter(m_value + rhs.m_value); }
            constexpr Daughter operator -(const Numeric & rhs) const { return Daughter(m_value - rhs.m_value); }

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

            static constexpr Daughter max()
            {
                return Daughter(std::numeric_limits<Underlying>::max());
            }

            static constexpr Daughter min()
            {
                return Daughter( std::numeric_limits<Underlying>::min() );
            }

        protected:

            Daughter& that()
            {
                return static_cast<Daughter&>(*this);
            }

            Underlying m_value = 0;
        };


        template <typename Underlying, typename Daughter>
        std::ostream& operator<< (std::ostream& oss, const Numeric<Underlying, Daughter> & x)
        {
            oss << x.AsScalar();
            return oss;
        }

        class Price : public Numeric <std::uint32_t, Price>
        {
        public:
            using Numeric<std::uint32_t, Price>::Numeric;

        public:
            constexpr Price operator *(double rhs) const { return Price(m_value * rhs); }
        };

        class Volume;
        class Nominal;

        class Quantity : public Numeric <std::uint32_t, Quantity>
        {
        public:
            using Numeric<std::uint32_t, Quantity>::Numeric;
        public:
            constexpr explicit operator Volume() noexcept;

            constexpr Nominal operator *(const Price & rhs);
        };
        
        class Volume : public Numeric < std::uint64_t, Volume >
        {
        public:
            using Numeric<std::uint64_t, Volume>::Numeric;
        public:
            constexpr Volume operator +(const Quantity & rhs) const { return Volume( m_value + rhs.AsScalar() ); }

            Volume& operator +=(const Quantity & rhs)
            {
                m_value += rhs.AsScalar();
                return *this;
            }

            Volume& operator -=(const Quantity & rhs)
            {
                m_value -= rhs.AsScalar();
                return *this;
            }
        };

        class Nominal : public Numeric < std::uint64_t, Nominal >
        {
        public:
            using Numeric<std::uint64_t, Nominal>::Numeric;
        };

        constexpr Nominal Quantity::operator *(const Price & rhs)
        {
            return Nominal(m_value * rhs.AsScalar());
        }

        constexpr Quantity::operator Volume() noexcept
        {
            return Volume(m_value);
        }

        inline Price operator"" _price(unsigned long long int n)
        {
            return Price(n);
        }

        inline Quantity operator"" _qty(unsigned long long int n)
        {
            return Quantity(n);
        }

        inline Volume operator"" _volume(unsigned long long int n)
        {
            return Volume(n);
        }

        inline Nominal operator"" _nominal(unsigned long long int n)
        {
            return Nominal(n);
        }

    }
}

