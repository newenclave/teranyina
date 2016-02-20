
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

        ~control( );

        typedef std::shared_ptr<control> shared_type;
        static shared_type create( application *app );

        const std::string &name( )  const noexcept override;

        std::string service_name( ) const noexcept;

        void init( )  override;
        void start( ) override;
        void stop( )  override;
    };

}}}

#endif

    
