#include <atomic>
#include <cstdint>

#include "subsys-fs.h"
#include "../application.h"

#include "vtrc-common/vtrc-closure-holder.h"
#include "vtrc-common/vtrc-mutex-typedefs.h"
#include "vtrc-server/vtrc-channels.h"
#include "vtrc-common/vtrc-exception.h"

#include "boost/filesystem.hpp"

#include "protocol/fs.pb.h"
#include "vtrc-common/protocol/vtrc-errors.pb.h"

#include "../files.h"

#define LOG(lev) log_(lev) << "[      fs] "
#define LOGINF   LOG(level::info)
#define LOGDBG   LOG(level::debug)
#define LOGERR   LOG(level::error)
#define LOGWRN   LOG(level::warning)
namespace ta { namespace agent { namespace subsys {

    namespace {

        namespace bfs   = boost::filesystem;
        namespace vcomm = vtrc::common;
        namespace proto = ta::proto;
        namespace verr  = vtrc::rpc::errors;

        const std::string subsys_name( "fs" );

        using level = agent::logger::level;

        typedef std::uint32_t handle_type;

        using path_map     = std::map<handle_type, bfs::path>;
        using iterator_map = std::map<handle_type, bfs::directory_iterator>;

        using file_sptr = std::shared_ptr<agent::file_iface> ;
        using file_wptr = vtrc::weak_ptr<agent::file_iface>;
        using file_map  = std::map<std::uint32_t, file_sptr>;

        class proto_fs_impl: public ta::proto::fs::instance {

            path_map            path_;
            vtrc::shared_mutex  path_lock_;

            iterator_map        iters_;
            vtrc::shared_mutex  iters_lock_;

            std::atomic<handle_type>  handle_;

            inline handle_type next_index( )
            {
                return ++handle_;
            }

            bfs::path path_from_request( const proto::fs::handle_path* request,
                                         handle_type &hdl )
            {
                bfs::path p(request->path( ));

                if( !request->has_hdl( ) || p.is_absolute( ) ) {
                    /// ok. new instance requested
                    p.normalize( );
                    hdl = next_index( );

                } else {

                    /// old path must be used
                    hdl = request->hdl( ).value( );

                    vtrc::shared_lock l( path_lock_ );
                    path_map::const_iterator f( path_.find( hdl ) );

                    if( f == path_.end( ) ) {
                        vcomm::throw_protocol_error( verr::ERR_INVALID_VALUE );
                    }
                    p = f->second;
                    p /= request->path( );
                    p.normalize( );
                }
                return p;
            }

            void open( ::google::protobuf::RpcController* /*controller*/,
                       const ::ta::proto::fs::handle_path* request,
                       ::ta::proto::fs::handle_path* response,
                       ::google::protobuf::Closure* done) override
            {
                vcomm::closure_holder holder(done);

                handle_type hdl;
                bfs::path p(path_from_request( request, hdl ));
                {
                    vtrc::unique_shared_lock l( path_lock_ );
                    path_.insert( std::make_pair( hdl, p ) );
                }
                response->mutable_hdl( )->set_value( hdl );
                response->set_path( p.string( ) );
            }

            void cd(::google::protobuf::RpcController* /*controller*/,
                         const ::ta::proto::fs::handle_path* request,
                         ::ta::proto::fs::handle_path* response,
                         ::google::protobuf::Closure* done) override
            {
                vcomm::closure_holder holder(done);

                vtrc::upgradable_lock ul( path_lock_ );

                handle_type hdl( request->hdl( ).value( ) );

                path_map::iterator f( path_.find( hdl ) );

                if( f == path_.end( ) ) {
                    vcomm::throw_protocol_error( verr::ERR_INVALID_VALUE );
                }

                bfs::path req( request->path( ) );
                bfs::path p;
                if( req.is_absolute( ) ) {
                    p = req;
                } else {
                    p = f->second / request->path( );
                }

                p.normalize( );
                response->set_path( p.string( ) );
                /// set new path
                vtrc::upgrade_to_unique utul( ul );
                f->second = p;
            }

            void pwd(::google::protobuf::RpcController* /*controller*/,
                         const ::ta::proto::fs::handle_path* request,
                         ::ta::proto::fs::handle_path* response,
                         ::google::protobuf::Closure* done) override
            {
                vcomm::closure_holder holder(done);
                handle_type hdl;
                bfs::path p(path_from_request( request, hdl ));
                response->set_path( p.string( ) );
            }

            void exists(::google::protobuf::RpcController* /*controller*/,
                         const ::ta::proto::fs::handle_path* request,
                         ::ta::proto::fs::element_info* response,
                         ::google::protobuf::Closure* done) override
            {
                vcomm::closure_holder holder(done);
                handle_type hdl;

                bfs::path p( path_from_request( request, hdl ) );
                response->set_is_exist( bfs::exists( p ) );
            }

            void file_size(::google::protobuf::RpcController* /*controller*/,
                         const ::ta::proto::fs::handle_path* request,
                         ::ta::proto::fs::file_position* response,
                         ::google::protobuf::Closure* done) override
            {
                vcomm::closure_holder holder(done);

                handle_type hdl;
                bfs::path p( path_from_request( request, hdl ) );

                response->set_position( bfs::file_size( p ) );
            }

            void fill_info( bfs::path const &p,
                            proto::fs::element_info* response )
            {
                bool is_exists = bfs::exists( p );
                response->set_is_exist( is_exists );
                if( is_exists ) {
                    bool is_dir = bfs::is_directory( p );
                    response->set_is_symlink( bfs::is_symlink( p ) );
                    response->set_is_directory( is_dir );
                    response->set_is_empty( true );

                    if( is_dir ) try {
                        response->set_is_empty( bfs::is_empty( p ) );
                    } catch( ... ) { ;;; }

                    response->set_is_regular( bfs::is_regular_file( p ) );
                }
            }

            void info( ::google::protobuf::RpcController* /*controller*/,
                       const ::ta::proto::fs::handle_path* request,
                       ::ta::proto::fs::element_info* response,
                       ::google::protobuf::Closure* done) override
            {
                vcomm::closure_holder holder(done);
                handle_type hdl;
                bfs::path p( path_from_request( request, hdl ) );
                fill_info( p, response );
            }

            void mkdir(::google::protobuf::RpcController* /*controller*/,
                         const ::ta::proto::fs::handle_path* request,
                         ::ta::proto::fs::handle_path* response,
                         ::google::protobuf::Closure* done) override
            {
                vcomm::closure_holder holder(done);
                handle_type hdl;
                bfs::path p( path_from_request( request, hdl ) );
                bfs::create_directories( p );
                response->mutable_hdl( )->set_value( hdl );
            }

            void rename(::google::protobuf::RpcController* /*controller*/,
                         const ::ta::proto::fs::rename_req* request,
                         ::ta::proto::fs::handle_path* response,
                         ::google::protobuf::Closure* done) override
            {
                vcomm::closure_holder holder(done);
                handle_type shdl;
                handle_type dhdl;

                bfs::path s( path_from_request( &request->src( ), shdl ) );
                bfs::path d( path_from_request( &request->dst( ), dhdl ) );

                bfs::rename( s, d );
                response->mutable_hdl( )->set_value( dhdl );

            }

            void del(::google::protobuf::RpcController* /*controller*/,
                         const ::ta::proto::fs::handle_path* request,
                         ::ta::proto::fs::handle_path* response,
                         ::google::protobuf::Closure* done) override
            {
                vcomm::closure_holder holder(done);
                handle_type hdl = 0;
                bfs::path p(path_from_request( request, hdl ));
                bfs::remove( p );
                response->mutable_hdl( )->set_value( hdl );
            }

            void remove_all(::google::protobuf::RpcController* /*controller*/,
                         const ::ta::proto::fs::handle_path* request,
                         ::ta::proto::fs::handle_path* response,
                         ::google::protobuf::Closure* done) override
            {
                vcomm::closure_holder holder(done);
                std::uint32_t hdl = 0;
                bfs::path p(path_from_request( request, hdl ));
                bfs::remove_all( p );
                response->mutable_hdl( )->set_value( hdl );
            }

            void fill_iter_info( const bfs::directory_iterator &iter,
                                 handle_type hdl,
                                 proto::fs::iterator_info* response)
            {
                response->mutable_hdl( )->set_value( hdl );
                response->set_end( iter == bfs::directory_iterator( ));
                if( !response->end( ) ) {
                    std::string path( iter->path( ).string( ) );
                    response->set_path( path );
                    fill_info( iter->path( ), response->mutable_info( ) );
                }
            }

            bfs::directory_iterator &get_iter_unsafe( handle_type hdl )
            {
                iterator_map::iterator f( iters_.find( hdl ) );
                if( f == iters_.end( ) ) {
                    vcomm::throw_protocol_error( verr::ERR_INVALID_VALUE );
                }
                return f->second;
            }

            bfs::directory_iterator &get_iter( handle_type hdl )
            {
                vtrc::shared_lock l( iters_lock_ );
                return get_iter_unsafe( hdl );
            }

            void iter_begin(::google::protobuf::RpcController* /*controller*/,
                         const ::ta::proto::fs::handle_path* request,
                         ::ta::proto::fs::iterator_info* response,
                         ::google::protobuf::Closure* done) override
            {
                vcomm::closure_holder holder(done);

                handle_type hdl;
                bfs::path p(path_from_request( request, hdl ));

                bfs::directory_iterator new_iterator(p);
                handle_type iter_hdl = next_index( );

                vtrc::unique_shared_lock usl( iters_lock_);
                iters_.insert( std::make_pair( iter_hdl, new_iterator ) );
                fill_iter_info( new_iterator, iter_hdl, response );
            }

            void iter_next(::google::protobuf::RpcController* /*controller*/,
                         const ::ta::proto::fs::iterator_info* request,
                         ::ta::proto::fs::iterator_info* response,
                         ::google::protobuf::Closure* done) override
            {
                vcomm::closure_holder holder(done);

                handle_type hdl( request->hdl( ).value( ) );
                vtrc::shared_lock usl( iters_lock_ );

                bfs::directory_iterator &iter( get_iter_unsafe( hdl ) );
                if( iter != bfs::directory_iterator( ) ) {
                    ++iter;
                    fill_iter_info( iter, hdl, response );
                } else {
                    vcomm::throw_protocol_error( verr::ERR_NO_DATA );
                }
            }

            void iter_info(::google::protobuf::RpcController* /*controller*/,
                         const ::ta::proto::fs::iterator_info* request,
                         ::ta::proto::fs::element_info* response,
                         ::google::protobuf::Closure* done) override
            {
                vcomm::closure_holder holder(done);
                handle_type hdl( request->hdl( ).value( ) );
                bfs::directory_iterator iter( get_iter( hdl ) );
                fill_info( iter->path( ), response );
            }

            void iter_clone(::google::protobuf::RpcController* /*controller*/,
                         const ::ta::proto::fs::iterator_info* request,
                         ::ta::proto::fs::iterator_info* response,
                         ::google::protobuf::Closure* done) override
            {
                vcomm::closure_holder holder(done);

                handle_type hdl( request->hdl( ).value( ) );
                bfs::directory_iterator iter(get_iter( hdl ));
                handle_type new_hdl = next_index( );
                fill_iter_info( iter, hdl, response );

                vtrc::unique_shared_lock usl( iters_lock_ );
                iters_.insert( std::make_pair( new_hdl, iter ) );
            }

            void close(::google::protobuf::RpcController*   /*controller*/,
                         const ::ta::proto::handle*         request,
                         ::ta::proto::empty*                /*response*/,
                         ::google::protobuf::Closure* done) override
            {
                vcomm::closure_holder holder(done);

                {
                    vtrc::upgradable_lock ul( path_lock_ );
                    path_map::iterator f( path_.find( request->value( ) ) );
                    if( f != path_.end( ) ) {
                        vtrc::upgrade_to_unique uul( ul );
                        path_.erase( f );
                        return;
                    }
                }

                {
                    vtrc::upgradable_lock ul( iters_lock_ );
                    iterator_map::iterator f( iters_.find( request->value( )));
                    if( f != iters_.end( ) ) {
                        vtrc::upgrade_to_unique uul( ul );
                        iters_.erase( f );
                    }
                }
            }

            void read_file(::google::protobuf::RpcController* /*controller*/,
                         const ::ta::proto::fs::read_file_req* request,
                         ::ta::proto::fs::read_file_res* response,
                         ::google::protobuf::Closure* done) override
            {
                vcomm::closure_holder holder(done);
                size_t len = request->len( ) > 44000 ? 44000 : request->len( );
                if( 0 == len ) {
                    return;
                }

                handle_type dhdl;
                bfs::path p( path_from_request( &request->dst( ), dhdl ) );

                std::vector<char> data(len);

                std::unique_ptr<file_iface> f( file::create(
                                                   p.string( ), "r" ) );
                size_t r = f->read( &data[0], len );
                response->set_data( &data[0], r );
            }

            void write_file(::google::protobuf::RpcController*  /*controller*/,
                         const ::ta::proto::fs::write_file_req* request,
                         ::ta::proto::fs::write_file_res* response,
                         ::google::protobuf::Closure* done) override
            {
                vcomm::closure_holder holder(done);
                size_t len = request->data( ).size( ) > 44000
                           ? 44000
                           : request->data( ).size( );

                handle_type dhdl;
                bfs::path p( path_from_request( &request->dst( ), dhdl ) );

                std::unique_ptr<file_iface> f( file::create( p.string( ),
                                                             "w" ) );
                response->set_len( f->write( request->data( ).c_str( ), len ) );
            }

        public:

            proto_fs_impl( ta::agent::application * /*app*/,
                           vcomm::connection_iface_wptr /*cl*/ )
                :handle_(100)
            { }

            static const std::string &name( )
            {
                return ta::proto::fs::instance::descriptor( )->full_name( );
            }
        };

        class proto_file_impl: public ta::proto::fs::file {

            vcomm::connection_iface_wptr  client_;
            file_map                      files_;
            vtrc::shared_mutex            files_lock_;

            std::atomic<handle_type>      index_;

        public:

            proto_file_impl( ta::agent::application *app,
                             vcomm::connection_iface_wptr &cli)
                :client_(cli)
                ,index_(100)
            { }

            ~proto_file_impl( )
            {
                try {
                    destroy_all( );
                } catch( ... ) { }
            }

            void destroy_all( )
            {

            }

            static const std::string &name( )
            {
                return ta::proto::fs::file::descriptor( )->full_name( );
            }

            inline handle_type next_id( )
            {
                return ++index_;
            }

        private:

            void del_file( handle_type id )
            {
                vtrc::unique_shared_lock lck( files_lock_ );
                files_.erase( id );
            }

            handle_type add_file( file_sptr &f )
            {
                handle_type id = next_id( );
                vtrc::unique_shared_lock lck( files_lock_ );
                files_[id] = f;
                return id;
            }

            file_sptr get_file( handle_type id )
            {
                vtrc::shared_lock lck( files_lock_ );
                file_map::iterator f( files_.find(id) );
                if( f == files_.end( ) ) {
                    vcomm::throw_protocol_error( verr::ERR_BAD_FILE );
                }
                return f->second;
            }

            void open( ::google::protobuf::RpcController* /*controller*/,
                       const ::ta::proto::fs::file_open_req* request,
                       ::ta::proto::handle*                  response,
                       ::google::protobuf::Closure* done ) override
            {
                vcomm::closure_holder holder(done);
                namespace afile = ta::agent::file;

                std::string fmode( request->strmode( ) );

                file_sptr new_file;

                new_file.reset( afile::create( request->path( ), fmode ) );

                response->set_value( add_file( new_file ) );
            }

            void tell(::google::protobuf::RpcController* /*controller*/,
                         const ::ta::proto::handle*      request,
                         ::ta::proto::fs::file_position* response,
                         ::google::protobuf::Closure* done) override
            {
                vcomm::closure_holder holder(done);
                file_sptr f(get_file( request->value( ) ));
                response->set_position( f->tell( ) );
            }

            static
            agent::file_iface::seek_whence value_to_enum( int v )
            {
                switch ( v ) {
                case proto::fs::POS_SEEK_CUR:
                case proto::fs::POS_SEEK_SET:
                case proto::fs::POS_SEEK_END:
                    return static_cast<agent::file_iface::seek_whence>(v);
                }
                return agent::file_iface::F_SEEK_SET;
            }

            void seek(::google::protobuf::RpcController* /*controller*/,
                         const ::ta::proto::fs::file_set_position* request,
                         ::ta::proto::fs::file_position* response,
                         ::google::protobuf::Closure* done) override
            {
                vcomm::closure_holder holder(done);
                file_sptr f(get_file( request->hdl( ).value( ) ));

                response->set_position( f->seek( request->position( ),
                                          value_to_enum(request->whence( )) ) );
            }

            void ioctl(::google::protobuf::RpcController* controller,
                         const ::ta::proto::fs::ioctl_req* request,
                         ::ta::proto::fs::ioctl_res* response,
                         ::google::protobuf::Closure* done) override
            {
                vcomm::closure_holder holder(done);
                file_sptr f(get_file( request->hdl( ).value( ) ));
                f->ioctl( request->code( ),
                          static_cast<unsigned long>( request->parameter( ) ) );

            }

            void ioctl_ptr(::google::protobuf::RpcController* controller,
                         const ::ta::proto::fs::ioctl_req* request,
                         ::ta::proto::fs::ioctl_res* response,
                         ::google::protobuf::Closure* done) override
            {
                vcomm::closure_holder holder(done);
                file_sptr f(get_file( request->hdl( ).value( ) ));

                response->mutable_ptr_param( )->assign( request->ptr_param( ) );
                auto data = request->ptr_param( ).empty( )
                          ? nullptr
                          : &(*response->mutable_ptr_param( ))[0];
                f->ioctl( request->code( ), data );
            }

            void read(::google::protobuf::RpcController* /*controller*/,
                         const ::ta::proto::fs::file_data_block* request,
                         ::ta::proto::fs::file_data_block* response,
                         ::google::protobuf::Closure* done) override
            {
                vcomm::closure_holder holder(done);

                file_sptr f(get_file( request->hdl( ).value( ) ));
                size_t max_block = request->length( );

                vtrc::uint64_t pos = 0;

                if( request->has_cust_pos( ) ) {
                    pos = f->tell( );
                    f->seek( request->cust_pos( ).position( ),
                             value_to_enum(request->cust_pos( ).whence( )) );
                }

                if( max_block > 44000 ) { /// protocol violation
                    max_block = 44000;
                }

                if( max_block == 0 ) { /// nothing to do here
                    return;
                }

                std::vector<char> data(max_block);
                size_t result = f->read( &data[0], max_block );

                if( request->cust_pos( ).set_back( ) ) {
                    f->seek( pos, agent::file_iface::F_SEEK_SET );
                }

                response->set_data( &data[0], result );
            }

            void write(::google::protobuf::RpcController* /*controller*/,
                         const ::ta::proto::fs::file_data_block* request,
                         ::ta::proto::fs::file_data_block* response,
                         ::google::protobuf::Closure* done) override
            {
                vcomm::closure_holder holder(done);
                file_sptr f(get_file( request->hdl( ).value( ) ));

                if( request->data( ).size( ) == 0 ) {
                    response->set_length( 0 );
                } else {

                    vtrc::uint64_t position = 0;

                    if( request->has_cust_pos( ) ) {
                        position = f->tell( );
                        f->seek( request->cust_pos( ).position( ),
                                 value_to_enum(request->cust_pos( ).whence( )));
                    }

                    size_t total = request->data( ).size( );
                    size_t pos = 0;

                    while ( pos != total ) {
                        pos += f->write( &request->data( )[pos], total - pos);
                    }

                    if( request->cust_pos( ).set_back( ) ) {
                        f->seek( position, agent::file_iface::F_SEEK_SET );
                    }

                    response->set_length( total );
                }
            }

            void flush(::google::protobuf::RpcController* /*controller*/,
                         const ::ta::proto::handle*       request,
                         ::ta::proto::empty*              /*response*/,
                         ::google::protobuf::Closure* done) override
            {
                vcomm::closure_holder holder(done);
                file_sptr f(get_file( request->value( ) ));
                f->flush( );
            }

            void close(::google::protobuf::RpcController* /*controller*/,
                         const ::ta::proto::handle*       request,
                         ::ta::proto::empty*              /*response*/,
                         ::google::protobuf::Closure* done) override
            {
                vcomm::closure_holder holder(done);
                vtrc::upgradable_lock lck( files_lock_ );
                file_map::iterator f(files_.find( request->value( ) ));
                if( f != files_.end( ) ) {
                    vtrc::upgrade_to_unique utl(lck);
                    files_.erase( f );
                }
            }

        };

        application::service_wrapper_sptr create_fs_inst(
                                      ta::agent::application *app,
                                      vtrc::common::connection_iface_wptr cl)
        {
            if( app->is_ctrl_connection( cl.lock( ).get( ) ) ) {
                auto inst(vtrc::make_shared<proto_fs_impl>( app, cl ));

                app->get_logger( )( level::debug )
                        << "[     fs] Create FS service.";

                return app->wrap_service( cl, inst );
            } else {
                return application::service_wrapper_sptr( );
            }
        }

        application::service_wrapper_sptr create_file_inst(
                                      ta::agent::application *app,
                                      vtrc::common::connection_iface_wptr cl)
        {
            if( app->is_ctrl_connection( cl.lock( ).get( ) ) ) {
                auto inst(vtrc::make_shared<proto_file_impl>( app, cl ) );

                app->get_logger( )( level::debug )
                        << "[     fs] Create FILE service.";

                return app->wrap_service( cl, inst );
            } else {
                return application::service_wrapper_sptr( );
            }
        }

    }

    struct fs::impl {

        application     *app_;
        agent::logger   &log_;

        impl( application *app )
            :app_(app)
            ,log_(app_->get_logger( ))
        { }

        void reg_creator( const std::string &name,
                          application::service_getter_type func )
        {
            app_->register_service_factory( name, func );
        }

        void unreg_creator( const std::string &name )
        {
            app_->unregister_service_factory( name );
        }

    };

    fs::fs( application *app )
        :impl_(new impl(app))
    { }

    fs::~fs( )
    {
        delete impl_;
    }

    /// static
    fs::shared_type fs::create( application *app )
    {
        shared_type new_inst(new fs(app));
        return new_inst;
    }

    const std::string &fs::name( ) const noexcept
    {
        return subsys_name;
    }

    void fs::init( )
    {

    }

    void fs::start( )
    {
        impl_->reg_creator( proto_fs_impl::name( ), create_fs_inst );
        impl_->reg_creator( proto_file_impl::name( ), create_file_inst );
        impl_->LOGINF << "Started";
    }

    void fs::stop( )
    {
        impl_->unreg_creator( proto_fs_impl::name( ) );
        impl_->unreg_creator( proto_file_impl::name( ) );
        impl_->LOGINF << "Stopped";
    }

}}}

    
