#ifndef FR_COMMAND_IFACE_H
#define FR_COMMAND_IFACE_H

#include "vtrc-memory.h"
#include <string>

namespace boost { namespace program_options {

    class variables_map;
    class options_description;

}}

namespace ta {

namespace client {
    class core;
}

namespace cc {

    struct cmd_iface {

        virtual ~cmd_iface( ) { }

        virtual const char *name( ) const = 0;

        virtual void exec( boost::program_options::variables_map &vm,
                           client::core &client ) = 0;

        virtual void add_options(
                boost::program_options::options_description &desc ) = 0;

        virtual std::string help( ) const = 0;
        virtual std::string desc( ) const = 0;

        virtual bool need_connect( ) const { return true; }

    };

    typedef vtrc::shared_ptr<cmd_iface> cmd_sptr;

}}

#endif // FR_COMMAND_IFACE_H
