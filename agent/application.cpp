#include <iostream>
#include <fstream>

#include "application.h"

#include "vtrc-common/vtrc-pool-pair.h"
#include "vtrc-common/protocol/vtrc-rpc-lowlevel.pb.h"
#include "vtrc-mutex.h"

#include "boost/program_options.hpp"
#include "boost/asio/io_service.hpp"

#include "subsystems/subsys-list.hxx"

#include "openssl/rsa.h"
#include "logger.h"

#include "common/utils.h"

namespace ta { namespace agent {

    namespace {

        namespace po    = boost::program_options;
        namespace vcomm = vtrc::common;
        namespace gpb   = google::protobuf;

#ifdef TA_RTTI_DISABLE
        using subsys_map    = std::map<std::uintptr_t, subsystem_sptr>;
#else
        using subsys_map    = std::map<vcomm::rtti_wrapper, subsystem_sptr>;
#endif
        using subsys_vector = std::vector<subsystem_sptr>;
        using string_vector = std::vector<std::string>;

        using service_map   =  std::map< std::string,
                                         application::service_getter_type >;

        using level = logger::level;

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

        void show_help( po::options_description const &desc )
        {
            std::cout << "Usage: ./teranyina_agent <options>\n"
                      << "Options: \n" << desc;
        }
        THREAD_LOCAL std::string s_thread_prefix;
    }

    struct application::impl: public vtrc::server::application {

        vcomm::pool_pair     &pools_;
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

        impl( vcomm::pool_pair &pp )
            :vtrc::server::application(pp)
            ,pools_(pp)
            ,logger_(pools_.get_io_service( ), logger::level::info)
            ,io_count_(1)
            ,rpc_count_(1)
        {
//            assign_io_service(  pools_.get_io_service( )  );
//            assign_rpc_service( pools_.get_rpc_service( ) );
        }

        void get_common_options( po::options_description& desc )
        {
            desc.add_options( )
            ( "help,h", "help message" )
            ( "config,c", po::value<std::string>(&config_file_),
                       "config ini-file" )
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
            parent_->add_subsystem<subsys::control>  ( );
            parent_->add_subsystem<subsys::fs>       ( );

            parent_->add_subsystem<subsys::multicast>( mcs_ );
            parent_->add_subsystem<subsys::listeners>( servers_ );
            parent_->add_subsystem<subsys::clients>  ( );

            parent_->add_subsystem<subsys::lua>      ( config_file_ );
        }

        vtrc::shared_ptr<vtrc::common::rpc_service_wrapper>
        get_service_by_name( vtrc::common::connection_iface *cl,
                             const std::string &name ) override
        {
            vtrc::lock_guard<vtrc::mutex> lck(services_lock_);
            auto f = services_.find( name );
            if( f != services_.end( ) ) {
                return f->second( parent_, cl->weak_from_this( ) );
            }

            logger_(level::debug, "app")
                    << " service " << name << " was not found.";

            return vtrc::shared_ptr<vtrc::common::rpc_service_wrapper>( );
        }

        void stop_all( )
        {
            parent_->stop_all( );
            logger_.dispatch( [this]( ) {
                pools_.stop_all( );
            } );
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

#ifdef TA_RTTI_DISABLE
    void application::add_subsys( std::uintptr_t info,
                                  subsystem_sptr inst )
    {
        get_logger( )(level::debug, "app")
                << "add subsystem with id 0x"
                << std::hex << info
                << " " << inst->name( );
        impl_->subsystems_.subsys_[info] = inst;
        impl_->subsystems_.subsys_order_.push_back( inst );
    }

    subsystem_iface *
    application::get_subsys( std::uintptr_t info ) NOEXCEPT
    {
        auto f = impl_->subsystems_.subsys_.find( info );

        if( f == impl_->subsystems_.subsys_.end( ) ) {
            return nullptr;
        } else {
            return f->second.get( );
        }
    }

    const subsystem_iface *
    application::get_subsys( std::uintptr_t info ) const NOEXCEPT
    {
        auto f = impl_->subsystems_.subsys_.find( info );

        if( f == impl_->subsystems_.subsys_.end( ) ) {
            return nullptr;
        } else {
            return f->second.get( );
        }
    }

#else
    void application::add_subsys( const std::type_info &info,
                                  subsystem_sptr inst )
    {
        impl_->subsystems_.subsys_[vcomm::rtti_wrapper(info)] = inst;
        impl_->subsystems_.subsys_order_.push_back( inst );
    }

    subsystem_iface *
    application::get_subsys( const std::type_info &info ) NOEXCEPT
    {
        auto f = impl_->subsystems_.subsys_.find( vcomm::rtti_wrapper(info) );

        if( f == impl_->subsystems_.subsys_.end( ) ) {
            return nullptr;
        } else {
            return f->second.get( );
        }
    }

    const subsystem_iface *
    application::get_subsys( const std::type_info &info ) const NOEXCEPT
    {
        auto f = impl_->subsystems_.subsys_.find( vcomm::rtti_wrapper(info) );

        if( f == impl_->subsystems_.subsys_.end( ) ) {
            return nullptr;
        } else {
            return f->second.get( );
        }
    }
#endif

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

    bool application::is_ctrl_connection( vtrc::common::connection_iface */*c*/)
    {
        return true;
    }

    vtrc::server::application *application::get_application( ) NOEXCEPT
    {
        return impl_;
    }

    const
    vtrc::server::application *application::get_application( ) const NOEXCEPT
    {
        return impl_;
    }

    boost::asio::io_service &application::get_io_service( ) NOEXCEPT
    {
        return impl_->get_io_service( );
    }

    boost::asio::io_service &application::get_rpc_service( ) NOEXCEPT
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
    *application::service_wrapper_impl::get_application( ) NOEXCEPT
    {
        return app_;
    }

    const application
    *application::service_wrapper_impl::get_application( ) const NOEXCEPT
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

    void application::register_service_factory( const std::string &name,
                                                service_getter_type func )
    {
        vtrc::lock_guard<vtrc::mutex> lck(impl_->services_lock_);

        get_logger( )( logger::level::debug, "app" )
                << "Add service " << name;

        auto f = impl_->services_.find( name );
        if( f != impl_->services_.end( ) ) {
            std::ostringstream oss;
            oss << "Service '" << name << "'' already exists.";
            throw std::runtime_error( oss.str( ) );
        }
        impl_->services_.insert( std::make_pair( name, func ) );
    }

    void application::unregister_service_factory( const std::string &name )
    {
        vtrc::lock_guard<vtrc::mutex> lck(impl_->services_lock_);
        auto f = impl_->services_.find( name );
        if( f != impl_->services_.end( ) ) {
            impl_->services_.erase( f );
        }
    }

    void application::quit( )
    {
        impl_->get_rpc_service( ).post( [this]( ) {impl_->stop_all( );} );
    }

    agent::logger &application::get_logger( ) NOEXCEPT
    {
        return impl_->logger_;
    }

    const agent::logger &application::get_logger( ) const NOEXCEPT
    {
        return impl_->logger_;
    }

    const std::string &application::thread_ptrfix( )
    {
        return s_thread_prefix;
    }

    vcomm::thread_pool::thread_decorator create_decorator( std::string pref )
    {
        using tc_type = vcomm::thread_pool::call_decorator_type;
        return [ pref ]( tc_type t ) {
            switch ( t ) {
            case vcomm::thread_pool::CALL_PROLOGUE:
                s_thread_prefix = pref;
                break;
            case vcomm::thread_pool::CALL_EPILOGUE:
                s_thread_prefix = "";
                break; /// do not add default: here
            }
        };
    }

    void application::run( int argc, const char *argv[ ] )
    {
        po::options_description desc;

        s_thread_prefix = "M";

        get_options( desc );
        impl_->get_common_options( desc );

        auto params = create_cmd_params( argc, argv, desc );

        impl_->get_cmd_options( desc );

        if( params.count( "help" ) ) {
            show_help( desc );
            return;
        }

        create_cmd_params( argc, argv, desc );

        get_logger( ).set_level(
                    logger::str2level( impl_->log_level_.c_str( ) ) );

        impl_->init_subsystems( );

        start_all( );

        impl_->pools_.get_io_pool( ).assign_exception_handler( [this]( ) {
            try {
                throw;
            } catch( const std::exception &ex ) {
                get_logger( )( logger::level::error )
                        << "Exception at io thread: "
                        << std::hex << "0x " << std::this_thread::get_id( )
                        << ": " << ex.what( );
                           ;
            } catch( ... ) {
                get_logger( )( logger::level::error )
                        << "Bad exception at io thread: "
                        << std::hex << "0x " << std::this_thread::get_id( );
                std::terminate( );
            }
        } );

        impl_->pools_.get_rpc_pool( ).assign_exception_handler( [this]( ) {
            try {
                throw;
            } catch( const std::exception &ex ) {
                get_logger( )( logger::level::error, "rpc thread" )
                        << "Exception: "
                        << std::hex << "0x " << std::this_thread::get_id( )
                        << ": " << ex.what( )
                           ;
            } catch( ... ) {
                get_logger( )( logger::level::error, "rpc thread" )
                    << "Bad exception: "
                    << std::hex << "0x " << std::this_thread::get_id( )
                       ;
                 //std::terminate( );
            }
        } );

        auto &pools( impl_->pools_ );

        pools.get_io_pool( ).assign_thread_decorator( create_decorator( "I" ));
        pools.get_rpc_pool( ).assign_thread_decorator( create_decorator( "R" ));
        pools.get_io_pool( ).add_threads( impl_->io_count_ - 1 );
        pools.get_rpc_pool( ).add_threads( impl_->rpc_count_ );
        pools.get_io_pool( ).attach( create_decorator( "M" ) );
        pools.join_all( );
    }

}}
