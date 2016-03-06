
#ifndef TA_SUBSYS_control_H
#define TA_SUBSYS_control_H

#include "subsys-iface.h"

namespace ta { namespace agent {

    class application;

namespace subsys {

    class control: public subsystem_iface {

        struct  impl;
        impl   *impl_;

    protected:

        control( application *app );

    public:

        static subsystem_iface::uuid_type get_uuid( );

        ~control( );

        typedef std::shared_ptr<control> shared_type;
        static shared_type create( application *app );

        const std::string &name( )  const NOEXCEPT override;

        std::string service_name( ) const NOEXCEPT;

        void init( )  override;
        void start( ) override;
        void stop( )  override;
    };

}}}

#endif

    
