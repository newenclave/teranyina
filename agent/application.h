#ifndef TA_APPLICATION_CPP
#define TA_APPLICATION_CPP

#include <memory>
#include <assert.h>

#include "vtrc-server/vtrc-application.h"
#include "vtrc-common/vtrc-signal-declaration.h"
#include "vtrc-common/vtrc-rtti-wrapper.h"

#include "subsystems/subsys-iface.h"
#include "logger.h"

namespace ta { namespace agent {

    class application: public vtrc::server::application {
        struct  impl;
        impl   *impl_;
    public:
        application( application & ) = delete;
        application & operator = ( application & ) = delete;

        application( );
        ~application( );
    private:

        template <class Tgt, class Src>
        static Tgt poly_downcast ( Src * x )
        {
            assert( dynamic_cast<Tgt>(x) == x );
            return static_cast<Tgt>(x);
        }

    public: // subsystems

        template <typename T>
        void add_subsystem( )
        {
            subsystem_sptr subsys ( T::create( this ) );
            add_subsys( typeid( T ), subsys );
        }

        template <typename T, typename ... Args >
        void add_subsystem( const Args & ... pars )
        {
            subsystem_sptr subsys ( T::create( this, pars ... ) );
            add_subsys( typeid( T ), subsys );
        }

        template <typename T>
        T &subsystem( )
        {
            subsystem_iface *subsys = get_subsys( typeid(T) );
            if( nullptr == subsys ) {
                throw std::runtime_error( "Invalid subsystem" );
            }
            return *(poly_downcast<T *>( subsys ));
        }

        template <typename T>
        const T &subsystem( ) const
        {
            const subsystem_iface *subsys = get_subsys( typeid(T) );
            if( nullptr == subsys ) {
                throw std::runtime_error( "Invalid subsystem" );
            }
            return *(poly_downcast<const T *>( subsys ));
        }

        template <typename T>
        T *subsystem_safe( ) noexcept
        {
            subsystem_iface *subsys = get_subsys( typeid(T) );
            return poly_downcast<const T *>( subsys );
        }

        template <typename T>
        const T *subsystem_safe( ) const noexcept
        {
            const subsystem_iface *subsys = get_subsys( typeid(T) );
            return poly_downcast<const T *>( subsys );
        }

        void start_all( );
        void stop_all( );

        void run( int argc, const char *argv[ ] );

    private:

        void add_subsys( const std::type_info &info, subsystem_sptr inst );

        /* === nothrow === */
        /*
         * returns nullptr if not found
        */

        subsystem_iface *
        get_subsys( const std::type_info &info ) noexcept;

        const subsystem_iface *
        get_subsys( const std::type_info &info) const noexcept;

        /* =============== */
    };
}}

#endif // APPLICATION_CPP

