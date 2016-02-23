#ifndef ITAAGENTCONTROL_H
#define ITAAGENTCONTROL_H

#include "IBaseIface.h"

namespace ta { namespace client {
    class core;
}}

namespace ta { namespace client { namespace interfaces {

    struct control: public base {

        virtual ~control( ) { }
        virtual void shutdown( ) const = 0;

        static control *create( ta::client::core &client );
    };

}}}

#endif // IASYNCOPERATION_H
