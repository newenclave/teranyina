
#include "client-core/iface/IScripting.h"

#include "protocol/scripting.pb.h"

#include "client-core/ta-client.h"

#include "vtrc-common/vtrc-stub-wrapper.h"
#include "vtrc-common/vtrc-rpc-channel.h"
#include "vtrc-stdint.h"

namespace ta { namespace client { namespace interfaces {

namespace {

    namespace vcomm = vtrc::common;
    typedef ta::proto::scripting::instance::Stub         stub_type;
    typedef vtrc::common::rpc_channel                    channel_type;
    typedef vcomm::stub_wrapper<stub_type, channel_type> client_type;

    struct impl: public scripting::iface {

        client_type client_;

        impl( core &cl )
            :client_(cl.get_client( ).create_channel( ), true)
        { }

        void init( ) override
        {
            client_.call( &stub_type::init );
        }

        void execute_buffer( const std::string &buf,
                             const std::string &name ) override
        {
            proto::scripting::execute_buffer_req req;
            req.set_buffer( buf );
            req.set_name( name );
            client_.call_request( &stub_type::execute_buffer, &req );
        }

        void execute_file( const std::string &path ) override
        {
            proto::scripting::execute_file_req req;
            req.set_path( path );
            client_.call_request( &stub_type::execute_file, &req );
        }

        vtrc::common::rpc_channel *channel( ) override
        {
            return client_.channel( );
        }

        const vtrc::common::rpc_channel *channel( ) const override
        {
            return client_.channel( );
        }

    };

}

    namespace scripting {
        iface_ptr create( core &cl )
        {
            return new impl( cl );
        }
    }

}}}
