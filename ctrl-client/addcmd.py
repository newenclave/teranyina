#!/usr/bin/env python
# -*- coding: utf-8 -*-

from sys import argv
import os

def source_file():
    """
#include "cmd-iface.h"
//#include "boost/program_options.hpp"

namespace ta { namespace cc { namespace cmd {

    namespace {

        namespace po = boost::program_options;
        namespace ifaces = ta::client::interfaces;
        typedef client::core core;

        const char *cmd_name = "%test-name%";

        struct impl: public cmd_iface {

            const char *name( ) const
            {
                return cmd_name;
            }

            void exec( po::variables_map &vm, core &client )
            {

            }

            void add_options( po::options_description &desc )
            {
                /// reserved as common
                /// "help,h"
                /// "command,c"
                /// "server,s"
                /// "io-pool-size,i"
                /// "rpc-pool-size,r"
                //desc.add_options( )
                ///  ("cmd1,C",   "desc")
                ///  ( "cmd2,D", po::value<std::string>( ), "desc" )
            }

            std::string help( ) const
            {
                return std::string( );
            }

            std::string desc( ) const
            {
                return std::string( "%test-name% command" );
            }

        };
    }

    namespace %test-name% {
        cmd_sptr create( )
        {
            return cmd_sptr( new impl );
        }
    }

}}}

    """
    return source_file.__doc__

def usage(  ):
    """
    usage: addcmd.py <cmd-name>
    """
    print( usage.__doc__ )

if __name__ == '__main__':
    if len( argv ) < 2:
        usage( )
        exit( 1 )
    
    test_name = argv[1]
    
    src_name = 'cmd-' + test_name + '.cpp';
    
    src_path = os.path.join( src_name )
    
    src_content = source_file(  ).replace( '%test-name%', test_name )
    
    s = open( src_path, 'w' );
    s.write( src_content )

