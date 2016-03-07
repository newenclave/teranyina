#ifndef TERANYINA_RESULT_HPP
#define TERANYINA_RESULT_HPP

#include <system_error>
#include <utility>

#include "noexcept.hpp"

namespace ta {

    class error_type {

        int val_          = no_error;
        const char *mess_ = "Success";

        public:

        enum { failed = -1, no_error = 0 };

        error_type( )
        { }

        explicit error_type(const char *mess)
            :val_(failed)
            ,mess_(mess)
        { }

        explicit error_type(const char *mess, int val)
            :val_(val)
            ,mess_(mess)
        { }

        const char *message( ) const
        {
            return mess_;
        }

        int value( ) const
        {
            return val_;
        }

        operator bool ( ) const
        {
            return value( ) != no_error;
        }

    };

    template <typename T, typename Err = error_type>
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

    template <typename T>
    std::ostream & operator << ( std::ostream &o, const result_type<T> &res )
    {
        if( !res.err ) {
            o << "Ok: "   << res.result;
        } else {
            o << "Fail: " << res.err.message( );
        }
        return o;
    }

}

#endif // RESULT_HPP
