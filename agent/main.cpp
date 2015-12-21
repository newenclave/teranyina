#include <iostream>

#include "application.h"

int main( int argc, const char *argv[ ] )
{
    try {
        ta::agent::application tapp;
        tapp.run( argc, argv );
    } catch( const std::exception &ex ) {
        std::cerr << "Application failed to run: " << ex.what( );
    }
    return 0;
}
