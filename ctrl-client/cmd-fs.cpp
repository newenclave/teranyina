#include "cmd-iface.h"

#include "boost/program_options.hpp"

#include "iface/IFilesystem.h"
#include "client-core/ta-client.h"

namespace ta { namespace cc { namespace cmd {

    namespace {

        namespace po     = boost::program_options;
        namespace ifaces = ta::client::interfaces;
        typedef client::core core;


        const char *cmd_name = "fs";
        typedef vtrc::unique_ptr<ifaces::filesystem::iface> iface_uptr;

        std::string leaf( const std::string &path )
        {
            auto pos = path.find_last_of( '/' );
            if( std::string::npos == pos ) {
                return path;
            } else {
                return std::string( path.begin( ) + pos + 1, path.end( ) );
            }
        }

        struct impl: public cmd_iface {

            std::string ls_path_;

            const char *name( ) const
            {
                return cmd_name;
            }

            void exec( po::variables_map &vm, client::core &cl )
            {
                if( vm.count( "ls" ) ) {
                    iface_uptr fi(ifaces::filesystem::create(cl,ls_path_));
                    std::set<std::string> dirs;
                    std::set<std::string> files;
                    for( auto d = fi->begin( ); d != fi->end( ); ++d ) {
                        bool is_dir   = d.info( ).is_directory;

                        if( is_dir ) {
                            dirs.insert( leaf(d->path) );
                        } else {
                            files.insert( leaf(d->path) );
                        }
                    }
                    for( auto &d: dirs ) {
                        std::cout << "[" << d << "]" << "\n";
                    }
                    for( auto &f: files ) {
                        std::cout << " " << f << "\n";
                    }
                }
            }

            void add_options( po::options_description &desc )
            {
                /// reserver as common
                /// "help,h" "command,c" "server,s"
                /// "io-pool-size,i" "rpc-pool-size,r"
                desc.add_options( )
                    ( "ls", po::value<std::string>(&ls_path_),
                            "show directory list" )
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

    namespace fs {
        cmd_sptr create( )
        {
            return cmd_sptr( new impl );
        }
    }

}}}


