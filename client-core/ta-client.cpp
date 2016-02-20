#include <iostream>
#include "client-core/ta-client.h"

#include "vtrc-client/vtrc-client.h"

namespace ta { namespace client {

    namespace vclient = vtrc::client;
    namespace vcomm   = vtrc::common;

    namespace {

        enum connection_name { CONN_NONE, CONN_PIPE, CONN_TCP };

        struct connect_info {
            connection_name     name_;
            std::string         server_;
            unsigned short      port_;
            bool                tcp_nowait_;
        };

        typedef vtrc::shared_ptr<connect_info> connect_info_sptr;

        connect_info_sptr get_connect_info( std::string const &name, bool nw )
        {
            size_t delim_pos = name.find_last_of( ':' );

            connect_info_sptr ci(vtrc::make_shared<connect_info>( ));

            ci->name_ = CONN_NONE;

            if( delim_pos == std::string::npos ) {

                /// local: <localname>
                ci->name_ = CONN_PIPE;
                ci->server_ = name;

            } else {

                ci->name_ = CONN_TCP;
                ci->server_.assign( name.begin( ),
                                    name.begin( ) + delim_pos );
                ci->port_ = atoi( name.c_str( ) + delim_pos + 1 );
                ci->tcp_nowait_ = nw;

            }
            return ci;
        }
    }

    struct core::impl {

        std::shared_ptr<vclient::vtrc_client> client_;
        connect_info_sptr                     last_connection_;
        mutable vtrc::mutex                   client_lock_;

        impl( vcomm::pool_pair &pp )
            :client_(vclient::vtrc_client::create(pp))
        { }

        void set_connection_info( const std::string &path )
        {
            connect_info_sptr res = get_connect_info(path, true);
            if( res->name_ == CONN_NONE ) {
                throw std::runtime_error( "Bad server" );
            }
            last_connection_ = res;
        }

        void connect( )
        {
            connect_info_sptr cl = last_connection_;
            switch( cl->name_ ) {

            case CONN_PIPE:
                client_->connect( cl->server_ );
                break;

            case CONN_TCP:
                client_->connect( cl->server_, cl->port_ );
                break;

            case CONN_NONE:
                throw std::runtime_error( "Bad server" );
                break;
            };
        }
    };

    core::core( vtrc::common::pool_pair &pp )
        :impl_(new impl(pp))
    { }

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


}}
