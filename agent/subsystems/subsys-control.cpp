#include <memory>

#include "subsys-control.h"
#include "../application.h"

#include "protocol/ctrl.pb.h"

#include "vtrc-common/vtrc-closure-holder.h"
#include "vtrc-common/vtrc-delayed-call.h"


#define LOG(lev) log_(lev) << "[ control] "
#define LOGINF   LOG(level::info)
#define LOGDBG   LOG(level::debug)
#define LOGERR   LOG(level::error)
#define LOGWRN   LOG(level::warning)

namespace ta { namespace agent { namespace subsys {

    namespace {

        const std::string subsys_name( "control" );

        using level = agent::logger::level;
        using closure_holder = vtrc::common::closure_holder;

        class ctrl_impl: public ta::proto::ctrl {

            ta::agent::application *app_;

        public:

            ctrl_impl(ta::agent::application *a)
                :app_(a)
            { }

            void shutdown(::google::protobuf::RpcController* /*controller*/,
                     const ::ta::proto::empty*               /*request*/,
                     ::ta::proto::empty*                     /*response*/,
                     ::google::protobuf::Closure* done) override
            {
                closure_holder _(done);
                app_->get_logger( )( level::info ) << "Shutting down agent.";
                app_->quit( );
            }
            static std::string name( )
            {
                return ta::proto::ctrl::descriptor( )->full_name( );
            }
        };

        application::service_wrapper_sptr create_service(
                                      ta::agent::application *app,
                                      vtrc::common::connection_iface_wptr cl )
        {

            app->get_logger( )( level::debug ) << "Create service";
            auto inst = std::make_shared<ctrl_impl>( app );
            return app->wrap_service( cl, inst );

            //return application::service_wrapper_sptr( );
        }
    }

    struct control::impl {

        application     *app_;
        agent::logger   &log_;
        vtrc::common::delayed_call dc_;

        impl( application *app )
            :app_(app)
            ,log_(app_->get_logger( ))
            ,dc_(app_->get_io_service( ))
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


    control::control( application *app )
        :impl_(new impl(app))
    { }

    control::~control( )
    {
        delete impl_;
    }

    /// static
    control::shared_type control::create( application *app )
    {
        shared_type new_inst(new control(app));
        return new_inst;
    }

    const std::string &control::name( ) const noexcept
    {
        return subsys_name;
    }

    void control::init( )
    {
        impl_->reg_creator( ctrl_impl::name( ), create_service );
    }

    void control::start( )
    {
        impl_->LOGINF << "Started.";
//        impl_->dc_.call_from_now( [this](...) { impl_->app_->quit( ); },
//            vtrc::common::timer::monotonic_traits::milliseconds(5000) );
    }

    void control::stop( )
    {
        impl_->LOGINF << "Stopped.";
    }


}}}

    
