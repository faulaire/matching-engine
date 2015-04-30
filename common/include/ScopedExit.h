#include <functional>

#include <boost/utility.hpp>

namespace exchange
{
    namespace common
    {
        template <typename TCallback>
        struct scope_exit : boost::noncopyable
        {
            scope_exit(const TCallback & f) : f_(f)
            {}
            
            scope_exit(scope_exit && rhs):
                f_(std::move(rhs.f_))
            {}

            ~scope_exit(void) { f_(); }
        private:
            TCallback f_;
        };


        template <typename TCallback>
        auto make_scope_exit(const TCallback & callback)
        {
            return scope_exit<TCallback>( callback );
        }

    }
}