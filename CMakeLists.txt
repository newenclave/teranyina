
if(WIN32)
    set(
        CMAKE_USER_MAKE_RULES_OVERRIDE
        "${CMAKE_CURRENT_SOURCE_DIR}/msvc_override.cmake"
    )
endif(WIN32)

cmake_minimum_required(VERSION 2.8)

set( INTERNAL_PROJECT_NAME teranyina )

set( TA_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include")

############################ VTRC library ############################
find_file( VTRC_IMPORT_CMAKE
           NAMES
               "vtrc-import.cmake"
           PATHS
              #"${CMAKE_CURRENT_SOURCE_DIR}/../vtrc/"
               "${CMAKE_CURRENT_SOURCE_DIR}/vtrc/" )

if( VTRC_IMPORT )
    set( VTRC_IMPORT_CMAKE ${VTRC_IMPORT} )
endif( )

if( NOT VTRC_IMPORT_CMAKE )
    message( "INITING submodule VTRC" )
    execute_process( COMMAND "git" "submodule" "init"
                     WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
    execute_process( COMMAND "git" "submodule" "update" "vtrc"
                     WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
    execute_process( COMMAND "git" "pull"
                     WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/vtrc")
endif( NOT VTRC_IMPORT_CMAKE )

add_subdirectory( vtrc )
set( VTRC_IMPORT_CMAKE "${CMAKE_CURRENT_SOURCE_DIR}/vtrc/vtrc-import.cmake")

include( ${VTRC_IMPORT_CMAKE} )
######################################################################

include( cxx11_check.cmake )

check_cxx11( CXX11_ENABLED )

if(NOT CXX11_ENABLED)
    message( FATAL_ERROR "Compiler has no C++11 support" )
endif(NOT CXX11_ENABLED)

if(NOT VTRC_OPENSSL_ENABLED)
    message( FATAL_ERROR "OpenSSL is require" )
endif(NOT VTRC_OPENSSL_ENABLED)

message( "static link: " ${VTRC_LINK_TYPE_STATIC} )

project( ${INTERNAL_PROJECT_NAME} )

if( ${VTRC_LINK_TYPE} MATCHES ${VTRC_LINK_TYPE_STATIC} )
    set(Boost_USE_STATIC_LIBS     ON)
    set(Boost_USE_STATIC_RUNTIME  ON)
endif( )

set(Boost_USE_MULTITHREADED   ON)

find_package( OpenSSL REQUIRED )
set( OPENSSL_INCLUDE_DIRS ${OPENSSL_INCLUDE_DIR} )

if(MSVC)

    find_package( Boost 1.50 COMPONENTS
                    system
                    thread
                    program_options
                    filesystem
                    date_time
                    chrono
                    regex
                  REQUIRED)
else( MSVC )

    find_package( Boost 1.50 COMPONENTS
                    system
                    thread
                    program_options
                    filesystem
                    date_time
                    regex
                    #context
                  REQUIRED)
endif( MSVC )

get_cmd_cxx11( VTRC_CXX_CMD )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${VTRC_CXX_CMD}")

find_package( Protobuf REQUIRED)

include_directories( ${VTRC_INCLUDE_DIRS} )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )
include_directories( ${TA_INCLUDE_DIRS} )

set( src_dirs )
set( src )

set( LUA_FOUND 0 )

if( NOT LUA_SRC )
    find_file( LUA_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/lua/src/lua.h" )
    if( NOT LUA_HEADER )
        execute_process( COMMAND "git" "submodule" "init"
                         WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
        execute_process( COMMAND "git" "submodule" "update" "lua"
                         WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
    endif( NOT LUA_HEADER )
    set( LUA_SRC "${CMAKE_CURRENT_SOURCE_DIR}/lua/src/" )
endif( NOT LUA_SRC )

if( LUA_SRC )

    add_subdirectory( lua-build )
    set( LUA_FOUND 1 )
    ADD_DEFINITIONS(-DLUA_FOUND=1)
    ADD_DEFINITIONS(-DLUA_WRAPPER_TOP_NAMESPACE=ta)

else( )
    message( FATAL_ERROR "Lua was not found." )
endif( )

if(MSVC)
    # Force to always compile with W4
    if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
        string( REGEX REPLACE "/W[0-4]" "/W4"
                CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    else( )
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    endif( )
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX )
    # Update if necessary

    set(CMAKE_CXX_FLAGS
        "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic")

elseif(  CMAKE_CXX_COMPILER_ID STREQUAL "Clang" )

    set(CMAKE_CXX_FLAGS
        "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic")

endif( )

#if( STATIC_GLIBC )
#    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -static-libgcc")
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
#    ADD_DEFINITIONS(-static-libgcc)
#    ADD_DEFINITIONS(-static)
#    SET( CMAKE_CXX_LINK_EXECUTABLE
#         "${CMAKE_CXX_LINK_EXECUTABLE} -static-libgcc -static-libstdc++")
#endif( STATIC_GLIBC )

configure_file (  "${PROJECT_SOURCE_DIR}/teranyina-config.h.in"
                  "${TA_INCLUDE_DIRS}/teranyina-config.h" )

file( COPY        "${TA_INCLUDE_DIRS}/teranyina-config.h"
      DESTINATION "${CMAKE_CURRENT_BINARY_DIR}" )

add_subdirectory( common )
add_subdirectory( protocol )
add_subdirectory( client-core )
add_subdirectory( ctrl-client )
add_subdirectory( agent )

if(LUA_FOUND)
#    add_subdirectory( lua-client )
endif(LUA_FOUND)

