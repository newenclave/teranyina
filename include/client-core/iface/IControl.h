#ifndef ITAAGENTCONTROL_H
#define ITAAGENTCONTROL_H

#include "IBaseIface.h"

namespace ta { namespace client {
    class core;
}}

namespace ta { namespace client {


namespace interfaces { namespace control {

    struct iface: public base {

        virtual ~iface( ) { }
        virtual void shutdown( ) const = 0;
        virtual void ping( ) const = 0;

    };

    iface *create( ta::client::core &client );

}}}}

#endif // IASYNCOPERATION_H
