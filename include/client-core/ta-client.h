#ifndef TA_CLIENT_H
#define TA_CLIENT_H

#include "vtrc-common/vtrc-pool-pair.h"
#include "vtrc-client/vtrc-client.h"
#include "vtrc-common/vtrc-closure.h"

namespace ta { namespace client {

    class core {

        struct           impl;
        friend struct    impl;
        impl            *impl_;

        VTRC_DECLARE_SIGNAL( on_init_error, void( const char *message ) );
        VTRC_DECLARE_SIGNAL( on_connect,    void( ) );
        VTRC_DECLARE_SIGNAL( on_disconnect, void( ) );
        VTRC_DECLARE_SIGNAL( on_ready,      void( ) );

    public:

        core( vtrc::common::pool_pair &pp );
        ~core( );

        vtrc::client::vtrc_client       &get_client( );
        const vtrc::client::vtrc_client &get_client( ) const;

        typedef vtrc::common::system_closure_type async_closure_func;
        void connect( const std::string &server );
        void async_connect( const std::string &server, async_closure_func cb );


    };
}}

#endif // TACLIENT_H

