
aux_source_directory(. src_dir)

set( exe_name ${INTERNAL_PROJECT_NAME}_client )

add_executable(${exe_name} ${src_dir})

list( APPEND internal
    ${INTERNAL_PROJECT_NAME}_client_core
    ${INTERNAL_PROJECT_NAME}_protocol
    ${INTERNAL_PROJECT_NAME}_common
    VTRC_CLIENT
    VTRC_COMMON
    )

include_directories( ${${INTERNAL_PROJECT_NAME}_protocol_INCLUDE_DIRS}  )
include_directories( ${${INTERNAL_PROJECT_NAME}_client_core_INCLUDE_DIRS}  )

message( "Core " ${${INTERNAL_PROJECT_NAME}_client_core_INCLUDE_DIRS} )

message( "-- Seting up internal dependencies for " ${exe_name} )
foreach( dep ${internal} )

    message( "\tAdd dependence: " ${dep} ${${dep}_LIBRARIES} )

    add_dependencies(       ${exe_name} ${dep}  ${${dep}_LIBRARIES}    )
    add_definitions(        ${${dep}_DEFINITIONS}   )
    target_link_libraries(  ${exe_name} ${${dep}_LIBRARIES} )

endforeach(dep)

if( LUA_FOUND )
    include_directories( ${LUA_INCLUDE_DIR} )
    target_link_libraries( ${exe_name} ${LUA_LIBRARIES} )
    add_dependencies( ${exe_name} lua_lib )
endif( )

list( APPEND dependencies
      Boost
      PROTOBUF
      OPENSSL
    )

message( "-- Seting up dependencies for " ${exe_name} )
foreach( dep ${dependencies} )

    message( "\tAdd dependence: " ${dep} )

    add_dependencies(       ${exe_name} ${dep}      )
    add_definitions(        ${${dep}_DEFINITIONS}   )
    include_directories(    ${${dep}_INCLUDE_DIRS}  )
    target_link_libraries(  ${exe_name} ${${dep}_LIBRARIES} )

endforeach(dep)


if(WIN32)
    target_link_libraries(${exe_name} ws2_32.lib)
endif(WIN32)

set_target_properties(${exe_name}
                      PROPERTIES COMPILE_DEFINITIONS "${VTRC_LINK_TYPE}")

