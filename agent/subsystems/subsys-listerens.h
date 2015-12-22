
#ifndef TA_SUBSYS_listerens_H
#define TA_SUBSYS_listerens_H

#include "subsys-iface.h"

#include "vtrc-common/vtrc-signal-declaration.h"

namespace ta { namespace agent {

    class application;

namespace subsys {

    class listerens: public subsystem_iface {

        struct  impl;
        impl   *impl_;

    protected:

        listerens( application *app );

    public:

        ~listerens( );

        static vtrc::shared_ptr<listerens> create( application *app );

        const std::string &name( )  const;

        void init( )  ;
        void start( ) ;
        void stop( )  ;
    };

}}}

#endif

    
