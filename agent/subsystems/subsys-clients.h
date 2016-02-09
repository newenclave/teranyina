
#ifndef TA_SUBSYS_clients_H
#define TA_SUBSYS_clients_H

#include "subsys-iface.h"

namespace vtrc { namespace client {
    class vtrc_client;
}}

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

        static std::shared_ptr<clients> create( application *app );

        void add_client( const std::string &path );

    public:

        const std::string &name( )  const;

        void init( )  ;
        void start( ) ;
        void stop( )  ;
    };

}}}

#endif

    
