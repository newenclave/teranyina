#ifndef TA_UTILS_H
#define TA_UTILS_H

#include <string>
#include <ostream>

namespace ta { namespace utilities {

    struct endpoint_info {
        enum ep_type {
             ENDPOINT_NONE   = 0
            ,ENDPOINT_LOCAL  = 1
            ,ENDPOINT_TCP    = 2
        };
        enum ep_flags {
             FLAG_SSL = 0x01
        };
        std::string addpess;
        std::string service;
        unsigned    flags = 0;
        ep_type     type  = ENDPOINT_NONE;

        bool is_local( ) const noexcept
        {
            return type == ENDPOINT_LOCAL;
        }

        bool is_ssl( ) const noexcept
        {
            return flags & FLAG_SSL;
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
