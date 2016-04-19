/*
* Copyright (C) 2016, Fabien Aulaire
* All rights reserved.
*/

#pragma once


#include <mutex>
#include <memory>

namespace exchange
{

    namespace common
    {

        template <class T>
        class CSingleton : private boost::noncopyable
        {
            public:

                template <typename... Args>
                static T& GetInstance(Args&&... args)
                {

                    std::call_once(  get_once_flag(),
                                [] (Args&&... args)
                                {
                                    instance_.reset(new T(std::forward<Args>(args)...));
                                },
                                std::forward<Args>(args)...
                              );

                    return *instance_.get();
                }

            protected:
                 explicit CSingleton<T>() {}
                ~CSingleton<T>() {}

            private:
                static std::unique_ptr<T> instance_;
                static std::once_flag& get_once_flag()
                {
                    static std::once_flag once_;
                    return once_;
                }
        };

        template<class T>
        std::unique_ptr<T> CSingleton<T>::instance_ = nullptr;

    }
}
