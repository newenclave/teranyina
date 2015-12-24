
#ifndef TA_SUBSYS_multicast_H
#define TA_SUBSYS_multicast_H

#include "subsys-iface.h"

#include <vector>

namespace ta { namespace agent {

    class application;

namespace subsys {

    class multicast: public subsystem_iface {

        struct  impl;
        impl   *impl_;

    protected:

        multicast( application *app );

    public:

        ~multicast( );

        static vtrc::shared_ptr<multicast> create( application *app,
                                        const std::vector<std::string> &def );

        const std::string &name( )  const;

        void init( )  ;
        void start( ) ;
        void stop( )  ;
    };

}}}

#endif

    
