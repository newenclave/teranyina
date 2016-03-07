#!/usr/bin/env python
# -*- coding: utf-8 -*-

from sys import argv
import os

def header_file( ):
    """
#ifndef TA_SUBSYS_%ss-name%_H
#define TA_SUBSYS_%ss-name%_H

#include "subsys-iface.h"

namespace ta { namespace agent {

    class application;

namespace subsys {

    class %ss-name%: public subsystem_iface {

        struct  impl;
        impl   *impl_;

    protected:

        %ss-name%( application *app );

    public:

        ~%ss-name%( );

        typedef std::shared_ptr<%ss-name%> shared_type;
        static shared_type create( application *app );

        const std::string &name( ) const NOEXCEPT override;

        void init( )  override;
        void start( ) override;
        void stop( )  override;
    };

}}}

#endif

    """
    return header_file.__doc__

def source_file( ):
    """
#include "subsys-%ss-name%.h"
#include "../application.h"

#define LOG(lev) log_(lev) << "[%ss-name%] "
#define LOGINF   LOG(level::info)
#define LOGDBG   LOG(level::debug)
#define LOGERR   LOG(level::error)
#define LOGWRN   LOG(level::warning)

namespace ta { namespace agent { namespace subsys {

    namespace {

        const std::string subsys_name( "%ss-name%" );

        using level = agent::logger::level;

        application::service_wrapper_sptr create_service(
                                      ta::agent::application * /*app*/,
                                      vtrc::common::connection_iface_wptr cl )
        {
            ///auto inst = std::make_shared<impl_type_here>( app, cl );
            ///return app->wrap_service( cl, inst );

            return application::service_wrapper_sptr( );
        }
    }

    struct %ss-name%::impl {

        application     *app_;
        agent::logger   &log_;

        impl( application *app )
            :app_(app)
            ,log_(app_->get_logger( ))
        { }

        void reg_creator( const std::string &name,
                          application::service_getter_type func )
        {
            app_->register_service_factory( name, func );
        }

        void unreg_creator( const std::string &name )
        {
            app_->unregister_service_factory( name );
        }
    };

    %ss-name%::%ss-name%( application *app )
        :impl_(new impl(app))
    { }

    %ss-name%::~%ss-name%( )
    {
        delete impl_;
    }

    /// static
    %ss-name%::shared_type %ss-name%::create( application *app )
    {
        shared_type new_inst(new %ss-name%(app));
        return new_inst;
    }

    const std::string &%ss-name%::name( ) const noexcept
    {
        return subsys_name;
    }

    void %ss-name%::init( )
    {

    }

    void %ss-name%::start( )
    {
        impl_->LOGINF << "Started";
    }

    void %ss-name%::stop( )
    {
        impl_->LOGINF << "Stopped";
    }

}}}

    """
    return source_file.__doc__

def usage(  ):
    """
    usage: addsubsys.py <subsystem-name>
    """
    print( usage.__doc__ )

def fix_iface_inc( ss_name ):
    src_path = os.path.join( 'subsys.inc' )
    s = open( src_path, 'r' );
    content = s.readlines(  )
    s.close()
    content.append( '#include "subsys-'  + ss_name + '.h"\n')
    s = open( src_path, 'w' );
    s.writelines( content )

if __name__ == '__main__':
    if len( argv ) < 2:
        usage( )
        exit( 1 )

    ss_file = argv[1]
    ss_name = ss_file # ss_file.replace( '-', '_' )

    src_name = 'subsys-' + ss_file + '.cpp';
    hdr_name = 'subsys-' + ss_file + '.h';

    if os.path.exists( src_name ) or os.path.exists( hdr_name ):
        print ( "File already exists" )
        exit(1)

    src_content = source_file(  ).replace( '%ss-name%', ss_name )
    hdr_content = header_file(  ).replace( '%ss-name%', ss_name )

    s = open( src_name, 'w' );
    s.write( src_content )

    h = open( hdr_name, 'w' );
    h.write( hdr_content )

    fix_iface_inc( ss_name )
