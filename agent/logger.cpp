#include "logger.h"

#include <chrono>
#include <iostream>
#include <thread>

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
        static const bpt::ptime epoch( bpt::ptime::date_type(1970, 1, 1) );

        bpt::ptime local_time = bpt::microsec_clock::local_time( );
        //bpt::time_duration td( local_time - epoch );

        //std::cout << std::hex << std::this_thread::get_id( ) << "!\n";

        impl_->dispatcher_.post( std::bind( &logger::do_write, this,
                                            lev, local_time, data ) );
    }

    void logger::do_write( level lvl, const bpt::ptime &tim,
                           std::string const &data ) noexcept
    {
        on_write_( lvl, tim, data );
    }

}}
