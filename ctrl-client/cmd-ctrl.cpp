#include <iostream>

#include "cmd-iface.h"

#include "boost/program_options.hpp"

#include "iface/IControl.h"
#include "client-core/ta-client.h"

namespace ta { namespace cc { namespace cmd {

    namespace {

        namespace po     = boost::program_options;
        namespace ifaces = ta::client::interfaces;

        using rpc_channel = vtrc::common::rpc_channel;
        typedef client::core core;


        const char *cmd_name = "ctrl";
        typedef vtrc::unique_ptr<ifaces::control::iface> iface_uptr;

        struct impl: public cmd_iface {

            const char *name( ) const
            {
                return cmd_name;
            }

            void exec( po::variables_map &vm, client::core &client )
            {
                iface_uptr cmd(ifaces::control::create(client));

                cmd->ping( );
                if( vm.count( "shutdown" ) ) {
                    std::cout << "Shuting down agent...";
                    cmd->channel( )->set_flag( rpc_channel::DISABLE_WAIT );
                    cmd->shutdown( );
                    std::cout << "Ok\n";
                }
            }

            void add_options( po::options_description &desc )
            {
                /// reserver as common
                /// "help,h" "command,c" "server,s"
                /// "io-pool-size,i" "rpc-pool-size,r"
                desc.add_options( )
                    ( "shutdown,q",  "shutdown remote agent" )
                    ;
            }

            std::string help( ) const
            {
                return std::string( );
            }

            std::string desc( ) const
            {
                return std::string( "Control remote agent" );
            }

        };
    }

    namespace ctrl {
        cmd_sptr create( )
        {
            return cmd_sptr( new impl );
        }
    }

}}}

    
