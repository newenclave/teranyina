
#ifndef TA_SUBSYS_multicast_H
#define TA_SUBSYS_multicast_H

#include "subsys-iface.h"
#include "vtrc-common/vtrc-signal-declaration.h"

#include <vector>
#include <cstdint>

namespace ta { namespace proto { namespace multicast {
    class ping;
    class pong;
}}}

namespace ta { namespace agent {

    class application;

namespace subsys {

    class multicast: public subsystem_iface {

        struct         impl;
        friend struct  impl;
        impl          *impl_;

        /// [in] ip, [in] port, [in] ping_message, [in, out] pong_message
        VTRC_DECLARE_SIGNAL( on_new_request,
                             void( const std::string &, std::uint16_t,
                                   const ta::proto::multicast::ping &,
                                   ta::proto::multicast::pong & ) );

    protected:

        multicast( application *app );

    public:

        static subsystem_iface::uuid_type get_uuid( );

        ~multicast( );

        typedef std::shared_ptr<multicast> shared_type;
        static shared_type create( application *app,
                                   const std::vector<std::string> &def );

        const std::string &name( )  const;

        void init( )  ;
        void start( ) ;
        void stop( )  ;
    };

}}}

#endif

    
