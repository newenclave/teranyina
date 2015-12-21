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
    }

    struct application::impl {

        subsystem_comtrainer    subsystems_;
        vcomm::pool_pair        pools_;
        logger                  logger_;

        impl( )
            :pools_(0, 0)
            ,logger_(pools_.get_io_service( ), logger::level::info)
        { }
    };

    application::application( )
        :impl_(new impl)
    { }

    application::~application( )
    {
        delete impl_;
    }

    void application::add_subsystem( const std::type_info &info,
                                     subsystem_sptr inst )
    {
        impl_->subsystems_.subsys_[vcomm::rtti_wrapper(info)] = inst;
        impl_->subsystems_.subsys_order_.push_back( inst );
    }

    subsystem_iface *application::subsystem( const std::type_info &info )
    {
        auto f = impl_->subsystems_.subsys_.find( vcomm::rtti_wrapper(info) );

        if( f == impl_->subsystems_.subsys_.end( ) ) {
            return nullptr;
        } else {
            return f->second.get( );
        }
    }

    const
    subsystem_iface *application::subsystem( const std::type_info &info ) const
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

    void application::run( int argc, const char **argv )
    {
        impl_->pools_.get_io_pool( ).add_thread( );
        impl_->pools_.get_io_pool( ).attach( );
    }

}}
