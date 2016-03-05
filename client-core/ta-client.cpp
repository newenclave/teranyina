#include <iostream>
#include "client-core/ta-client.h"

#include "vtrc-client/vtrc-client.h"

#include "common/utils.h"

namespace ta { namespace client {

    namespace vclient = vtrc::client;
    namespace vcomm   = vtrc::common;

    namespace {

    }

    struct core::impl {

        using vtrc_client_sptr = std::shared_ptr<vclient::vtrc_client>;

        core                    *parent_;
        vtrc_client_sptr         client_;
        utilities::endpoint_info last_connection_;
        mutable vtrc::mutex      client_lock_;

        impl( vcomm::pool_pair &pp )
            :client_(vclient::vtrc_client::create(pp))
        { }

        void init( )
        {
#define CONNECT( name, func ) name##_connect( func )
            namespace ph = std::placeholders;
            client_->CONNECT( on_connect,
                              std::bind( &impl::on_connect, this ) );

            client_->CONNECT( on_disconnect,
                              std::bind( &impl::on_disconnect, this ) );

            client_->CONNECT( on_ready,
                              std::bind( &impl::on_ready, this ) );

            client_->CONNECT( on_init_error,
                              std::bind( &impl::on_init_error, this,
                                         ph::_1, ph::_2 ) );
#undef CONNECT
        }

        void on_init_error( const vtrc::rpc::errors::container & /*ec*/,
                            const char *message )
        {
            parent_->on_init_error_( message );
        }

        void on_connect( )
        {
            parent_->on_connect_( );
        }

        void on_disconnect( )
        {
            parent_->on_disconnect_( );
        }

        void on_ready( )
        {
            parent_->on_ready_( );
        }

        void set_connection_info( const std::string &path )
        {
            auto epi = utilities::get_endpoint_info( path );
            if ( epi.is_none( ) ) {
                throw std::runtime_error( "Bad server" );
            }
            last_connection_ = epi;
        }

        void async_connect( core::async_closure_func cb )
        {
            switch( last_connection_.type ) {

            case utilities::endpoint_info::ENDPOINT_LOCAL:
                client_->async_connect( last_connection_.addpess, cb );
                break;

            case utilities::endpoint_info::ENDPOINT_TCP:
                client_->async_connect( last_connection_.addpess,
                                        last_connection_.service, cb );
                break;
            case utilities::endpoint_info::ENDPOINT_NONE:
                throw std::runtime_error( "Bad server" );
                break;
            };
        }

        void connect( )
        {
            switch( last_connection_.type ) {

            case utilities::endpoint_info::ENDPOINT_LOCAL:
                client_->connect( last_connection_.addpess );
                break;

            case utilities::endpoint_info::ENDPOINT_TCP:
                client_->connect( last_connection_.addpess,
                                  last_connection_.service );
                break;
            case utilities::endpoint_info::ENDPOINT_NONE:
                throw std::runtime_error( "Bad server" );
                break;
            };
        }
    };

    core::core( vtrc::common::pool_pair &pp )
    {
        std::unique_ptr<core::impl> uimpl(new impl(pp));
        uimpl->parent_ = this;
        uimpl->init( );
        impl_ = uimpl.release( );
    }

    core::~core( )
    {
        delete impl_;
    }

    vtrc::client::vtrc_client &core::get_client( )
    {
        return *impl_->client_;
    }

    const vtrc::client::vtrc_client &core::get_client( ) const
    {
        return *impl_->client_;
    }

    void core::connect( const std::string &server )
    {
        impl_->set_connection_info(server);
        impl_->connect( );
    }

    void core::async_connect( const std::string &server, async_closure_func cb )
    {
        impl_->set_connection_info(server);
        impl_->async_connect( cb );
    }

}}
