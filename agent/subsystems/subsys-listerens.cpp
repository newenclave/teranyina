#include <list>

#include "subsys-listerens.h"
#include "../application.h"

#include "vtrc-server/vtrc-listener.h"
#include "vtrc-server/vtrc-listener-tcp.h"
#include "vtrc-server/vtrc-listener-ssl.h"
#include "vtrc-server/vtrc-listener-local.h"

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

        using listerens_list = std::list<vserv::listener_sptr>;

        const std::string subsys_name( "listerens" );

        application::service_wrapper_sptr create_service(
                                      ta::agent::application * /*app*/,
                                      vcomm::connection_iface_wptr cl )
        {
            ///auto inst(vtrc::make_shared<impl_type_here>( app, cl ));
            ///return app->wrap_service( cl, inst );

            return application::service_wrapper_sptr( );
        }
    }

    struct listerens::impl {

        application     *app_;
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

//        impl_->LOGINF << "Started.";
    }

    void listerens::stop( )
    {
//        impl_->LOGINF << "Stopped.";
    }


}}}

    
