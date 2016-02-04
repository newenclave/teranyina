
#include "subsys-logging.h"
#include "../application.h"
#include "../logger.h"

//#include "subsys-log.h"

//#include "vtrc-memory.h"

//#define LOG(lev) log_(lev) << "[logging] "
//#define LOGINF   LOG(logger::info)
//#define LOGDBG   LOG(logger::debug)
//#define LOGERR   LOG(logger::error)
//#define LOGWRN   LOG(logger::warning)

namespace ta { namespace agent { namespace subsys {

    namespace {
        const std::string subsys_name( "logging" );

        inline logger::level str2lvl( const char *str )
        {
            return logger::str2level( str );
        }

        void str2logger( const std::string &str )
        {
            size_t delim_pos = str.find_last_of( '[' );
            std::string path;
            std::string from_lvl;
            std::string to_lvl;

            if( delim_pos == std::string::npos ) {
                path = str;
            } else {

                path = std::string( str.begin( ), str.begin( ) + delim_pos );
                auto *to = &from_lvl;
                bool found_int = true;

                for( auto d = ++delim_pos; d < str.size( ); ++d ) {
                    switch( str[d] ) {
                    case '-':
                        to->assign( str.begin( ) + delim_pos,
                                    str.begin( ) + d );
                        to = &to_lvl;
                        delim_pos = d + 1;
                        break;
                    case ']':
                        to->assign( str.begin( ) + delim_pos,
                                    str.begin( ) + d );
                        found_int = false;
                        break;
                    }
                }

                if( found_int ) {
                    to->assign( str.begin( ) + delim_pos, str.end( ) );
                }

                std::cout << "1Log file: " << path << " "
                             << from_lvl.size( ) << " "
                             << to_lvl.size( ) << "\n";
            }
        }

    }

    struct logging::impl {

        application     *app_;
        common::logger  &log_;

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
            str2logger( d );
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

    
