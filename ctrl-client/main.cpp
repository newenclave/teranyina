#include <iostream>
#include <iomanip>

#include "client-core/ta-client.h"

#include "boost/program_options.hpp"
#include "boost/system/error_code.hpp"

#include "google/protobuf/descriptor.h"

#include "vtrc-common/vtrc-exception.h"
#include "vtrc-common/vtrc-hash-iface.h"

#include "client-core/ta-client.h"

#include "cmd-list.h"

namespace po = boost::program_options;

namespace vcommon = vtrc::common;

namespace {

    typedef std::map<std::string, ta::cc::cmd_sptr> cmd_map;
    typedef vtrc::shared_ptr<vcommon::pool_pair> pool_pair_sptr;

    cmd_map::value_type cmd( ta::cc::cmd_sptr ptr )
    {
        return std::make_pair( ptr->name( ), ptr );
    }

    void fill_common_options( po::options_description &desc )
    {
        desc.add_options( )

            ("help,h",   "help message")

            ( "command,c", po::value<std::string>( ), "command to exec" )

            ("server,s", po::value<std::string>( ),
                    "endpoint name; <tcp address>:<port> or <file name>")

            ("id", po::value<std::string>( ),
                    "set client ID for the remote agent")

            ("key", po::value<std::string>( ),
                    "set client KEY for the remote agent")

            ("io-pool-size,i",  po::value<unsigned>( ),
                    "threads for io operations; default = 1")

            ("rpc-pool-size,r", po::value<unsigned>( ),
                    "threads for rpc calls; default = 1")

            ;
    }

    cmd_map create_map( ta::cc::cmd_sptr &def )
    {
        cmd_map res;
        def = ta::cc::cmd::ctrl::create( );

        res.insert( cmd( def ) );
        res.insert( cmd( ta::cc::cmd::fs::create( ) ) );

        return res;
    }

    pool_pair_sptr pp_from_cmd( const po::variables_map &vm )
    {

        unsigned io_size = vm.count( "io-pool-size" )
                ? vm["io-pool-size"].as<unsigned>( )
                : 1;

        unsigned rpc_size = vm.count( "rpc-pool-size" )
                ? vm["rpc-pool-size"].as<unsigned>( )
                : 1;

        if( io_size < 1) {
            throw std::runtime_error( "io_size must be at least 1" );
        }

        if( rpc_size < 1) {
            throw std::runtime_error( "rpc-pool-size must be at least 1" );
        }

        return vtrc::make_shared<vcommon::pool_pair>( io_size, rpc_size );
    }

    po::variables_map parse_common( int argc, const char **argv,
                                    const po::options_description &desc )
    {
        po::variables_map vm;
        po::parsed_options parsed (
            po::command_line_parser( argc, argv )
                .options(desc)
                .allow_unregistered( )
                .run( ));
        po::store(parsed, vm);
        po::notify(vm);
        return vm;
    }

    po::variables_map parse_command( int argc, const char **argv,
                                     ta::cc::cmd_sptr command,
                                     po::options_description &desc )
    {
        command->add_options( desc );
        po::variables_map vm;
        po::parsed_options parsed (
            po::command_line_parser( argc, argv )
                .options(desc)
                .run( ));
        po::store(parsed, vm);
        po::notify(vm);
        return vm;
    }

    void show_init_help( const po::options_description &desc,
                         const cmd_map &cm)
    {
        std::cout << "Usage: teranyina_client -c <command> [command opts]\n"
            << "      'teranyina_client -h' for this help\n"
            << "      'teranyina_client -c <command> -h'"
            << " for command's help\n"
            << desc << "\n";

        std::cout << "Commands (" << cm.size( ) << " total)" << ": \n";

        for( cmd_map::const_iterator b(cm.begin( )),
             e(cm.end( )); b!=e; ++b )
        {
            std::cout << "\t" << b->first
                      << ": " << b->second->desc( )
                      << "\n";
        }
    }

    void show_command_help( const po::options_description &cdesc,
                            ta::cc::cmd_sptr &command )
    {
        po::options_description desc;
        command->add_options( desc );

        std::cout << cdesc << "\n";
        std::cout << "Usage: ferro_remote_client -c " << command->name( )
                  << " [opts]\n"
                  << "Command options: \n" << desc << "\n";
    }

}

namespace {

    void on_connect( )
    {
        std::cout << "connect...";
    }

    void on_disconnect( )
    {
        std::cout << "disconnect...\n";
    }

    void on_ready( )
    {
        std::cout << "ready...\n";
    }

    void on_init_error( const char *message )
    {
        std::cout << "init error '" << message << "'\n";
    }
}

int main( int argc, const char **argv )
{ try {

    po::options_description desc("Common options");
    fill_common_options( desc );
    po::variables_map vm( parse_common( argc, argv, desc ) );

    ta::cc::cmd_sptr current_command;
    cmd_map cm = create_map( current_command );

    if( vm.count( "command" ) ) {
        std::string c(vm["command"].as<std::string>( ));
        cmd_map::iterator f(cm.find( c ));
        if( f == cm.end( ) ) {
            std::cout << "Invalid command '" << c << "'\n";
            show_init_help( desc, cm );
            return 1;
        } else {
            current_command = f->second;
        }
    }

    if( vm.count("help") ) {
        current_command.get( ) ? show_command_help( desc, current_command )
                               : show_init_help( desc, cm );
        return 0;
    }

    if( !current_command.get( ) ) {
        std::cout << "Invalid command '<empty>'\n";
        show_init_help( desc, cm );
        return 2;
    }

    std::string server( vm.count("server")
                        ? vm["server"].as<std::string>( )
                        : "");

    if( server.empty( ) && current_command->need_connect( ) ) {
        std::cout << "Invalid server '<empty>'\n";
        show_init_help( desc, cm );
        return 3;
    }

    std::string c_id( vm.count("id")
                        ? vm["id"].as<std::string>( )
                        : "");

    std::string c_key( vm.count("key")
                        ? vm["key"].as<std::string>( )
                        : "");

    pool_pair_sptr pp(pp_from_cmd( vm ));

    ta::client::core client(*pp);

    if( !c_id.empty( ) ) {
        client.get_client( ).set_session_id( c_id );
    }

    if( !c_key.empty( ) ) {
        std::string key_info( c_id + c_key );
        vcommon::hash_iface_uptr s(vcommon::hash::sha2::create256( ));
        std::string hs(s->get_data_hash( &key_info[0], key_info.size( ) ));
        client.get_client( ).set_session_key( hs );
    }

    client.on_connect_connect   ( &on_connect    );
    client.on_disconnect_connect( &on_disconnect );
    client.on_ready_connect     ( &on_ready      );
    client.on_init_error_connect( &on_init_error );

    vm = parse_command( argc, argv, current_command, desc );

    if( current_command->need_connect( ) ) {
        std::cout << "Connecting...\n";
        //client.async_connect( server, connect_success );
        client.connect( server );
    }

    current_command->exec( vm, client );

    pp->stop_all( );
    pp->join_all( );

    google::protobuf::ShutdownProtobufLibrary( );

    return 0;

} catch( const vtrc::common::exception &ex ) {
    std::cerr << "Protocol client error: "
              << ex.what( ) << ": "
              << ex.additional( )
              << "\n";
    return 10;
} catch( const std::exception &ex ) {
    std::cerr << "General client error: " << ex.what( ) << "\n";
    return 10;
} }
