#include "logger.h"
#include <chrono>
#include "boost/date_time/posix_time/posix_time.hpp"

namespace ta { namespace agent {

    namespace bpt = boost::posix_time;

    logger::logger( level lvl )
        :common::logger( lvl )
    { }

    void logger::send_data( level lev, const std::string &data )
    {
        static const bpt::ptime epoch(bpt::ptime::date_type(1970, 1, 1));

        bpt::ptime local_time = bpt::microsec_clock::local_time( );
        bpt::time_duration td( local_time - epoch );
        on_write_( lev, td.total_microseconds( ), data );
//        dispatcher_.post( std::bind( &my_logger::do_write, this,
//                                     lev, td.total_microseconds( ),
//                                     data ) );
    }

}}
