
#ifndef TA_SUBSYS_security_H
#define TA_SUBSYS_security_H

#include "subsys-iface.h"

namespace ta { namespace agent {

    class application;

namespace subsys {

    class security: public subsystem_iface {

        struct  impl;
        impl   *impl_;

    protected:

        security( application *app );

    public:

        static subsystem_iface::uuid_type get_uuid( );

        ~security( );

        typedef std::shared_ptr<security> shared_type;
        static shared_type create( application *app );

        const std::string &name( )  const NOEXCEPT override;

        void init( )  override;
        void start( ) override;
        void stop( )  override;
    };

}}}

#endif

    
