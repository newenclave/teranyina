#include "logger.h"

#include <chrono>
#include <iostream>

#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/asio/strand.hpp"

namespace ta { namespace agent {

    namespace bpt = boost::posix_time;
    namespace ba  = boost::asio;

    struct logger::impl {
        ba::io_service::strand dispatcher_;
        impl(ba::io_service &ios)
            :dispatcher_(ios)
        { }
    };

    logger::logger( boost::asio::io_service &ios, level lvl )
        :common::logger( lvl )
        ,impl_(new impl(ios))
    { }

    logger::~logger( )
    {
        delete impl_;
    }

    void logger::send_data( level lev, const std::string &data )
    {
        static const bpt::ptime epoch(bpt::ptime::date_type(1970, 1, 1));

        bpt::ptime local_time = bpt::microsec_clock::local_time( );
        bpt::time_duration td( local_time - epoch );

        impl_->dispatcher_.post( std::bind( &logger::do_write, this,
                                            lev, td.total_microseconds( ),
                                            data ) );
    }

    void logger::do_write( level lvl, std::uint64_t microsec,
                           std::string const &data )
    {
        try {
            on_write_( lvl, microsec, data );
        } catch( const std::exception &ex ) {
            std::cerr << "agent::logger do_write exception: "
                      << ex.what( ) << "\n";
        } catch( ... ) {
            std::cerr << "agent::logger do_write exception: ... \n";
        }
    }

}}
