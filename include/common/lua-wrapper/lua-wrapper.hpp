#ifndef LUA_WRAPPER_HPP
#define LUA_WRAPPER_HPP

#include <stdexcept>
#include <list>

#include <stdlib.h>

extern "C" {
#include "lualib.h"
#include "lauxlib.h"
#include "lua.h"
}

#if( LUA_VERSION_NUM < 503 )
#error "Lua version is too old; Use 5.3 or higher"
#endif

#include "lua-type-wrapper.hpp"
#include "lua-objects.hpp"

#ifdef LUA_WRAPPER_TOP_NAMESPACE

namespace LUA_WRAPPER_TOP_NAMESPACE {

#endif

namespace lua {

    class state {
        lua_State *vm_;
        bool       own_;

        static void *def_alloc( void * /*ud*/, void *ptr,
                                size_t old_size, size_t new_size )
        {
            void *tmp = NULL;

            if ( old_size && new_size && ptr ) {
                if ( old_size < new_size ) {
                    tmp = realloc ( ptr, new_size );
                } else {
                    tmp = ptr;
                }
            } else if( new_size ) {
                tmp = malloc( new_size );
            } else if( new_size == 0 ) {
                free( ptr );
                tmp = NULL;
            }
            return tmp;
        }

        struct object_wrapper {
            const objects::base *obj_;
            object_wrapper( const objects::base *o )
                :obj_(o)
            { }
        };

        void push( object_wrapper value )
        {
            value.obj_->push( vm_ );
        }

    public:

        enum state_owning {
             NOT_OWN_STATE = 0
            ,OWN_STATE     = 1
        };

        state( lua_State *vm, state_owning os = NOT_OWN_STATE )
            :vm_(vm)
            ,own_(os == OWN_STATE)
        { }

        state( )
            :vm_(lua_newstate( def_alloc, 0 ))
            ,own_(true)
        { }

        ~state( )
        {
            if( own_ && vm_ ) {
                lua_close( vm_ );
            }
        }

        void openlibs( )
        {
            luaL_openlibs( vm_ );
        }

        int openlib( const char *libname, lua_CFunction func, int results = 1 )
        {
#if( LUA_VERSION_NUM < 502 )
            push( func );
            lua_call( vm_, 0, 0 );
#else
            push( func );
            lua_call( vm_, 0, results );
            if( results > 0 ) {
                lua_setglobal( vm_, libname );
            }
#endif
            return 1;

        }

        int openlib( const char *libname )
        {
            static const struct {
                std::string     name;
                lua_CFunction   func;
                int             results;
            } libs[ ] = {
                 { LUA_TABLIBNAME,  &luaopen_table,   1 }
                ,{ LUA_IOLIBNAME,   &luaopen_io,      1 }
                ,{ LUA_OSLIBNAME,   &luaopen_os,      1 }
                ,{ LUA_STRLIBNAME,  &luaopen_string,  1 }
                ,{ LUA_MATHLIBNAME, &luaopen_math,    1 }
                ,{ LUA_DBLIBNAME,   &luaopen_debug,   1 }
                ,{ LUA_LOADLIBNAME, &luaopen_package, 1 }
                ,{ LUA_UTF8LIBNAME, &luaopen_utf8,    1 }
                ,{ "base",          &luaopen_base,    0 }
            };

            const size_t libs_count = sizeof( libs ) / sizeof( libs[0] );

            for( size_t i=0; i<libs_count; ++i ) {
                if( 0 == libs[i].name.compare( libname ) ) {                    
                    return openlib( libname, libs[i].func, libs[i].results );
                }
            }
            return 0;
        }

        lua_State *get_state( )
        {
            return vm_;
        }

        const lua_State *get_state( ) const
        {
            return vm_;
        }

        void clean_stack( )
        {
            pop( get_top( ) );
        }

        void pop( )
        {
            lua_pop( vm_, 1 );
        }

        void pop( int n )
        {
            lua_pop( vm_, n );
        }

        std::string pop_error( )
        {
            const char *str = lua_tostring(vm_, -1);
            std::string res( str ? str : "Unknown error" );
            pop( );
            return res;
        }

        void register_call( const char *name, lua_CFunction fn )
        {
            lua_register( vm_, name, fn );
        }

        std::string error( )
        {
            std::string res( lua_tostring(vm_, -1) );
            return res;
        }

        void check_call_error( int res )
        {
            if( 0 != res ) {
                throw std::runtime_error( pop_error( ) );
            }
        }

        void push_value( int idx = -1 )
        {
            lua_pushvalue( vm_, idx );
        }

        void push( )
        {
            lua_pushnil( vm_ );
        }

        void push( bool value )
        {
            lua_pushboolean( vm_, value ? 1 : 0 );
        }

        void push( const char* value )
        {
            lua_pushstring( vm_, value );
        }

        void push( const char* value, size_t len )
        {
            lua_pushlstring( vm_, value, len );
        }

        void push( const std::string& value )
        {
            lua_pushlstring( vm_, value.c_str( ), value.size( ) );
        }

        void push( lua_CFunction value )
        {
            lua_pushcfunction( vm_, value );
        }

        template<typename T>
        void push( T * value )
        {
            lua_pushlightuserdata( vm_, reinterpret_cast<void *>( value ) );
        }

        template<typename T>
        void push( T value )
        {
            lua_pushinteger( vm_, static_cast<T>( value ) );
        }

        template<typename T>
        void push_num( T value )
        {
            lua_pushnumber( vm_, static_cast<T>( value ) );
        }

        template <typename ErrT>
        int push_nil_error( ErrT err, int def_params = 1 )
        {
            int res = def_params;
            while ( def_params-- ) {
                push( );
            }
            push( err );
            return res + 1;
        }

        int get_type( int id = -1 )
        {
            return lua_type( vm_, id );
        }

        int get_top( )
        {
            return lua_gettop( vm_ );
        }

        bool none_or_nil( int id = -1 ) const
        {
            return lua_isnoneornil( vm_, id );
        }
        template<typename T>
        T get( int id = -1 )
        {
            typedef types::id_traits<T> traits;
            if( !traits::check( vm_, id ) ) {
                throw std::runtime_error( std::string("bad type '")
                        + types::id_to_string( traits::type_index )
                        + std::string("'. lua type is '")
                        + types::id_to_string( get_type( id ) )
                        + std::string("'") );
            }
            return traits::get( vm_, id );
        }

        template<typename T>
        T get_opt( int id = -1, const T& def = T( ) )
        {
            typedef types::id_traits<T> traits;
            if( id > get_top( ) || !traits::check( vm_, id ) ) {
                return def;
            }
            return traits::get( vm_, id );
        }

        template<typename T>
        T get_field( const char *key, int id = -1 )
        {
            T p = T( );
            lua_getfield( vm_, id, key );
            if( !none_or_nil(  ) ) {
                try {
                    p = get<T>( );
                } catch( ... ) {
                    pop( );
                    throw;
                }
            }
            pop( );
            return p;
        }

        objects::base_sptr get_table( int idx = -1, unsigned flags = 0 )
        {
            lua_pushvalue( vm_, idx );
            lua_pushnil( vm_ );

            objects::table_sptr new_table( objects::new_table( ) );

            while ( lua_next( vm_, -2 ) ) {
                lua_pushvalue( vm_, -2 );
                objects::base_sptr first = get_type( -1 ) == LUA_TTABLE
                    ? objects::base_sptr( new objects::reference( vm_, -1 ) )
                    : get_object( -1, flags );

                objects::base_sptr second = get_type( -2 ) == LUA_TTABLE
                    ? objects::base_sptr( new objects::reference( vm_, -2 ) )
                    : get_object( -2, flags );

                objects::pair_sptr np( objects::new_pair( first, second ) );

                new_table->push_back( np );
                lua_pop( vm_, 2 );
            }

            lua_pop( vm_, 1 );
            return new_table;
        }

        /// bad do not use this
        objects::base_sptr get_table0( int idx = -1, unsigned flags = 0 )
        {
            lua_pushvalue( vm_, idx );
            lua_pushnil( vm_ );

            objects::table_sptr new_table( objects::new_table( ) );

            while ( lua_next( vm_, -2 ) ) {
                lua_pushvalue( vm_, -2 );
                objects::pair_sptr new_pair
                        ( objects::new_pair( get_object( -1, flags ),
                                             get_object( -2, flags ) ) );
                new_table->push_back( new_pair );
                lua_pop( vm_, 2 );
            }

            lua_pop( vm_, 1 );
            return new_table;
        }

        objects::base_sptr get_object( int idx = -1, unsigned flags = 0 )
        {

            typedef objects::base_sptr base_sptr;

            int t = lua_type( vm_, idx );
            switch( t ) {
            case LUA_TBOOLEAN:
                return base_sptr(
                    new objects::boolean( !!lua_toboolean( vm_, idx ) ));
            case LUA_TLIGHTUSERDATA:
            case LUA_TUSERDATA:
                return base_sptr(
                    new objects::light_userdata( lua_touserdata( vm_, idx ) ));
            case LUA_TNUMBER:
                return flags
                  ? base_sptr(new objects::integer( lua_tointeger( vm_, idx ) ))
                  : base_sptr(new objects::number( lua_tonumber( vm_, idx ) )) ;
            case LUA_TSTRING: {
                    size_t length = 0;
                    const char *ptr = lua_tolstring( vm_, idx, &length );
                    return base_sptr(new objects::string( ptr, length ));
                }
            case LUA_TFUNCTION: {
                return base_sptr(new objects::reference( vm_, idx ));
            }
            case LUA_TTABLE:
                return get_table( idx, flags );
            case LUA_TTHREAD:
                return base_sptr(
                    new objects::thread( vm_, lua_tothread( vm_, idx ) ));

        //    case LUA_TUSERDATA:
        //        return "userdata";
            }
            return base_sptr(new objects::nil);
        }

        objects::base_sptr ref_to_object( const objects::base *o,
                                          unsigned flags = 0)
        {
            objects::base_sptr res;
            o->push( vm_ );
            res = get_object( -1, flags );
            pop( );
            return res;
        }

        /*
         * struc metatable_trait {
         *      static const char *name( ); // metatable name
         *      static const struct luaL_Reg *table( ) // table w.calls
         * };
         *
         */
        template <typename T>
        static void register_metatable( lua_State *L )
        {
            static const luaL_Reg empty = { NULL, NULL };

            bool tostr_found = false;
            bool gc_found    = false;

            std::vector<luaL_Reg> call_table;

            const luaL_Reg *p = T::table( );

            while( p && p->name ) {

                const std::string name(p->name);

                if( name == "__tostring" ) {
                    tostr_found = true;
                } else if( name == "__gc" ) {
                    gc_found = true;
                }

                call_table.push_back( *p );
                ++p;
            }

            if( !tostr_found ) {
                luaL_Reg tostr = { "__tostring",
                                   &state::lcall_default_tostring<T> };
                call_table.push_back( tostr );
            }

            if( !gc_found ) {
                luaL_Reg tostr = { "__gc", &state::lcall_default_gc<T> };
                call_table.push_back( tostr );
            }

            call_table.push_back( empty );

            objects::metatable mt( T::name( ), &call_table[0] );
            mt.push( L );
        }

        template <typename T>
        void register_metatable( )
        {
            register_metatable<T>( vm_ );
        }

        template <typename T>
        static T *create_metatable( lua_State *L )
        {
            void *ud = lua_newuserdata( L, sizeof(T) );
            if( ud ) {
                T *inst = new (ud) T;
                luaL_getmetatable( L, T::name( ) );
                lua_setmetatable(L, -2);
                return inst;
            }
            return NULL;
        }

        template <typename T>
        static int create_metatable_call( lua_State *L )
        {
            void *ud = lua_newuserdata( L, sizeof(T) );
            if( ud ) {
                new (ud) T;
                luaL_getmetatable( L, T::name( ) );
                lua_setmetatable( L, -2 );
                return 1;
            }
            return 0;
        }

        template <typename T>
        T *create_metatable( )
        {
            return create_metatable<T>( vm_ );
        }

        /// get metatable. returns null if failed
        template <typename T>
        static T *test_metatable( lua_State *L, int id = -1 )
        {
            return lcall_get_instance<T>( L, id );
        }

        template <typename T>
        T *test_metatable( int id = -1 )
        {
            return lcall_get_instance<T>( vm_, id );
        }

        /// check and get metatable.
        /// raises error if failed
        template <typename T>
        static T *check_metatable( lua_State *L, int id = -1 )
        {
            void *ud = luaL_checkudata( L, id, T::name( ) );
            return static_cast<T *>(ud);
        }

        template <typename T>
        T *check_metatable( int id = -1 )
        {
            return check_metatable<T>( vm_, id );
        }

    private:

        template <typename T>
        static T *lcall_get_instance( lua_State *L, int id )
        {
            void *ud = luaL_testudata( L, id, T::name( ) );
            return static_cast<T *>(ud);
        }

        template <typename T>
        static int lcall_default_tostring( lua_State *L )
        {
            T *inst = lcall_get_instance<T>( L, 1 );
            std::ostringstream oss;
            oss << T::name( ) << "@" << std::hex << inst;
            lua_pushstring( L, oss.str( ).c_str( ) );
            return 1;
        }

        template <typename T>
        static int lcall_default_gc( lua_State *L )
        {
            T *inst = lcall_get_instance<T>( L, 1 );
            if( inst ) {
                inst->~T( );
            }
            return 0;
        }

    private:

        void get_global( const char *val )
        {
            lua_getglobal( vm_, val );
        }

        void set_global( const char *val )
        {
            lua_setglobal( vm_, val );
        }

        void set_table( int id = -3 )
        {
            lua_settable( vm_, id );
        }

        static size_t path_root( const char *path )
        {
            const char *p = path;
            for( ; *p && (*p != '.'); ++p );
            return p - path;
        }

        static const char *path_leaf( const char *path )
        {
            const char *p  = path;
            const char *sp = path;
            for( ; *p; ++p ) {
                if( *p == '.' ) {
                    sp = p;
                }
            }
            return sp;
        }

        template <typename T>
        void create_or_push( const char *path, T value )
        {
            if( *path ) {

                std::string p( path, path_root( path ) );
                const char *tail = path + p.size( );

                lua_newtable( vm_ );
                push( p.c_str( ) );
                create_or_push( !*tail ? "" : tail + 1, value );
                set_table( -3 );
            } else {
                push( value );
            }
        }

        template <typename T>
        void set_to_stack( const char *path, T value )
        {
            std::string p( path, path_root( path ) );
            const char *tail = path + p.size( );

            if( *tail ) {

                lua_getfield( vm_, -1, p.c_str( ) );

                if( !lua_istable( vm_, -1 ) ) {
                    pop( 1 );
                    push( p.c_str( ) );
                    create_or_push( tail + 1, value );
                    set_table( -3 );
                } else {
                    set_to_stack( tail + 1, value );
                    pop( 1 );
                }
            } else {
                push( p.c_str( ) );
                push( value );
                set_table( -3 );
            }
        }

    public:

        template <typename T>
        void set( const char *path, T value )
        {
            std::string p( path, path_root( path ) );
            const char *tail = path + p.size( );

            get_global( p.c_str( ) );

            if( !lua_istable( vm_, -1 ) ) {
                pop( 1 );
                if( !*tail ) {
                    push( value );
                } else {
                    create_or_push( tail + 1, value );
                }
            } else {
                if( !*tail ) {
                    pop( 1 );
                    push( value );
                } else {
                    set_to_stack( tail + 1, value );
                }
            }
            set_global( p.c_str( ) );
        }

        void set( const char *path )
        {
            static const objects::nil nil_value;
            set_object( path, &nil_value );
        }

        void set_object( const char *path, const objects::base *obj )
        {
            set<object_wrapper>( path, object_wrapper( obj ) );
        }

        int get_table( const char *path )
        {
            int level = 1;
            std::string r(path, path_root( path ));
            const char *tail = path + r.size( );

            lua_getglobal( vm_, r.c_str( ) );

            while( 1 ) {
                if( !lua_istable( vm_, -1 ) ) {
                    lua_pop( vm_, level );
                    level = 0;
                    break;
                } else {
                    if( *tail ) {
                        tail++;
                        level++;
                        r.assign( tail, path_root( tail ) );
                        tail = tail + r.size( );
                        lua_getfield( vm_, -1, r.c_str( ) );
                    } else {
                        break;
                    }
                }
            }
            return level;
        }

        template <typename T>
        T get( const char *path )
        {
            const char *pl = path_leaf( path );
            if( pl == path ) {
                get_global( pl );
                T val = T( );
                if( !none_or_nil(  ) ) try {
                    val = get<T>( );
                    pop( );
                } catch( ... ) {
                    pop( );
                    throw;
                }
                return val;
            } else {
                std::string tpath( path, (pl - path) );
                int level = get_table( tpath.c_str( ) );
                if( level ) {
                    T val = T( );
                    try {
                        val = get_field<T>( pl + 1 );
                        pop( level );
                    } catch( ... ) {
                        pop( level );
                        throw;
                    }
                    return val;
                }
            }
            return T( );
        }

        objects::base_sptr get_ref( int idx = -1 ) const
        {
            return std::make_shared<objects::reference>( vm_, idx );
        }

        bool exists( const char *path )
        {
            const char *pl = path_leaf( path );
            bool res = false;
            if( pl == path ) {
                get_global( pl );
                if( !none_or_nil(  ) ) {
                    res = true;
                    pop( );
                }
            } else {
                std::string tpath( path, (pl - path) );
                int level = get_table( tpath.c_str( ) );
                if( level ) {
                    lua_getfield( vm_, -1, pl + 1 );
                    if( !none_or_nil(  ) ) {
                        res = true;
                        pop( );
                    }
                    pop( level );
                }
            }
            return res;
        }

        int exec_function( const char* func )
        {
            lua_getglobal( vm_, func );
            int rc = lua_pcall( vm_, 0, LUA_MULTRET, 0 );
            return rc;
        }

        int exec_function( const char* func, const objects::base &bo )
        {
            lua_getglobal( vm_, func );
            bo.push( vm_ );
            int rc = lua_pcall( vm_, 1, LUA_MULTRET, 0 );
            return rc;
        }

        void push_object_list( const std::vector<objects::base_sptr> &bo )
        {
            typedef std::vector<objects::base_sptr>::const_iterator citr;
            for( citr b(bo.begin( )), e(bo.end( )); b!=e; ++b ) {
                (*b)->push( vm_ );
            }
        }

        void set_value( const char *path, int idx = -1 )
        {
            //// crutch ... WILL FIX IT LATER
            set( path, 0 );
            ////////////////////

            const char *pl = path_leaf( path );

            if( pl == path ) {

                push_value( idx );
                set_global( pl );

            } else {

                std::string tpath( path, (pl - path) );

                int level = get_table( tpath.c_str( ) ); // set table level

                if( level ) {

                    lua_getfield( vm_, -1, pl + 1 );
                    pop( );             // old value
                    push( pl + 1 );     // name
                    push_value( idx );  // value by index
                    set_table( -3 );
                    pop( level );       // clean table level
                }

            }
        }

        lua_State *create_thread( const char *path )
        {
            lua_State *res = lua_newthread( vm_ ); // push new value 'state'
            int index = get_top( );                // table index in parameters
            set_value( path, index );              // copy here
            pop( 1 );                              // pop state from stack
            return res;
        }

        int exec_function( const char* func,
                           const std::vector<objects::base_sptr> &bo )
        {
            lua_getglobal( vm_, func );
            push_object_list( bo );
            int rc = lua_pcall( vm_, bo.size( ), LUA_MULTRET, 0 );
            return rc;
        }

        template <typename P0>
        int exec_function( const char* func, P0 p0 )
        {

            lua_getglobal( vm_, func );
            push( p0 );
            int rc = lua_pcall( vm_, 1, LUA_MULTRET, 0 );
            return rc;
        }

        template <typename P0>
        int exec_function( const char* func, P0 p0,
                           const std::vector<objects::base_sptr> &bo )
        {

            lua_getglobal( vm_, func );
            push( p0 );
            push_object_list( bo );
            int rc = lua_pcall( vm_, 1 + bo.size( ), LUA_MULTRET, 0 );
            return rc;
        }

        template <typename P0, typename P1>
        int exec_function( const char* func, P0 p0, P1 p1 )
        {
            lua_getglobal( vm_, func );
            push( p0 );
            push( p1 );
            int rc = lua_pcall( vm_, 2, LUA_MULTRET, 0 );
            return rc;
        }

        template <typename P0, typename P1>
        int exec_function( const char* func, P0 p0, P1 p1,
                           const std::vector<objects::base_sptr> &bo )
        {
            lua_getglobal( vm_, func );
            push( p0 );
            push( p1 );
            push_object_list( bo );
            int rc = lua_pcall( vm_, 2 + bo.size( ), LUA_MULTRET, 0 );
            return rc;
        }

        int load_file( const char *path )
        {
            int res = luaL_loadfile( vm_, path );
            if( 0 == res ) {
                res = lua_pcall( vm_, 0, LUA_MULTRET, 0);
            }
            return res;
        }

        int load_buffer( const char *buf, size_t length,
                         const char *name = NULL )
        {
            int res = luaL_loadbuffer ( vm_, buf, length, name ? name : "" );
            if( LUA_OK == res ) {
                return lua_pcall( vm_, 0, LUA_MULTRET, 0);
            } else {
                std::string err = pop_error( );
                push( );
                push( err );
                return 2;
            }
        }
    };
    typedef std::shared_ptr<state> state_sptr;

    struct path_element_info{

        std::string name_;
        int         type_;
        int         res_type_;

        path_element_info( const std::string &name )
            :name_(name)
            ,type_(objects::base::TYPE_NONE)
            ,res_type_(objects::base::TYPE_NONE)
        { }

        path_element_info( )
            :type_(objects::base::TYPE_NONE)
            ,res_type_(objects::base::TYPE_NONE)
        { }

        void push_back( std::string::value_type c )
        {
            name_.push_back( c );
        }

        void clear( )
        {
            name_.clear( );
            type_ = objects::base::TYPE_NONE;
        }

        bool empty( ) const
        {
            return name_.empty( );
        }

        bool check( const std::string &n, int t ) const
        {
            return (name_ == n) &&
                 ( (type_ == objects::base::TYPE_NONE)
                           ? true
                           : type_ == t );
        }

        bool check_res_type( int t ) const
        {
            return ( (res_type_ == objects::base::TYPE_NONE)
                           ? true
                           : type_ == t );
        }
    };

    typedef std::list<path_element_info> path_element_info_list;

    inline void split_path( const char *str, path_element_info_list &res )
    {
        path_element_info_list tmp;

        path_element_info next;

        next.name_.reserve( 16 );

        for( ; *str; ++str ) {

            switch( *str ) {
            case '\'':
                ++str;
                next.type_ = objects::base::TYPE_STRING;
                while( *str ) {
                    if( *str == '\\' ) {
                        ++str;
                    } else if( *str == '\'' ) {
                        break;
                    }
                    next.push_back( *str++ );
                }
                break;
            case '.':
                tmp.push_back( next );
                next.clear( );
                break;
            case '\\':
                ++str;
                if( !*str ) {
                    break;
                }
            default:
                next.push_back( *str );
                break;
            }
        }
        if( !next.empty( ) ) {
            tmp.push_back( next );
        }
        res.swap( tmp );
    }

    inline objects::base_sptr object_by_path( lua_State *L,
                                              const objects::base *o,
                                              const char *str )
    {
        typedef path_element_info_list::iterator iter;

        objects::base_sptr result;

        path_element_info_list res;
        split_path( str, res );

        size_t len = res.size( );

        lua::state ls(L);
        objects::base_sptr tmp;

        for( iter b(res.begin( )), e(res.end( )); b != e; ++b ) {

            if( objects::base::is_reference( o ) ) {
                tmp = ls.ref_to_object( o );
                o = tmp.get( );
            }

            if( o->type_id( ) != objects::base::TYPE_TABLE ) {
                break;
            }

            bool found = false;

            for( size_t i=0; i<o->count( ); ++i ) {
                const objects::base *next = o->at( i );
                const objects::base *name = next->at( 0 );
                if( b->check( name->str( ), name->type_id( ) ) ) {
                    o = next->at( 1 );
                    found = true;
                    break;
                }
            }

            if( !found ) {
                break;
            }

            if( 0 == --len ) {
                if( objects::base::is_reference( o ) ) {
                    result = ls.ref_to_object( o, 1 );
                } else {
                    result.reset( o->clone( ) );
                }
            }
        }
        return result;
    }

}

#ifdef LUA_WRAPPER_TOP_NAMESPACE

} // LUA_WRAPPER_TOP_NAMESPACE

#endif


#endif // LUAWRAPPER_HPP
