/*
* Copyright (C) 2014, Fabien Aulaire
* All rights reserved.
*/

#ifndef SINGLETON_HH
#define SINGLETON_HH

namespace exchange
{

    namespace common
    {

        template <typename T>
        class CSingleton
        {

        protected:

            CSingleton(){};
            ~CSingleton() {}

        public:

            static T & GetInstance()
            {
                if (NULL == _singleton)
                {
                    _singleton = new T;
                }
                return *_singleton;
            }

            static void Kill()
            {
                if (NULL != _singleton)
                {
                    delete _singleton;
                    _singleton = NULL;
                }
            }

        private:
            static T * _singleton;
        };

        template <typename T>
        T *  CSingleton<T>::_singleton;// = NULL;

    }
}

#endif
