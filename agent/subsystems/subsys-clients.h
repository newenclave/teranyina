
#ifndef TA_SUBSYS_clients_H
#define TA_SUBSYS_clients_H

#include "subsys-iface.h"

namespace ta { namespace agent {

    class application;

namespace subsys {

    class clients: public subsystem_iface {

        struct  impl;
        impl   *impl_;

    protected:

        clients( application *app );

    public:

        ~clients( );

        static vtrc::shared_ptr<clients> create( application *app );

        const std::string &name( )  const;

        void init( )  ;
        void start( ) ;
        void stop( )  ;
    };

}}}

#endif

    
