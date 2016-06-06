
#include "cmd-iface.h"
#include "boost/program_options.hpp"

#include "client-core/iface/IScripting.h"

namespace ta { namespace cc { namespace cmd {

    namespace {

        namespace po = boost::program_options;
        namespace ifaces = ta::client::interfaces;
        typedef client::core core;

        const char *cmd_name = "scripting";

        struct impl: public cmd_iface {

            const char *name( ) const
            {
                return cmd_name;
            }

            void exec( po::variables_map &vm, core &client )
            {
                using ciface = ifaces::scripting::iface;
                std::unique_ptr<ciface> cl(ifaces::scripting::create(client));

                std::string name = vm.count( "name" )
                        ? vm["name"].as<std::string>( )
                        : "";

                cl->init( );

                if( vm.count( "buffer" ) ) {
                    auto buf = vm["buffer"].as<std::string>( );
                    cl->execute_buffer( buf, name );
                } else if( vm.count( "file" ) ) {
                    /// file
                }
            }

            void add_options( po::options_description &desc )
            {
                /// reserved as common
                /// "help,h"
                /// "command,c"
                /// "server,s"
                /// "io-pool-size,i"
                /// "rpc-pool-size,r"
                desc.add_options( )
                    ( "function,f", po::value<std::string>( ),
                                 "execute function; for -B or -F" )
                    ( "file,F", po::value<std::string>( ),   "execute file" )
                    ( "buffer,B", po::value<std::string>( ), "execute buffer" )
                    ( "name,n", po::value<std::string>( ),   "buffer name" )
                        ;
            }

            std::string help( ) const
            {
                return std::string( );
            }

            std::string desc( ) const
            {
                return std::string( "scripting command" );
            }

        };
    }

    namespace scripting {
        cmd_sptr create( )
        {
            return cmd_sptr( new impl );
        }
    }

}}}

    
