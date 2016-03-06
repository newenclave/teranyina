
#ifndef TA_SUBSYS_fs_H
#define TA_SUBSYS_fs_H

#include "subsys-iface.h"
#include "common/noexcept.hpp"

namespace ta { namespace agent {

    class application;

namespace subsys {

    class fs: public subsystem_iface {

        struct  impl;
        impl   *impl_;

    protected:

        fs( application *app );

    public:

        ~fs( );

        typedef std::shared_ptr<fs> shared_type;
        static shared_type create( application *app );

        static subsystem_iface::uuid_type get_uuid( );
        const std::string &name( ) const NOEXCEPT override;

        void init( )  override;
        void start( ) override;
        void stop( )  override;
    };

}}}

#endif

    
