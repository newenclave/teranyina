#ifndef TA_UTILS_H
#define TA_UTILS_H

#include <string>
#include <cstdint>
#include <ostream>

#include "vtrc-common/vtrc-noexcept.h"

namespace ta { namespace utilities {

    template <typename T>
    struct result_type {

        T result;
        bool ok;

        result_type(const result_type &o)
            :result(o.result)
            ,ok(o.ok)
        { }

        result_type(result_type &&o)
            :result(std::move(o.result))
            ,ok(o.ok)
        { }

        result_type( )
            :ok(false)
        { }

        result_type( bool f )
            :ok(f)
        { }

        result_type( bool f, const T &s )
            :result(s)
            ,ok(f)
        { }

        result_type( bool f, T &&s )
            :result(s)
            ,ok(f)
        { }

        operator bool ( ) const
        {
            return ok;
        }
    };

    result_type<std::string> bin2hex( void const *bytes, size_t length );
    result_type<std::string> bin2hex( std::string const &input );
    result_type<std::string> hex2bin( std::string const &input );

    template <typename T>
    std::ostream & operator << ( std::ostream &o, const result_type<T> &res )
    {
        o << ( res ? "Ok: " : "Fail: " ) << res.result;
        return o;
    }

    namespace console {
        std::ostream &light ( std::ostream &s );
        std::ostream &red   ( std::ostream &s );
        std::ostream &green ( std::ostream &s );
        std::ostream &blue  ( std::ostream &s );
        std::ostream &cyan  ( std::ostream &s );
        std::ostream &yellow( std::ostream &s );
        std::ostream &none  ( std::ostream &s );
    }

    struct endpoint_info {
        enum ep_type {
             ENDPOINT_NONE   = 0
            ,ENDPOINT_LOCAL  = 1
            ,ENDPOINT_TCP    = 2
        };
        enum ep_flags {
             FLAG_SSL = 0x01
        };
        std::string    addpess;
        std::uint16_t  service = 0;
        unsigned       flags   = 0;
        ep_type        type    = ENDPOINT_NONE;

        bool is_local( ) const NOEXCEPT
        {
            return type == ENDPOINT_LOCAL;
        }

        bool is_none( ) const NOEXCEPT
        {
            return type == ENDPOINT_NONE;
        }

        bool is_ssl( ) const NOEXCEPT
        {
            return flags & FLAG_SSL;
        }

        operator bool( ) const NOEXCEPT
        {
            return !is_none( );
        }

    };

    std::ostream & operator << ( std::ostream &os, const endpoint_info &ei );

    ///  0.0.0.0:12345             - tcp  endpoint (address:port)
    ///  0.0.0.0:12345             - tcp6 endpoint (address:port)
    /// @0.0.0.0:12345             - tcp  + ssl endpoint( !address:port )
    /// @:::12345                  - tcp6 + ssl endpoint( !address:port )
    ///  /home/data/teranyina.sock - local endpoint
    ///                                (/local/socket or \\.\pipe\name )
    endpoint_info get_endpoint_info( const std::string &ep );

}}


#endif // UTILS_H
