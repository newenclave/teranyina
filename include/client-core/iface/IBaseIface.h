#ifndef TA_IBASEIFACE_H
#define TA_IBASEIFACE_H

#include "vtrc-function.h"

namespace vtrc { namespace common {
    class rpc_channel;
}}

namespace ta { namespace client { namespace interfaces {

    struct base {
        virtual ~base( ) { }
        virtual       vtrc::common::rpc_channel *channel( )       = 0;
        virtual const vtrc::common::rpc_channel *channel( ) const = 0;
    };
}

}}

#endif // TA_IBASEIFACE_H
