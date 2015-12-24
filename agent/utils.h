#ifndef TA_UTILS_H
#define TA_UTILS_H

#include <string>

namespace ta { namespace utilities {

    struct endpoint_info {
        enum ep_type {
             ENDPOINT_LOCAL  = 1
            ,ENDPOINT_TCP    = 2
        };
        enum ep_flags {
             FLAG_SSL = 0x01
        };
        std::string addpess;
        std::string service;
        unsigned    flags = 0;
    };

    ///  0.0.0.0:12345            - tcp endpoint (address:port)
    /// !0.0.0.0:12345            - tcp + ssl endpoint( !address:port )
    ///  /ome/data/teranyina.sock - local endpoint
    ///                                (/local/socket or \\.\pipe\name )
    endpoint_info get_endpoint_info( const std::string &ep );

}}

#endif // UTILS_H
