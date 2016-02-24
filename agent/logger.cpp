#include "logger.h"

#include <chrono>
#include <iostream>
#include <thread>

#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/asio/strand.hpp"
#include "boost/algorithm/string.hpp"

namespace ta { namespace agent {

    namespace bpt = boost::posix_time;
    namespace ba  = boost::asio;

    struct logger::impl {
        ba::io_service::strand dispatcher_;
        const char *split_string_;
        impl(ba::io_service &ios, const char *split_string)
            :dispatcher_(ios)
            ,split_string_(split_string)
        { }
    };

    logger::logger( boost::asio::io_service &ios, level lvl,
                    const char *split_string )
        :common::logger( lvl )
        ,impl_(new impl(ios, split_string))
    { }

    logger::~logger( )
    {
        delete impl_;
    }

    void logger::dispatch( std::function<void ( )> call )
    {
        impl_->dispatcher_.post( call );
    }

    void logger::send_data( level lev, const std::string &data )
    {
        //static const bpt::ptime epoch( bpt::ptime::date_type(1970, 1, 1) );

        bpt::ptime local_time = bpt::microsec_clock::local_time( );

        //bpt::time_duration td( local_time - epoch );
        //std::cout << std::hex << std::this_thread::get_id( ) << "!\n";

        impl_->dispatcher_.post( std::bind( &logger::do_write, this,
                                            lev, local_time, data, true ) );
    }

    void logger::send_data_nosplit( level lev, const std::string &data )
    {
        bpt::ptime local_time = bpt::microsec_clock::local_time( );
        impl_->dispatcher_.post( std::bind( &logger::do_write, this,
                                            lev, local_time, data, false ) );
    }


    void logger::do_write( level lvl, const bpt::ptime &tim,
                           std::string const &data , bool split) NOEXCEPT
    {
        logger_data_type all;
        if(split) {
            boost::split( all, data, boost::is_any_of(impl_->split_string_) );
        } else {
            all.push_back( data );
        }
        on_write_( static_cast<int>(lvl), tim, all );
    }

}}
