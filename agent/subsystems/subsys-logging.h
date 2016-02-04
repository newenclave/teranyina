
#ifndef TA_SUBSYS_logging_H
#define TA_SUBSYS_logging_H

#include <vector>

#include "subsys-iface.h"

namespace ta { namespace agent {

    class application;

namespace subsys {

    class logging: public subsystem_iface {

        struct  impl;
        impl   *impl_;

    protected:

        logging( application *app );

    public:

        ~logging( );

        static vtrc::shared_ptr<logging> create( application *app,
                                         const std::vector<std::string> &def );

        void add_logger_output( const std::string &params );
        void del_logger_output( const std::string &name );

    public:

        const std::string &name( )  const;
        void init( )  ;
        void start( ) ;
        void stop( )  ;
    };

}}}

#endif

    
