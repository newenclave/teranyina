#include <sstream>

#include "utils.h"

namespace {

    enum color_value {
         COLOR_NONE     = 0
        ,COLOR_RED      = 1
        ,COLOR_GREEN    = 2
        ,COLOR_BLUE     = 3
        ,COLOR_YELLOW   = 4
        ,COLOR_WHITE    = 5
        ,COLOR_CYAN     = 6
    };

#ifdef _WIN32

    enum colors_enum
    {
        Black       = 0,
        Grey        = FOREGROUND_INTENSITY,
        LightGrey   = FOREGROUND_RED   | FOREGROUND_GREEN
                                       | FOREGROUND_BLUE,
        White       = FOREGROUND_RED   | FOREGROUND_GREEN
                                       | FOREGROUND_BLUE
                                       | FOREGROUND_INTENSITY,
        Blue        = FOREGROUND_BLUE,
        Green       = FOREGROUND_GREEN,
        Cyan        = FOREGROUND_GREEN | FOREGROUND_BLUE,
        Red         = FOREGROUND_RED,
        Purple      = FOREGROUND_RED   | FOREGROUND_BLUE,
        LightBlue   = FOREGROUND_BLUE  | FOREGROUND_INTENSITY,
        LightGreen  = FOREGROUND_GREEN | FOREGROUND_INTENSITY,
        LightCyan   = FOREGROUND_GREEN | FOREGROUND_BLUE
                                       | FOREGROUND_INTENSITY,
        LightRed    = FOREGROUND_RED   | FOREGROUND_INTENSITY,
        LightPurple = FOREGROUND_RED   | FOREGROUND_BLUE
                                       | FOREGROUND_INTENSITY,
        Orange      = FOREGROUND_RED   | FOREGROUND_GREEN,
        Yellow      = FOREGROUND_RED   | FOREGROUND_GREEN
                                       | FOREGROUND_INTENSITY,
    };

    const unsigned color_map[ ] = {
        LightGrey,
        LightRed, LightGreen, LightBlue,
        Yellow, White, Cyan
    };
#else

    static const char * cp_none      = "\033[0m";
    //static const char * cp_none      = "\x1b[0m";
    //static const char * cp_black     = "\x1b[30;1m";
    static const char * cp_red       = "\x1b[31;1m";
    static const char * cp_green     = "\x1b[32;1m";
    static const char * cp_yellow    = "\x1b[33;1m";
    //static const char * cp_orange    = "\x1b[33;1m";
    static const char * cp_blue      = "\x1b[34;1m";
    //static const char * cp_purple    = "\x1b[35;1m";
    static const char * cp_cyan      = "\x1b[36;1m";
    static const char * cp_white     = "\x1b[37;1m";

    const char *color_map[ ] = {
        cp_none,
        cp_red, cp_green, cp_blue,
        cp_yellow, cp_white, cp_cyan
    };
#endif
    const size_t color_count = sizeof(color_map)/sizeof(color_map[0]);

    std::ostream &set_stream_color( std::ostream &s, unsigned color )
    {
        if( color >= color_count ) {
            color = COLOR_NONE;
        }
#ifdef _WIN32
        SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ),
                                 color_map[color] );
#else
        s << color_map[color];
#endif
        return s;
    }
}

namespace ta { namespace utilities {

    namespace console {
        std::ostream &light ( std::ostream &s )
        {
            return set_stream_color(s, COLOR_WHITE);
        }

        std::ostream &red   ( std::ostream &s )
        {
            return set_stream_color(s, COLOR_RED);
        }

        std::ostream &green ( std::ostream &s )
        {
            return set_stream_color(s, COLOR_GREEN);
        }

        std::ostream &blue  ( std::ostream &s )
        {
            return set_stream_color(s, COLOR_BLUE);
        }

        std::ostream &yellow( std::ostream &s )
        {
            return set_stream_color(s, COLOR_YELLOW);
        }

        std::ostream &none  ( std::ostream &s )
        {
            return set_stream_color(s, COLOR_NONE);
        }

        std::ostream &cyan  ( std::ostream &s )
        {
            return set_stream_color(s, COLOR_CYAN);
        }
    }

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

