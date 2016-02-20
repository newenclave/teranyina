#include "client-core/iface/IControl.h"
#include "protocol/ctrl.pb.h"

#include "client-core/ta-client.h"

#include "vtrc-common/vtrc-stub-wrapper.h"

namespace ta { namespace client { namespace interfaces {

namespace {

    namespace vcomm   = vtrc::common;
    namespace vclient = vtrc::client;

    struct impl: public control {

        typedef ta::proto::ctrl_Stub                               stub_type;
        typedef vcomm::stub_wrapper<stub_type, vcomm::rpc_channel> client_stub;

        mutable client_stub channel_;

        impl(ta::client::core &client)
            :channel_(client.get_client( ).create_channel( ), true)
        { }

        vcomm::rpc_channel *channel( )
        {
            return channel_.channel( );
        }

        const vcomm::rpc_channel *channel( ) const
        {
            return channel_.channel( );
        }

        void shutdown( ) const
        {
            channel_.channel( )->set_flags( vcomm::rpc_channel::DISABLE_WAIT );
            channel_.call( &stub_type::shutdown );
        }

    };

}

    control *control::create( ta::client::core &client )
    {
        return new impl(client);
    }

}}}
