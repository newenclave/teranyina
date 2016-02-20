#ifndef TA_CLIENT_H
#define TA_CLIENT_H

#include "vtrc-common/vtrc-signal-declaration.h"
#include "vtrc-common/vtrc-pool-pair.h"
#include "vtrc-client/vtrc-client.h"

namespace ta { namespace client {

    class core {

        struct           impl;
        friend struct    impl;
        impl            *impl_;

    public:
        core( vtrc::common::pool_pair &pp );
        ~core( );

        vtrc::client::vtrc_client       &get_client( );
        const vtrc::client::vtrc_client &get_client( ) const;

    };
}}

#endif // TACLIENT_H

