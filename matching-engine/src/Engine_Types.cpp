#include <Engine_Types.h>


namespace exchange
{
    namespace engine
    {

        Price operator"" _price(unsigned long long int n)
        {
            return Price(n);
        }

        Quantity operator"" _qty(unsigned long long int n)
        {
            return Quantity(n);
        }

    }
}