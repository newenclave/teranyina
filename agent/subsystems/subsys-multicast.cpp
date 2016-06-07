#include <map>
#include <sstream>
#include <iostream>

#include "subsys-multicast.h"
#include "../application.h"
#include "protocol/protocol-constants.h"

#include "protocol/multicast.pb.h"

#include "boost/asio.hpp"

#include "common/utils.h"

#define LOG(lev) log_(lev, "multcast")
#define LOGINF   LOG(logger::level::info)
#define LOGDBG   LOG(logger::level::debug)
#define LOGERR   LOG(logger::level::error)
#define LOGWRN   LOG(logger::level::warning)
namespace ta { namespace agent { namespace subsys {

    namespace {

        namespace ba   = boost::asio;
        namespace bs   = boost::system;
        namespace bip  = boost::asio::ip;

        const size_t maximum_buffer_length = 1024 * 4;
        const std::string subsys_name( "multicast" );

        struct socket_data {
            bip::udp::socket      sock_;
            std::vector<char>     data_;
            ba::ip::udp::endpoint ep_;
            socket_data( ba::io_service &ios, size_t buf_max )
                :sock_(ios)
                ,data_(buf_max)
            { }
        };

        using socket_data_sptr = std::shared_ptr<socket_data>;
        using socket_data_wptr = std::weak_ptr<socket_data>;
        using socket_map       = std::map<std::string, socket_data_sptr>;
        using string_vector    = std::vector<std::string>;

//        application::service_wrapper_sptr create_service(
//                                      ta::agent::application * /*app*/,
//                                      vtrc::common::connection_iface_wptr/*cl*/)
//        {
//            ///auto inst = std::make_shared<impl_type_here>( app, cl );
//            ///return app->wrap_service( cl, inst );

//            return application::service_wrapper_sptr( );
//        }
    }

    struct multicast::impl {

        application             *app_;
        agent::logger           &log_;
        multicast               *parent_;
        socket_map               sockets_;
        ba::io_service::strand   dispatcher_;
        string_vector            default_points_;

        impl( application *app )
            :app_(app)
            ,log_(app_->get_logger( ))
            ,dispatcher_(app_->get_io_service( ))
        { }

        void add_endpoint( const std::string &ep_str )
        {
            static const std::string v4any = "0.0.0.0";
            static const std::string v6any = "::";

            std::string     mcaddr;
            std::uint16_t   port = 0;

            if( ep_str == "def4" ) {
                mcaddr = proto::multicast::default_address_v4;
                port   = proto::multicast::default_port;
            } else if( ep_str == "def6" ) {
                mcaddr = proto::multicast::default_address_v6;
                port   = proto::multicast::default_port;
            } else {
                auto ep_info = utilities::get_endpoint_info( ep_str );
                if( ep_info && !ep_info.is_local( ) ) {
                    mcaddr = ep_info.addpess;
                    port   = ep_info.service;
                }
            }

            auto sock = std::make_shared<socket_data>
                            ( std::ref(app_->get_io_service( )),
                              maximum_buffer_length );

            auto mca = ba::ip::address::from_string(mcaddr);

            bip::udp::endpoint ep( bip::address::from_string( mca.is_v4( )
                                                            ? v4any
                                                            : v6any ), port );

            sock->sock_.open( mca.is_v4( ) ? bip::udp::v4( ) : bip::udp::v6( ));
            sock->sock_.set_option( bip::udp::socket::reuse_address(true) );
            sock->sock_.bind(ep);

            // Join the multicast group.
            sock->sock_.set_option(bip::multicast::join_group(mca));

            std::ostringstream oss;
            oss << mca.to_string( ) << ":" << port;
            std::string key_string = oss.str( );

            LOGINF << "create multicast receiver: " << key_string << "\n";
            dispatcher_.post( [this, sock, key_string]( ) {
                LOGINF << "start multicast receiver: " << key_string << "\n";
                sockets_[key_string] = sock;
                start_recv( sock );
            });

            //start_recv( socket_v4_, sender_endpoint_v4_ );
        }

        void handler( bs::error_code err, size_t total, socket_data_wptr wsock )
        {
            auto sock(wsock.lock( ));
            if( !sock ) {
                return;
            }

            if( err ) {
                return;
            }

            using ping_req = ta::proto::multicast::ping;
            using pong_res = ta::proto::multicast::pong;

            ping_req input;
            pong_res output;

            input.ParseFromArray( &sock->data_[0], total );
            output.set_active( true );

            try {
                parent_->on_new_request_( sock->ep_.address( ).to_string( ),
                                          sock->ep_.port( ),
                                          input, output );
                if( output.active( ) ) {
                    auto buf = output.SerializeAsString( );
                    sock->sock_.send_to( ba::buffer( buf ), sock->ep_ );
                }
            } catch( const std::exception &ex ) {
                LOGERR << "multicast handler failed; " << ex.what( );
                ;;;; /// log here
            }

            start_recv( sock );
        }

        void start_recv( socket_data_sptr sock )
        {
            namespace ph = std::placeholders;
            sock->ep_ = ba::ip::udp::endpoint( );
            try {
                sock->sock_.async_receive_from(
                    ba::buffer( sock->data_, sock->data_.size( ) ),
                    sock->ep_,
                    dispatcher_.wrap(
                        std::bind( &impl::handler, this,
                            ph::_1, ph::_2,
                            socket_data_wptr(sock)
                        )
                    )
                );

            } catch( const std::exception& ex ) {
                LOGERR << "failed to start receiving; " << ex.what( );
            }
        }

        void reg_creator( const std::string &name,
                          application::service_getter_type func )
        {
            app_->register_service_factory( name, func );
        }

        void unreg_creator( const std::string &name )
        {
            app_->unregister_service_factory( name );
        }

        void start_all( )
        {
            for( auto &e: default_points_ ) {
                add_endpoint( e );
            }
        }

        void stop_all( )
        {
            sockets_.clear( );
        }
    };


    multicast::multicast( application *app )
        :impl_(new impl(app))
    {
        impl_->parent_ = this;
    }

    multicast::~multicast( )
    {
        delete impl_;
    }

    /// static
    multicast::shared_type multicast::create( application *app,
                                              const string_vector &def )
    {
        shared_type new_inst(new multicast(app));
        new_inst->impl_->default_points_.assign( def.begin( ), def.end( ) );
        return new_inst;
    }

    const std::string &multicast::name( )  const
    {
        return subsys_name;
    }

    void multicast::init( )
    {

    }

    void multicast::start( )
    {
        impl_->start_all( );
//        impl_->add_endpoint( proto::multicast::default_address_v4,
//                             proto::multicast::default_port );
//        impl_->add_endpoint( proto::multicast::default_address_v6,
//                             proto::multicast::default_port );
        impl_->LOGINF << "Started";
    }

    void multicast::stop( )
    {
        impl_->stop_all( );
        impl_->LOGINF << "Stopped";
    }

}}}

    
