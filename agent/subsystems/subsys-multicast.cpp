
#include "subsys-multicast.h"
#include "../application.h"
//#include "subsys-log.h"

//#include "vtrc-memory.h"

//#define LOG(lev) log_(lev) << "[multicast] "
//#define LOGINF   LOG(logger::info)
//#define LOGDBG   LOG(logger::debug)
//#define LOGERR   LOG(logger::error)
//#define LOGWRN   LOG(logger::warning)

namespace ta { namespace agent { namespace subsys {

    namespace {
        const std::string subsys_name( "multicast" );

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
//        impl_->LOGINF << "Started.";
    }

    void multicast::stop( )
    {
//        impl_->LOGINF << "Stopped.";
    }


}}}

    