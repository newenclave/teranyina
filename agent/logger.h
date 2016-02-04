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

    private:
        virtual void send_data( level lev, const std::string &data );
        void do_write( level lvl, std::uint64_t microsec,
                       std::string const &data ) noexcept;

    };
}}

#endif // LOGGER_H

