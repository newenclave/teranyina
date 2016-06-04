#include <iostream>


#include "application.h"
#include "google/protobuf/stubs/common.h"

#include <cstdint>
#include <iostream>

int main( int argc, const char *argv[ ] )
{
    try {
        ta::agent::application tapp;
        tapp.run( argc, argv );
    } catch( const std::exception &ex ) {
        std::cerr << "Application failed to run: " << ex.what( ) << "\n";
    }
    google::protobuf::ShutdownProtobufLibrary( );
    return 0;
}
