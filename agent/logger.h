#ifndef TA_AGENT_LOGGER_H
#define TA_AGENT_LOGGER_H

#include <cstdint>

#include "common/logger.hxx"
#include "vtrc-common/vtrc-signal-declaration.h"

namespace boost { namespace asio {
    class io_service;
}}

namespace ta { namespace agent {

    using logger_signal_type = void ( common::logger::level, std::uint64_t,
                                      std::string const & ) noexcept;

    class logger: public common::logger {

        struct  impl;
        impl   *impl_;
        VTRC_DECLARE_SIGNAL( on_write, logger_signal_type );

    public:

        using level = common::logger::level;

        logger( boost::asio::io_service &ios, level lvl );
        ~logger( );

        static const char *level2str( level lvl,
                                      const char *def = "unk" )
        {
            switch( lvl ) {
            case level::zero:
                return "zer";
            case level::error:
                return "err";
            case level::warning:
                return "wrn";
            case level::info:
                return "inf";
            case level::debug:
                return "dbg";
            default:
                return def;
            }
        }

        static level str2level( const char *str,
                                level def = level::info )
        {
            struct lvl2str {
                const char *str_;
                level       lvl_;
            };

            static lvl2str levels[ ] =
            {
                { "zero",     level::zero    },
                { "zro",      level::zero    },
                { "error",    level::error   },
                { "err",      level::error   },
                { "warning",  level::warning },
                { "wrn",      level::warning },
                { "info",     level::info    },
                { "inf",      level::info    },
                { "debug",    level::debug   },
                { "dbg",      level::debug   }
            };

            for( auto &lvl: levels ) {
                if( 0 == strcmp( lvl.str_, str ) ) {
                    return lvl.lvl_;
                }
            }
            return def;
        }

    private:
        virtual void send_data( level lev, const std::string &data );
        void do_write( level lvl, std::uint64_t microsec,
                       std::string const &data ) noexcept;

    };
}}

#endif // LOGGER_H

