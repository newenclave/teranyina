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

#define LOG(lev) log_(lev) << "[listerens] "
#define LOGINF   LOG(logger::level::info)
#define LOGDBG   LOG(logger::level::debug)
#define LOGERR   LOG(logger::level::error)
#define LOGWRN   LOG(logger::level::warning)

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
        agent::logger   &log_;

        string_vector    endpoints_;
        listerens_map    listeners_;

        impl( application *app )
            :app_(app)
            ,log_(app_->get_logger( ))
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
            LOGINF << "start point: " << lck->name( )
                   ;
        }

        void cb_on_stop( listener_wptr inst )
        {
            auto lck = inst.lock( );
            if( !lck ) {
                return;
            }
            LOGINF << "stop point: " << lck->name( )
                   ;
        }

        void cb_on_accept_failed( const bsys::error_code &err,
                                  listener_wptr inst )
        {
            auto lck = inst.lock( );
            if( !lck ) {
                return;
            }
            if( err ) {
                LOGERR << "accept failed: " << err.message( )
                       ;
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
            LOGINF << "new connection: " << cl->name( )
                   ;
        }

        void cb_on_stop_connection( const vcomm::connection_iface *cl,
                                    listener_wptr inst )
        {
            auto lck = inst.lock( );
            if( !lck ) {
                return;
            }
            LOGINF << "stop connection: " << cl->name( )
                   ;
        }

        void connect_signals( vserv::listener_sptr &l )
        {
            namespace ph = std::placeholders;

#define LIST_CONNECT_IMPL(name) l->name##_connect(              \
                        std::bind( &impl::cb_##name, this,      \
                                   l->weak_from_this( ) ) )

#define LIST_CONNECT_IMPL1(name) l->name##_connect(             \
                        std::bind( &impl::cb_##name, this,      \
                            ph::_1, l->weak_from_this( ) ) )

            LIST_CONNECT_IMPL1( on_new_connection );
            LIST_CONNECT_IMPL1( on_stop_connection );
            LIST_CONNECT_IMPL1( on_accept_failed );

            LIST_CONNECT_IMPL( on_start );
            LIST_CONNECT_IMPL( on_stop );

//            l->on_new_connection_connect(
//                std::bind( &impl::cb_on_new_connection, this,
//                           ph::_1, l->weak_from_this( ) ) );

//            l->on_stop_connection_connect(
//                std::bind( &impl::on_stop_connection, this,
//                           ph::_1, l->weak_from_this( ) ) );

//            l->on_accept_failed_connect(
//                std::bind( &impl::cb_on_accept_failed, this,
//                           ph::_1, l->weak_from_this( ) ) );

//            l->on_start_connect(
//                std::bind( &impl::cb_on_start, this,
//                           l->weak_from_this( ) ) );

//            l->on_stop_connect(
//                std::bind( &impl::cb_on_stop, this,
//                           l->weak_from_this( ) ) );

#undef LIST_CONNECT_IMPL1
#undef LIST_CONNECT_IMPL

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
                        LOGINF << "Start local ep: " << inf.addpess
                               << " (" << inf << ") "
                               ;
                    } else {

                        next = tcp::create( *app_->get_application( ),
                                             inf.addpess, inf.service );
                        LOGINF << "Start tcp ep: "
                                  << inf.addpess  << " " << inf.service
                                  << " ssl: "
                                  << true_false[inf.is_ssl( ) ? 1 : 0]
                                  << " (" << inf << ") "
                                  ;
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
        impl_->LOGINF << "Init";
    }

    void listerens::start( )
    {
        impl_->start_all( );
        impl_->LOGINF << "Started";
    }

    void listerens::stop( )
    {
        impl_->LOGINF << "Stopped";
    }


}}}

    
