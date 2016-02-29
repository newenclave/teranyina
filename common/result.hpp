#ifndef TERANYINA_RESULT_HPP
#define TERANYINA_RESULT_HPP

#include <system_error>
#include <utility>

#include "noexcept.hpp"

namespace ta {

    class error_message {

        const char *mess_ = "Success";

        public:

        error_message( )
        { }

        explicit error_message(const char *mess)
            :mess_(mess)
        { }

        const char *message( ) const
        {
            return mess_;
        }

        int value( ) const
        {
            return -1;
        }

    };

    template <typename T, typename Err = error_message>
    struct result_type {

        typedef Err error_type;
        typedef T   value_type;

        value_type result;
        error_type err;

        result_type( )
        { }

        result_type( T &&res )
            :result(std::forward<T>(res))
        { }

        result_type( T &&res, Err e )
            :result(std::forward<T>(res))
            ,err(e)
        { }
    };
}

#endif // RESULT_HPP
