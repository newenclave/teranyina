#include <map>
#include <sstream>
#include <iostream>

#include "subsys-multicast.h"
#include "../application.h"
#include "protocol/protocol-constants.h"

#include "boost/asio.hpp"

namespace ta { namespace agent { namespace subsys {

    namespace {

        namespace ba   = boost::asio;
        namespace bs   = boost::system;
        namespace bip  = boost::asio::ip;


        const size_t maximum_buffer_length = 1024 * 4;
        const std::string subsys_name( "multicast" );

        struct socket_data {
            bip::udp::socket        sock_;
            std::vector<char>       data_;
            ba::ip::udp::endpoint   ep_;
            socket_data( ba::io_service &ios, size_t buf_max )
                :sock_(ios)
                ,data_(buf_max)
            { }
        };

        using socket_sptr = std::shared_ptr<socket_data>;
        using socket_wptr = std::weak_ptr<socket_data>;

        using socket_map  = std::map<std::string, socket_sptr>;

        application::service_wrapper_sptr create_service(
                                      ta::agent::application * /*app*/,
                                      vtrc::common::connection_iface_wptr cl )
        {
            ///auto inst = std::make_shared<impl_type_here>( app, cl );
            ///return app->wrap_service( cl, inst );

            return application::service_wrapper_sptr( );
        }
    }

    struct multicast::impl {

        application             *app_;
        socket_map               sockets_;
        ba::io_service::strand   dispatcher_;

        impl( application *app )
            :app_(app)
            ,dispatcher_(app_->get_io_service( ))
//            ,log_(app_->subsystem<subsys::log>( ).get_logger( ))
        { }

        void add_endpoint( const std::string &mcaddr, std::uint16_t port )
        {
            static const std::string v4any = "0.0.0.0";
            static const std::string v6any = "::";

            auto sock = std::make_shared<socket_data>
                            ( std::ref(app_->get_io_service( )),
                              maximum_buffer_length);

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
            oss << ep.address( ).to_string( ) << ":" << port;
            std::string key_string = oss.str( );

            //std::cout << "create: " << key_string << "\n";

            dispatcher_.post( [this, sock, key_string]( ) {
                sockets_[key_string] = sock;
                start_recv( sock );
            });

            //start_recv( socket_v4_, sender_endpoint_v4_ );
        }

        void handler( bs::error_code err, size_t total, socket_wptr wsock )
        {
            auto sock(wsock.lock( ));
            if( !sock ) {
                return;
            }

            if( err ) {
                return;
            }

            start_recv( sock );
        }

        void start_recv( socket_sptr sock )
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
                            socket_wptr(sock)
                        )
                    )
                );

            } catch( const std::exception& ex ) {
                //std::cerr << "mc error: " << ex.what( ) << "\n";
            }
        }

        void reg_creator( const std::string &name,
                          application::service_getter_type func )
        {
            app_->register_service_creator( name, func );
        }

        void unreg_creator( const std::string &name )
        {
            app_->unregister_service_creator( name );
        }

    };


    multicast::multicast( application *app )
        :impl_(new impl(app))
    { }

    multicast::~multicast( )
    {
        delete impl_;
    }

    /// static
    vtrc::shared_ptr<multicast> multicast::create( application *app )
    {
        vtrc::shared_ptr<multicast> new_inst(new multicast(app));
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
//        impl_->add_endpoint( proto::multicast::default_address_v4,
//                             proto::multicast::default_port );
//        impl_->add_endpoint( proto::multicast::default_address_v6,
//                             proto::multicast::default_port );
//        impl_->LOGINF << "Started.";
    }

    void multicast::stop( )
    {
//        impl_->LOGINF << "Stopped.";
    }


}}}

    
