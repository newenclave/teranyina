#include <iostream>
#include <list>
#include <memory>

#include "subsys-listeners.h"
#include "../application.h"

#include "vtrc-server/vtrc-listener.h"
#include "vtrc-server/vtrc-listener-tcp.h"
#include "vtrc-server/vtrc-listener-ssl.h"
#include "vtrc-server/vtrc-listener-local.h"

#include "vtrc-common/vtrc-mutex-typedefs.h"

#include "common/utils.h"

#include "boost/system/error_code.hpp"

#ifndef _WIN32
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#define LOG(lev) log_(lev, "listener")
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

    struct listeners::impl {

        application     *app_;
        agent::logger   &log_;
        listeners       *parent_;

        string_vector       endpoints_;
        listerens_map       listeners_;
        vtrc::shared_mutex  listeners_lock_;

        impl( application *app )
            :app_(app)
            ,log_(app_->get_logger( ))
        { }

        void reg_creator( const std::string &name,
                          application::service_getter_type func )
        {
            app_->register_service_factory( name, func );
        }

        void unreg_creator( const std::string &name )
        {
            app_->unregister_service_factory( name );
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

        void cb_on_new_connection( vcomm::connection_iface *cl,
                                   listener_wptr inst )
        {
            auto lck = inst.lock( );
            if( !lck ) {
                return;
            }

            parent_->on_new_connection_( *cl, *lck );
            LOGINF << "new  connection: " << cl->name( )
                   ;
        }

        void cb_on_stop_connection( vcomm::connection_iface *cl,
                                    listener_wptr inst )
        {
            auto lck = inst.lock( );
            if( !lck ) {
                return;
            }
            parent_->on_stop_connection_( *cl );
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

#undef LIST_CONNECT_IMPL1
#undef LIST_CONNECT_IMPL

        }

        void create_from_endpoint_info( const utilities::endpoint_info &inf,
                                        const std::string &ep )
        {
            using namespace vserv::listeners;
            static const char *true_false[2] = { "off", " on" };

            {
                vtrc::shared_lock _(listeners_lock_);
                if( listeners_.find( ep ) != listeners_.end( ) ) {
                    LOGWRN << "Listener '" << ep << "' already exists. "
                           << "Ignoring...";
                    return;
                }
            }

            if( inf ) {
                vserv::listener_sptr next;
                if( inf.is_local( ) ) {
#ifndef _WIN32
                    struct stat st = {0};
                    int r = ::stat( inf.addpess.c_str( ), &st );
                    if( -1 != r ) {
                        if( S_ISSOCK( st.st_mode ) ) {
                            LOGINF << "Socket found. Unlinking...";
                            ::unlink( inf.addpess.c_str( ) );
                        } else {
                            LOGERR << "File " << inf.addpess << " found. "
                                   << "But it is not a socket. Ignoring.";
                            return;
                        }
                    } else {
                        std::error_code ec( errno, std::system_category( ));
                        LOGDBG << "::stat( ) for '"
                               << inf.addpess << "' failed. "
                               << ec.message( );
                    }
#endif
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

                if( inf.is_dummy( ) ) {
                    namespace lld = vcomm::lowlevel::dummy;
                    next->assign_lowlevel_protocol_factory( &lld::create );
                }

                if( next ) {
                    connect_signals( next );
                    next->start( );
                    vtrc::unique_shared_lock _(listeners_lock_);
                    listeners_[ep] = next;
                }
            }
        }

        void start_all( )
        {
            for( auto &ep: endpoints_ ) {
                auto inf = utilities::get_endpoint_info( ep );
                create_from_endpoint_info( inf, ep );
            }
        }

        void del_listener( const std::string &name )
        {
            vtrc::shared_lock _(listeners_lock_);
            auto l = listeners_.find( name );
            if( l != listeners_.end( ) ) {
                l->second->stop( );
                listeners_.erase( l );
            }
        }

    };

    listeners::listeners( application *app )
        :impl_(new impl(app))
    {
        impl_->parent_ = this;
    }

    listeners::~listeners( )
    {
        delete impl_;
    }

    /// static
    listeners::shared_type listeners::create( application *app,
                                      const std::vector<std::string> &def )
    {
        vtrc::shared_ptr<listeners> new_inst(new listeners(app));
        new_inst->impl_->endpoints_.assign( def.begin( ), def.end( ) );

        return new_inst;
    }

    void listeners::add_listener( const std::string &name,
                                  bool ssl, bool dummy )
    {
        utilities::endpoint_info epi = utilities::get_endpoint_info( name );
        if( dummy ) {
            epi.flags |= utilities::endpoint_info::FLAG_DUMMY;
        }
        impl_->create_from_endpoint_info( epi, name );
    }

    void listeners::del_listener( const std::string &name )
    {
        impl_->del_listener( name );
    }

    const std::string &listeners::name( ) const
    {
        return subsys_name;
    }

    void listeners::init( )
    {
        impl_->LOGINF << "Init";
    }

    void listeners::start( )
    {
        impl_->start_all( );
        impl_->LOGINF << "Started";
    }

    void listeners::stop( )
    {
        impl_->LOGINF << "Stopped";
    }

}}}

    
