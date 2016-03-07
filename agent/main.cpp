#include <iostream>

#include <cstdint>
#include <iostream>

#include "application.h"
#include "google/protobuf/stubs/common.h"

template <typename T>
struct type_unique_id {
    typedef T type;
    static std::uintptr_t id( )
    {
        static const char i = '!';
        return reinterpret_cast<std::uintptr_t>(&i);
    }
};

int main( int argc, const char *argv[ ] )
{
    std::cout << type_unique_id<std::string>::id( ) << "\n";
    std::cout << type_unique_id<std::string>::id( ) << "\n";
    std::cout << type_unique_id<int>::id( ) << "\n";
    std::cout << type_unique_id<long>::id( ) << "\n";
    std::cout << type_unique_id<ta::agent::application>::id( ) << "\n";

    return 0;
    try {
        ta::agent::application tapp;
        tapp.run( argc, argv );
    } catch( const std::exception &ex ) {
        std::cerr << "Application failed to run: " << ex.what( ) << "\n";
    }
    google::protobuf::ShutdownProtobufLibrary( );
    return 0;
}
