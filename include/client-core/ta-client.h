#ifndef TA_CLIENT_H
#define TA_CLIENT_H

#include "vtrc-common/vtrc-signal-declaration.h"
#include "vtrc-common/vtrc-pool-pair.h"

namespace vtrc { namespace client {
    class vtrc_client;
}}

namespace ta { namespace client {

    class core {

        struct         impl;
        friend struct  impl;
        impl          *impl_;

    public:
        core( vtrc::common::pool_pair &pp );
        ~core( );
    };
}}

#endif // TACLIENT_H

