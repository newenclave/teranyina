#include <iostream>
#include <list>
#include <memory>

#include "subsys-listerens.h"
#include "../application.h"

#include "vtrc-server/vtrc-listener.h"
#include "vtrc-server/vtrc-listener-tcp.h"
#include "vtrc-server/vtrc-listener-ssl.h"
#include "vtrc-server/vtrc-listener-local.h"

#include "utils.h"

#include "boost/system/error_code.hpp"

//#include "subsys-log.h"

//#include "vtrc-memory.h"

//#define LOG(lev) log_(lev) << "[listerens] "
//#define LOGINF   LOG(logger::info)
//#define LOGDBG   LOG(logger::debug)
//#define LOGERR   LOG(logger::error)
//#define LOGWRN   LOG(logger::warning)

namespace ta { namespace agent { namespace subsys {

    namespace {

        namespace vcomm = vtrc::common;
        namespace vserv = vtrc::server;
        namespace bsys  = boost::system;

        using string_vector  = std::vector<std::string>;
        using listerens_map  = std::map<std::string, vserv::listener_sptr>;
        using listener_wptr  = std::weak_ptr<vserv::listener>;


        const std::string subsys_name( "listerens" );

    }

    struct listerens::impl {

        application     *app_;
        string_vector    endpoints_;
        listerens_map    listeners_;

//        logger          &log_;

        impl( application *app )
            :app_(app)
//            ,log_(app_->subsystem<subsys::log>( ).get_logger( ))
        { }

        void reg_creator( const std::string &name,
                          application::service_getter_type func )
        {
            app_->register_service_creator( name, func );
        }

        void unreg_creator( const std::string &name )
        {
            app_->unregister_service_creator( name );
        }

///////////// signals
        void cb_on_start( listener_wptr inst )
        {
            auto lck = inst.lock( );
            if( !lck ) {
                return;
            }
            std::cout << "[listeren] start: " << lck->name( )
                      << std::endl;
        }

        void cb_on_stop( listener_wptr inst )
        {
            auto lck = inst.lock( );
            if( !lck ) {
                return;
            }
            std::cout << "[listeren] stop: " << lck->name( )
                      << std::endl;
        }

        void cb_on_accept_failed( const bsys::error_code &err,
                                  listener_wptr inst )
        {
            auto lck = inst.lock( );
            if( !lck ) {
                return;
            }
            if( err ) {
                std::cout << "[listener] accept failed: " << err.message( )
                          << std::endl;
                /// recall accept? or stop?
                return;
            }
        }

        void cb_on_new_connection( const vcomm::connection_iface *cl,
                                   listener_wptr inst )
        {
            auto lck = inst.lock( );
            if( !lck ) {
                return;
            }
            std::cout << "[listener] new connection: " << cl->name( )
                      << std::endl;
        }

        void on_stop_connection( const vcomm::connection_iface *cl,
                                 listener_wptr inst )
        {
            auto lck = inst.lock( );
            if( !lck ) {
                return;
            }
            std::cout << "[listener] stop connection: " << cl->name( )
                      << std::endl;
        }

        void connect_signals( vserv::listener_sptr &l )
        {
            namespace ph = std::placeholders;

            l->on_new_connection_connect(
                std::bind( &impl::cb_on_new_connection, this,
                           ph::_1, l->weak_from_this( ) ) );

            l->on_stop_connection_connect(
                std::bind( &impl::on_stop_connection, this,
                           ph::_1, l->weak_from_this( ) ) );

            l->on_accept_failed_connect(
                std::bind( &impl::cb_on_accept_failed, this,
                           ph::_1, l->weak_from_this( ) ) );

            l->on_start_connect(
                std::bind( &impl::cb_on_start, this,
                           l->weak_from_this( ) ) );

            l->on_stop_connect(
                std::bind( &impl::cb_on_stop, this,
                           l->weak_from_this( ) ) );

        }

        void start_all( )
        {
            using namespace vserv::listeners;
            static const char *true_false[2] = { "off", " on" };

            for( auto &ep: endpoints_ ) {
                auto inf = utilities::get_endpoint_info( ep );
                if( inf ) {
                    vserv::listener_sptr next;
                    if( inf.is_local( ) ) {

                        next = local::create( *app_->get_application( ),
                                              inf.addpess );
                        std::cout << "Start local ep: " << inf.addpess
                                  << " (" << inf << ") "
                                  << std::endl;
                    } else {

                        next = tcp::create( *app_->get_application( ),
                                             inf.addpess, inf.service );
                        std::cout << "Start tcp ep: "
                                  << inf.addpess  << " " << inf.service
                                  << " ssl: "
                                  << true_false[inf.is_ssl( ) ? 1 : 0]
                                  << " (" << inf << ") "
                                  << std::endl;
                    }
                    if( next ) {
                        connect_signals( next );
                        next->start( );
                        listeners_[ep] = next;
                    }
                }
            }
        }

    };


    listerens::listerens( application *app )
        :impl_(new impl(app))
    { }

    listerens::~listerens( )
    {
        delete impl_;
    }

    /// static
    vtrc::shared_ptr<listerens> listerens::create( application *app,
                                           const std::vector<std::string> &def )
    {
        vtrc::shared_ptr<listerens> new_inst(new listerens(app));
        new_inst->impl_->endpoints_.assign( def.begin( ), def.end( ) );

        return new_inst;
    }

    const std::string &listerens::name( ) const
    {
        return subsys_name;
    }

    void listerens::init( )
    {

    }

    void listerens::start( )
    {
        impl_->start_all( );
//        impl_->LOGINF << "Started.";
    }

    void listerens::stop( )
    {
//        impl_->LOGINF << "Stopped.";
    }


}}}

    
