#include <iostream>

#include "vtrc-server/vtrc-application.h"
#include "vtrc-server/vtrc-listener-ssl.h"
#include "vtrc-server/vtrc-listener-tcp.h"
#include "boost/asio.hpp"

class ta_app: public vtrc::server::application {
    boost::asio::io_service ios_;
    boost::asio::io_service::work wrk_;
public:
    ta_app( )
        :vtrc::server::application(ios_)
        ,wrk_(ios_)
    { }
    boost::asio::io_service &get_io_service( )
    {
        return ios_;
    }
};

int main( )
{
    try {
        ta_app tapp;
        auto ssl = vtrc::server::listeners::tcp::create( tapp, "0.0.0.0", 12345 );
        //auto ssl = vtrc::server::listeners::tcp_ssl::create( tapp, "0.0.0.0", 12345 );
        ssl->start( );
        tapp.get_io_service( ).run( );
    } catch( const std::exception &ex ) {
        std::cerr << "failed to start: " << ex.what( );
    }
    return 0;
}
