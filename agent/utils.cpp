#include <sstream>

#include "utils.h"

namespace ta { namespace utilities {

    endpoint_info get_endpoint_info( const std::string &ep )
    {
        endpoint_info res;
        std::string   tmp_addr;

        size_t delim_pos = ep.find_last_of( ':' );

        if( delim_pos == std::string::npos ) {

            /// local: <localname>
            tmp_addr = ep;
            res.type = endpoint_info::ENDPOINT_LOCAL;

        } else {

            /// tcp: <addr>:port
            res.type = endpoint_info::ENDPOINT_TCP;
            tmp_addr.assign( ep.begin( ), ep.begin( ) + delim_pos );

            std::string svc_tmp( ep.begin( ) + delim_pos + 1, ep.end( ) );
            int port = atoi( svc_tmp.c_str( ) );
            if( port > 0 ) {
                res.service = port;
            } else {
                res.type = endpoint_info::ENDPOINT_NONE;
                return res;
            }
        }

        if( tmp_addr.size( ) > 0 && tmp_addr[0] == '@' ) {
            /// ssl !<ep>
            res.addpess.assign( tmp_addr.begin( ) + 1, tmp_addr.end( ) );
            res.flags = endpoint_info::FLAG_SSL;
        } else {
            res.addpess.assign( tmp_addr.begin( ), tmp_addr.end( ) );
        }

        return res;
    }

    std::ostream & operator << ( std::ostream &os, const endpoint_info &ei )
    {
        static const char *ssl_flag[2] = { "", "@" };

        if( !ei ) {
            os << "";
        } else if( ei.is_local( ) ) {
            os << ei.addpess;
        } else {
            os << ssl_flag[ei.is_ssl( ) ? 1 : 0]
               << ei.addpess << ":" << ei.service;
        }
        return os;
    }


}}

