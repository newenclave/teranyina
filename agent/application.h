#ifndef TA_APPLICATION_CPP
#define TA_APPLICATION_CPP

#include <memory>
#include <assert.h>

#include "vtrc-server/vtrc-application.h"
#include "vtrc-common/vtrc-signal-declaration.h"
#include "vtrc-common/vtrc-rtti-wrapper.h"
#include "vtrc-common/vtrc-connection-iface.h"
#include "vtrc-common/vtrc-pool-pair.h"

#include "common/utils.h"

#include "vtrc-function.h"

#include "subsystems/subsys-iface.h"
#include "logger.h"

#define TA_RTTI_DISABLE

#ifdef TA_RTTI_DISABLE
#   define TA_TYPE_ID( T ) ta::utilities::type_uid<T>::uid( )
#else
#   define TA_TYPE_ID( T ) typeid( T )
#endif

namespace ta { namespace agent {

    class application {

        struct  impl;
        vtrc::common::pool_pair  pools_;
        impl                    *impl_;

        using ta_app                = ta::agent::application;
        using connection_iface_wptr = vtrc::common::connection_iface_wptr;

    public:

        application( const application & )         = delete;
        application(       application && )        = delete;
        application & operator = ( application & ) = delete;

        application( );
        ~application( );

        class service_wrapper_impl: public vtrc::common::rpc_service_wrapper {

            application *app_;
            vtrc::common::connection_iface_wptr client_;

            typedef vtrc::common::rpc_service_wrapper super_type;

        public:

            typedef super_type::service_type service_type;
            typedef super_type::service_ptr  service_ptr;
            typedef super_type::service_sptr service_sptr;

            typedef super_type::method_type  method_type;

            vtrc::common::connection_iface_wptr &client( )
            {
                return client_;
            }

            const vtrc::common::connection_iface_wptr &client( ) const
            {
                return client_;
            }

            service_wrapper_impl( application *app,
                                  vtrc::common::connection_iface_wptr c,
                                  service_sptr serv );

            ~service_wrapper_impl( );

        protected:

            const method_type *get_method ( const std::string &name ) const;
            application       *get_application( )       NOEXCEPT;
            const application *get_application( ) const NOEXCEPT;
        };

        typedef vtrc::common::rpc_service_wrapper     parent_service_type;
        typedef std::shared_ptr<parent_service_type>  parent_service_sptr;

        typedef service_wrapper_impl              service_wrapper;
        typedef std::shared_ptr<service_wrapper>  service_wrapper_sptr;

        ///
        /// func( app, connection )
        ///
        typedef std::function <
            service_wrapper_sptr (ta_app *, connection_iface_wptr)
        > service_getter_type;

        service_wrapper_sptr wrap_service ( connection_iface_wptr c,
                                    service_wrapper_impl::service_sptr serv );
    public: // services

        void register_service_factory( const std::string &name,
                                       service_getter_type func );

        void unregister_service_factory( const std::string &name );

        void quit( );

    public: // subsystems

        template <typename T>
        void add_subsystem( )
        {
            subsystem_sptr subsys ( T::create( this ) );
            add_subsys( TA_TYPE_ID(T), subsys );
        }

        template <typename T, typename ... Args >
        void add_subsystem( const Args & ... pars )
        {
            subsystem_sptr subsys ( T::create( this,
                                    std::forward<decltype(pars)>(pars) ... ) );
            add_subsys( TA_TYPE_ID(T), subsys );
        }

        template <typename T>
        T &subsystem( )
        {
            subsystem_iface *subsys = get_subsys( TA_TYPE_ID(T) );
            if( nullptr == subsys ) {
                throw std::runtime_error( "Invalid subsystem" );
            }
            return *(poly_downcast<T *>( subsys ));
        }

        template <typename T>
        const T &subsystem( ) const
        {
            const subsystem_iface *subsys = get_subsys( TA_TYPE_ID(T) );
            if( nullptr == subsys ) {
                throw std::runtime_error( "Invalid subsystem" );
            }
            return *(poly_downcast<const T *>( subsys ));
        }

        template <typename T>
        T *subsystem_safe( ) NOEXCEPT
        {
            subsystem_iface *subsys = get_subsys( TA_TYPE_ID(T) );
            return poly_downcast<const T *>( subsys );
        }

        template <typename T>
        const T *subsystem_safe( ) const NOEXCEPT
        {
            const subsystem_iface *subsys = get_subsys( TA_TYPE_ID(T) );
            return poly_downcast<const T *>( subsys );
        }

        bool is_ctrl_connection( vtrc::common::connection_iface *c );

        vtrc::server::application       *get_application( )       NOEXCEPT;
        const vtrc::server::application *get_application( ) const NOEXCEPT;

        boost::asio::io_service &get_io_service( )  NOEXCEPT;
        boost::asio::io_service &get_rpc_service( ) NOEXCEPT;

        agent::logger       &get_logger( )       NOEXCEPT;
        const agent::logger &get_logger( ) const NOEXCEPT;

        static const std::string &thread_ptrfix( );

        void start_all( );
        void stop_all( );

        void run( int argc, const char *argv[ ] );

    private:

        template <class Tgt, class Src>
        static Tgt poly_downcast ( Src * x )
        {
//#ifndef TA_RTTI_DISABLE
            assert( dynamic_cast<Tgt>(x) == x );
//#endif
            return static_cast<Tgt>(x);
        }

#ifdef TA_RTTI_DISABLE

        void add_subsys( std::uintptr_t info, subsystem_sptr inst );
        /* === nothrow === */
        /*
         * return nullptr if not found
        */
        subsystem_iface       *get_subsys( std::uintptr_t info )      NOEXCEPT;
        const subsystem_iface *get_subsys( std::uintptr_t info) const NOEXCEPT;

        /* =============== */
#else

        void add_subsys( const std::type_info &info, subsystem_sptr inst );
        /* === nothrow === */
        /*
         * return nullptr if not found
        */
        subsystem_iface *
        get_subsys( const std::type_info &info ) NOEXCEPT;

        const subsystem_iface *
        get_subsys( const std::type_info &info) const NOEXCEPT;

        /* =============== */
#endif

    };
}}

#endif // APPLICATION_CPP

