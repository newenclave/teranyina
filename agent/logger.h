#ifndef TA_AGENT_LOGGER_H
#define TA_AGENT_LOGGER_H

#include <cstdint>
#include <functional>

#include "common/logger.hxx"
#include "vtrc-common/vtrc-signal-declaration.h"
#include "vtrc-common/vtrc-noexcept.h"

namespace boost {
namespace asio {
    class io_service;
}
namespace posix_time {
    class ptime;
}
}

namespace ta { namespace agent {

    using logger_data_type   = std::vector<std::string>;
    using logger_signal_type = void ( int, /// level
                                      const boost::posix_time::ptime &tim,
                                      logger_data_type const & );

    class logger: public common::logger {

        struct  impl;
        impl   *impl_;
        VTRC_DECLARE_SIGNAL( on_write, logger_signal_type );

    public:

        using level = common::logger::level;

        logger( boost::asio::io_service &ios, level lvl,
                const char *split_string = "\n" );
        ~logger( );

        void dispatch( std::function<void ( )> call );

        static const char *level2str( level lvl, const char *def = "unk" )
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

        static level str2level( const char *str, level def = level::info )
        {
            struct lvl2str {
                const char *str_;
                level       lvl_;
            };

            static lvl2str levels[ ] =
            {
                { "zero",     level::zero    },
                { "zer",      level::zero    },
                { "ZER",      level::zero    },
                { "error",    level::error   },
                { "err",      level::error   },
                { "ERR",      level::error   },
                { "warning",  level::warning },
                { "wrn",      level::warning },
                { "WRN",      level::warning },
                { "info",     level::info    },
                { "inf",      level::info    },
                { "INF",      level::info    },
                { "debug",    level::debug   },
                { "dbg",      level::debug   },
                { "DBG",      level::debug   }
            };

            for( auto &lvl: levels ) {
                if( 0 == strcmp( lvl.str_, str ) ) {
                    return lvl.lvl_;
                }
            }
            return def;
        }

    private:

        void send_data( level lev, const std::string &data ) override;
        void send_data_nosplit( level lev, const std::string &data ) override;

        void do_write( level lvl, const boost::posix_time::ptime &tim,
                       std::string const &data, bool split ) NOEXCEPT;

    };
}}

#endif // LOGGER_H

