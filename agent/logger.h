#ifndef TA_AGENT_LOGGER_H
#define TA_AGENT_LOGGER_H

#include <cstdint>

#include "common/logger.hxx"
#include "vtrc-common/vtrc-signal-declaration.h"

namespace ta { namespace agent {

    using logger_signal_type = void ( common::log_level, std::uint64_t,
                                      std::string const & );

    class logger: public common::logger {

        VTRC_DECLARE_SIGNAL( on_write, logger_signal_type );
    public:
        using level = common::log_level;
        logger( level lvl );
    private:
        virtual void send_data( level lev, const std::string &data );
    };
}}

#endif // LOGGER_H

