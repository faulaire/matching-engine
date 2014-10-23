namespace exchange
{
    namespace common
    {
        namespace tools
        {

            template <typename Array>
            void to_base64(UInt64 iToEncode, Array & oEncoded, size_t FirstPos)
            {
                static constexpr const char digits_array[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                                             "abcdefghijklmnopqrstuvwxyz"
                                                             "0123456789+/";
                auto mod = iToEncode % 64;
                do
                {
                    oEncoded[FirstPos] = digits_array[mod];
                    FirstPos--;

                    iToEncode = iToEncode / 64;
                    mod = iToEncode % 64;
                }
                while (mod != 0);
            }
        }
    }
    
}
