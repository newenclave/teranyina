
#ifndef TA_SUBSYS_listerens_H
#define TA_SUBSYS_listerens_H

#include "subsys-iface.h"

#include "vtrc-common/vtrc-signal-declaration.h"

#include <vector>

namespace vtrc {

    namespace common {
        struct connection_iface;
    }

    namespace server {
        class listener;
    }
}

namespace ta { namespace agent {

    class application;

namespace subsys {

    class listeners: public subsystem_iface {

        struct  impl;
        impl   *impl_;

        using connection_iface = vtrc::common::connection_iface;
        using vlistener        = vtrc::server::listener;

        VTRC_DECLARE_SIGNAL( on_new_connection,
                             void ( const connection_iface &, vlistener & ) );

        VTRC_DECLARE_SIGNAL( on_stop_connection,
                             void ( const connection_iface & ) );

    protected:

        listeners( application *app );

    public:

        static subsystem_iface::uuid_type get_uuid( );

        ~listeners( );

        typedef std::shared_ptr<listeners> shared_type;
        static shared_type create( application *app,
                                   const std::vector<std::string> &def );

        const std::string &name( ) const;

        void init( )  ;
        void start( ) ;
        void stop( )  ;
    };

}}}

#endif

    
