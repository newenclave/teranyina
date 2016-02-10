#include <iostream>
#include <fstream>

#include "application.h"
#include "vtrc-common/vtrc-pool-pair.h"

#include "vtrc-mutex.h"

#include "logger.h"

#include "boost/program_options.hpp"

#include "subsystems/subsys-list.hxx"

#include "openssl/rsa.h"

#include "google/protobuf/stubs/common.h"

namespace ta { namespace agent {

    namespace {

        namespace po    = boost::program_options;
        namespace vcomm = vtrc::common;
        namespace gpb   = google::protobuf;

        using subsys_map    = std::map<vcomm::rtti_wrapper, subsystem_sptr>;
        using subsys_vector = std::vector<subsystem_sptr>;
        using string_vector = std::vector<std::string>;

        using service_map   =  std::map< std::string,
                                         application::service_getter_type >;

        struct subsystem_comtrainer {
            subsys_map      subsys_;
            subsys_vector   subsys_order_;
        };

        void get_options( po::options_description& desc )
        {
//            desc.add_options( )
//                ( "help,h", "help message" )
//            ;
        }

        po::variables_map create_cmd_params( int argc, const char *argv[ ],
                                             po::options_description const dsc )
        {
            po::variables_map vm;
            po::parsed_options parsed (
                po::command_line_parser(argc, argv)
                    .options(dsc)
                    .allow_unregistered( )
                    .run( ));
            po::store(parsed, vm);
            po::notify(vm);
            return vm;
        }

        po::variables_map create_file_params( const std::string &path,
                                           po::options_description const dsc )
        {
            po::variables_map vm;
            std::ifstream ini_file(path);
            po::store(po::parse_config_file( ini_file, dsc, true ), vm);
            po::notify( vm );
            return vm;
        }

        void show_help( po::options_description const &desc )
        {
            std::cout << "Usage: ./teranyina_agent <options>\n"
                      << "Options: \n" << desc;
        }
    }

    struct application::impl: public vtrc::server::application {

        agent::application   *parent_;
        subsystem_comtrainer  subsystems_;
        logger                logger_;

        service_map           services_;
        vtrc::mutex           services_lock_;

        unsigned              io_count_;
        unsigned              rpc_count_;

        string_vector         servers_;
        string_vector         mcs_;
        string_vector         loggers_;
        std::string           log_level_;

        std::string           config_file_;

        impl(vcomm::pool_pair &pools)
            :vtrc::server::application(pools)
            ,logger_(pools.get_io_service( ), logger::level::info)
            ,io_count_(1)
            ,rpc_count_(1)
        { }

        void get_common_options( po::options_description& desc )
        {
            desc.add_options( )
            ( "help,h", "help message" )
            ( "config,c", po::value<std::string>(&config_file_),
                       "config ini-file" )
             ;
        }

        void get_file_options( po::options_description& desc )
        {
            desc.add_options( )
            /// threading
            ( "threads.io", po::value<unsigned>(&io_count_)->default_value(1),
                      "io threads count" )
            ( "threads.rpc", po::value<unsigned>(&rpc_count_)->default_value(1),
                      "rpc threads count" )

            /// listeners
            ( "server.value", po::value<string_vector>(&servers_),
                      "servers points" )

            /// multicast
            ( "multicast.value", po::value<string_vector>(&mcs_),
                       "multicast listeners" )
            /// logger output
            ( "logger.value", po::value<string_vector>(&loggers_),
                       "logger output device; /path/to/logger[[level[-level]]]")
            ( "logger.level", po::value<std::string>(&log_level_)
                                           ->default_value("dbg"),
                       "default logger level; err, inf, wrn, dbg[default]")
            ;
        }

        void get_cmd_options( po::options_description& desc )
        {
            desc.add_options( )

            ( "io,i", po::value<unsigned>(&io_count_)->default_value(1),
                      "io threads count" )

            ( "rpc,r", po::value<unsigned>(&rpc_count_)->default_value(1),
                      "rpc threads count" )

            ( "multicast,m", po::value<string_vector>(&mcs_),
                       "multicast listeners" )

            ( "server,s", po::value<string_vector>(&servers_),
                       "servers points" )
            ( "log,o", po::value<string_vector>(&loggers_),
                       "logger output device; /path/to/logger[:level[:level]]")
            ( "level,l", po::value<std::string>(&log_level_)
                         ->default_value("dbg"),
                       "default logger level; err, inf, wrn, dbg[default]")


            ;
        }

        void init_subsystems( )
        {
            using namespace agent;
            parent_->add_subsystem<subsys::logging>  ( loggers_ );
            parent_->add_subsystem<subsys::security> ( );
            parent_->add_subsystem<subsys::multicast>( mcs_ );
            parent_->add_subsystem<subsys::control>  ( );
            parent_->add_subsystem<subsys::listeners>( servers_ );
            parent_->add_subsystem<subsys::clients>  ( );
        }

        vtrc::shared_ptr<vtrc::common::rpc_service_wrapper>
        get_service_by_name( vtrc::common::connection_iface *cl,
                             const std::string &name ) override
        {
            vtrc::lock_guard<vtrc::mutex> lck(services_lock_);
            auto f = services_.find( name );
            parent_->get_logger( )( logger::level::info ) << "Return " << name;
            if( f != services_.end( ) ) {
                parent_->get_logger( )( logger::level::info ) << "Return " << name;
                return f->second( parent_, cl->weak_from_this( ) );
            }
            return vtrc::shared_ptr<vtrc::common::rpc_service_wrapper>( );
        }

    };

    application::application( )
        :pools_(0, 0)
        ,impl_(new impl(pools_))
    {
        impl_->parent_ = this;
    }

    application::~application( )
    {
        delete impl_;
    }

    void application::add_subsys( const std::type_info &info,
                                  subsystem_sptr inst )
    {
        impl_->subsystems_.subsys_[vcomm::rtti_wrapper(info)] = inst;
        impl_->subsystems_.subsys_order_.push_back( inst );
    }

    subsystem_iface *
    application::get_subsys( const std::type_info &info ) noexcept
    {
        auto f = impl_->subsystems_.subsys_.find( vcomm::rtti_wrapper(info) );

        if( f == impl_->subsystems_.subsys_.end( ) ) {
            return nullptr;
        } else {
            return f->second.get( );
        }
    }

    const subsystem_iface *
    application::get_subsys( const std::type_info &info ) const noexcept
    {
        auto f = impl_->subsystems_.subsys_.find( vcomm::rtti_wrapper(info) );

        if( f == impl_->subsystems_.subsys_.end( ) ) {
            return nullptr;
        } else {
            return f->second.get( );
        }
    }

    void application::start_all( )
    {
        typedef subsys_vector::iterator iter_type;
        subsys_vector &vec(impl_->subsystems_.subsys_order_);

        for( iter_type b(vec.begin( )), e(vec.end( )); b!=e; ++b ) {
            (*b)->init( );
        }

        for( iter_type b(vec.begin( )), e(vec.end( )); b!=e; ++b ) {
            (*b)->start( );
        }
    }

    void application::stop_all( )
    {
        typedef subsys_vector::reverse_iterator iter_type;
        subsys_vector &vec(impl_->subsystems_.subsys_order_);

        for( iter_type b(vec.rbegin( )), e(vec.rend( )); b!=e; ++b ) {
            (*b)->stop( );
        }

    }

    vtrc::server::application *application::get_application( ) noexcept
    {
        return impl_;
    }

    const
    vtrc::server::application *application::get_application( ) const noexcept
    {
        return impl_;
    }

    boost::asio::io_service &application::get_io_service( ) noexcept
    {
        return impl_->get_io_service( );
    }

    boost::asio::io_service &application::get_rpc_service( ) noexcept
    {
        return impl_->get_rpc_service( );
    }

    //////// service wrapper

    application::service_wrapper_impl::service_wrapper_impl( application *app,
                              vcomm::connection_iface_wptr c,
                              vtrc::shared_ptr<gpb::Service> serv)
        :vcomm::rpc_service_wrapper( serv )
        ,app_(app)
        ,client_(c)
    { }

    application::service_wrapper_impl::~service_wrapper_impl( )
    { }

    const
    application::service_wrapper_impl::method_type *
                application::service_wrapper_impl
                           ::get_method( const std::string &name ) const
    {
        const method_type* m = super_type::find_method( name );
        return m;
    }

    application
    *application::service_wrapper_impl::get_application( ) noexcept
    {
        return app_;
    }

    const application
    *application::service_wrapper_impl::get_application( ) const noexcept
    {
        return app_;
    }

    /////////////////////////////

    application::service_wrapper_sptr
    application::wrap_service ( vtrc::common::connection_iface_wptr cl,
                                service_wrapper_impl::service_sptr serv )
    {
        return std::make_shared<application::service_wrapper>(this, cl, serv);
    }

    void application::register_service_creator( const std::string &name,
                                                service_getter_type func )
    {
        vtrc::lock_guard<vtrc::mutex> lck(impl_->services_lock_);

        get_logger( )( logger::level::info ) << "[     app] Add service " << name;

        auto f = impl_->services_.find( name );
        if( f != impl_->services_.end( ) ) {
            std::ostringstream oss;
            oss << "Service '" << name << "'' already exists.";
            throw std::runtime_error( oss.str( ) );
        }
        impl_->services_.insert( std::make_pair( name, func ) );
    }

    void application::unregister_service_creator( const std::string &name )
    {
        vtrc::lock_guard<vtrc::mutex> lck(impl_->services_lock_);
        auto f = impl_->services_.find( name );
        if( f != impl_->services_.end( ) ) {
            impl_->services_.erase( f );
        }
    }

    void application::quit( )
    {
        pools_.stop_all( );
    }

    agent::logger &application::get_logger( ) noexcept
    {
        return impl_->logger_;
    }

    const agent::logger &application::get_logger( ) const noexcept
    {
        return impl_->logger_;
    }

    void application::run( int argc, const char *argv[ ] )
    {
        po::options_description desc;

        get_options( desc );
        impl_->get_common_options( desc );

        auto params = create_cmd_params( argc, argv, desc );

        impl_->get_cmd_options( desc );

        if( params.count( "help" ) ) {
            show_help( desc );
            return;
        }

        if( !impl_->config_file_.empty( ) )  {
            po::options_description init_desc;
            impl_->get_file_options( init_desc );
            create_file_params( impl_->config_file_, init_desc );
        } else {
            create_cmd_params( argc, argv, desc );
        }

        get_logger( ).set_level(
                    logger::str2level( impl_->log_level_.c_str( ) ) );

        impl_->init_subsystems( );
        start_all( );

        pools_.get_io_pool( ).add_threads( impl_->io_count_ - 1 );
        pools_.get_rpc_pool( ).add_threads( impl_->rpc_count_ );

        pools_.get_io_pool( ).attach( );

        pools_.join_all( );
        google::protobuf::ShutdownProtobufLibrary( );

    }

}}
