
#ifndef TA_SUBSYS_listerens_H
#define TA_SUBSYS_listerens_H

#include "subsys-iface.h"

#include "vtrc-common/vtrc-signal-declaration.h"

#include <vector>

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

        typedef std::shared_ptr<listerens> shared_type;
        static shared_type create( application *app,
                                   const std::vector<std::string> &def );

        const std::string &name( )  const;

        void init( )  ;
        void start( ) ;
        void stop( )  ;
    };

}}}

#endif

    
