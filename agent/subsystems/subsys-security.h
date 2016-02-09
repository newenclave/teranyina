
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

        ~security( );

        static vtrc::shared_ptr<security> create( application *app );

        const std::string &name( )  const noexcept override;

        void init( )  override;
        void start( ) override;
        void stop( )  override;
    };

}}}

#endif

    