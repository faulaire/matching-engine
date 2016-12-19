#pragma once

#include <vector>
#include <cstdint>

namespace exchange
{
    namespace gateway
    {
        class Message
        {
        public:

            union Header
            {
                unsigned char m_Buffer[4];
                std::uint32_t m_BodyLength = 0;
            };

            static_assert(sizeof(Header) == 4);

        public:

            static constexpr std::uint32_t header_length = sizeof(Header);

        public:
            Message() = default;

            unsigned char * body()     { return &m_data[0]; }
            unsigned char * header()   { return m_header.m_Buffer; }
            size_t body_length() const { return m_header.m_BodyLength; }

            bool decode_header()
            {
                m_data.resize(m_header.m_BodyLength);
                return true;
            }

        private:
            Header                     m_header;
            std::vector<unsigned char> m_data = {};
        };
    }
}