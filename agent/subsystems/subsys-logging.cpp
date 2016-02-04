#include <memory>
#include <chrono>

#include "subsys-logging.h"
#include "../application.h"
#include "../logger.h"

#include "boost/date_time/posix_time/posix_time.hpp"

//#include "subsys-log.h"

//#include "vtrc-memory.h"

#define LOG(lev) log_(lev) << "[logging] "
#define LOGINF   LOG(logger::level::info)
#define LOGDBG   LOG(logger::level::debug)
#define LOGERR   LOG(logger::level::error)
#define LOGWRN   LOG(logger::level::warning)

namespace ta { namespace agent { namespace subsys {

    namespace {

        const std::string subsys_name( "logging" );
        namespace bsig = boost::signals2;
        namespace bpt  = boost::posix_time;

        inline logger::level str2lvl( const char *str )
        {
            return logger::str2level( str );
        }

        struct connection_info {
            bsig::connection conn_;
        };

        struct ostream_inf {
            std::unique_ptr<std::ostream> stream_;
            size_t           length_;
            ostream_inf( )
                :length_(0)
            { }
        };

        std::string str2logger( const std::string &str,
                                std::string &fromlvl, std::string &tolvl )
        {
            size_t delim_pos = str.find_last_of( '[' );
            std::string path;

            if( delim_pos == std::string::npos ) {
                path = str;
            } else {

                path = std::string( str.begin( ), str.begin( ) + delim_pos );
                auto *to = &fromlvl;
                bool found_int = true;

                for( auto d = ++delim_pos; d < str.size( ); ++d ) {
                    switch( str[d] ) {
                    case ']':
                        found_int = false;
                    case '-':
                        to->assign( str.begin( ) + delim_pos,
                                    str.begin( ) + d );
                        to = &tolvl;
                        delim_pos = d + 1;
                        break;
                    }
                }

                if( found_int ) {
                    to->assign( str.begin( ) + delim_pos, str.end( ) );
                }
            }

            return path;
        }

    }

    struct logging::impl {

        application     *app_;
        agent::logger   &log_;

        connection_info  stdout_connection_;
        connection_info  stderr_connection_;

        impl( application *app )
            :app_(app)
            ,log_(app_->get_logger( ))
        { }

        void reg_creator( const std::string &name,
                          application::service_getter_type func )
        {
            app_->register_service_creator( name, func );
        }

        void unreg_creator( const std::string &name )
        {
            app_->unregister_service_creator( name );
        }

        void console_log( std::ostream &o, common::logger::level lvl,
                          bpt::ptime const &tim, std::string const &data )
        {
            o << tim << " [" << agent::logger::level2str(lvl) << "] "
              << data << std::endl;
        }

        void add_logger( const std::string &path, const std::string &from,
                         const std::string &to )
        {
            namespace ph = std::placeholders;
            if( path == "stdout" ) { /// cout

                stdout_connection_.conn_.disconnect( );
                stdout_connection_.conn_ = log_.on_write_connect(
                            std::bind( &impl::console_log, this,
                                       std::ref(std::cout),
                                       ph::_1, ph::_2, ph::_3 ) );

            } else if( path == "stderr" ) { /// cerr

                stderr_connection_.conn_.disconnect( );
                stderr_connection_.conn_ = log_.on_write_connect(
                            std::bind( &impl::console_log, this,
                                       std::ref(std::cerr),
                                       ph::_1, ph::_2, ph::_3 ) );

            } else {

            }
        }

    };


    logging::logging( application *app )
        :impl_(new impl(app))
    { }

    logging::~logging( )
    {
        delete impl_;
    }

    /// static
    vtrc::shared_ptr<logging> logging::create(application *app,
                              const std::vector<std::string> &def )
    {
        vtrc::shared_ptr<logging> new_inst(new logging(app));

        for( auto &d: def ) {
            std::string from;
            std::string to;
            auto path = str2logger( d, from, to );
            new_inst->impl_->add_logger( path, from, to );
        }

        return new_inst;
    }

    const std::string &logging::name( )  const
    {
        return subsys_name;
    }

    void logging::init( )
    {

    }

    void logging::start( )
    {
//        impl_->LOGINF << "Started.";
    }

    void logging::stop( )
    {
//        impl_->LOGINF << "Stopped.";
    }


}}}

    
