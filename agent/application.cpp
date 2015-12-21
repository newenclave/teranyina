#include "application.h"
#include "vtrc-common/vtrc-pool-pair.h"

#include "logger.h"

#include "boost/program_options.hpp"

namespace ta { namespace agent {

    namespace {

        namespace po = boost::program_options;

        namespace vcomm = vtrc::common;

        typedef std::map<vcomm::rtti_wrapper, subsystem_sptr> subsys_map;
        typedef std::vector<subsystem_sptr>                   subsys_vector;

        struct subsystem_comtrainer {
            subsys_map      subsys_;
            subsys_vector   subsys_order_;
        };

        void get_options( po::options_description& desc )
        {
            desc.add_options( )
                ( "help,h", "help message" )
            ;
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
    }

    struct application::impl {

        subsystem_comtrainer    subsystems_;
        vcomm::pool_pair        pools_;
        logger                  logger_;

        unsigned                io_count_;
        unsigned                rpc_count_;

        impl( )
            :pools_(0, 0)
            ,logger_(pools_.get_io_service( ), logger::level::info)
            ,io_count_(1)
            ,rpc_count_(1)
        { }

        void get_agent_options( po::options_description& desc )
        {
            desc.add_options( )
            ( "io,i", po::value<unsigned>(&io_count_)->default_value(1),
                      "io threads count" )
            ( "rpc,r", po::value<unsigned>(&rpc_count_)->default_value(1),
                      "rpc threads count" )
            ;
        }

    };

    application::application( )
        :impl_(new impl)
    { }

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

    void application::run( int argc, const char *argv[ ] )
    {
        po::options_description desc;

        get_options( desc );
        impl_->get_agent_options( desc );

        auto params = create_cmd_params( argc, argv, desc );
        if( params.count( "help" ) ) {
            show_help( desc );
            return;
        }

        impl_->pools_.get_io_pool( ).add_threads( impl_->io_count_ - 1 );
        impl_->pools_.get_rpc_pool( ).add_threads( impl_->rpc_count_ );

        impl_->pools_.get_io_pool( ).attach( );
    }

}}
