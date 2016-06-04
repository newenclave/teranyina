
#ifndef TA_SUBSYS_lua_H
#define TA_SUBSYS_lua_H

#include "subsys-iface.h"

namespace ta { namespace agent {

    class application;

namespace subsys {

    class lua: public subsystem_iface {

        struct  impl;
        impl   *impl_;

    protected:

        lua( application *app , const std::string &conf );

    public:

        ~lua( );

        typedef std::shared_ptr<lua> shared_type;
        static shared_type create( application *app, const std::string &conf );

        const std::string &name( ) const NOEXCEPT override;

        void init( )  override;
        void start( ) override;
        void stop( )  override;
    };

}}}

#endif

    
