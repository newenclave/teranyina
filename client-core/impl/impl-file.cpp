
#include "client-core/iface/IFile.h"

#include "protocol/fs.pb.h"

#include "client-core/ta-client.h"

#include "vtrc-common/vtrc-stub-wrapper.h"
#include "vtrc-common/vtrc-rpc-channel.h"

#include "vtrc-bind.h"

namespace ta { namespace client { namespace interfaces {

    namespace {

        namespace fproto = proto::fs;
        namespace vcomm  = vtrc::common;
        typedef vcomm::rpc_channel* channel_ptr;

        typedef vcomm::rpc_channel                            channel_type;
        typedef fproto::file::Stub                            stub_type;
        typedef vcomm::stub_wrapper<stub_type, channel_type>  client_type;

        const unsigned nw_flag  = vcomm::rpc_channel::DISABLE_WAIT;

        proto::handle open_file( client_type &cl, const std::string &path,
                                 const std::string &mode, bool as_device )
        {
            fproto::file_open_req req;
            proto::handle         res;
            req.set_path( path );
            req.set_strmode( mode );
            req.set_as_device( as_device );
            cl.call( &stub_type::open, &req, &res );

            return res;
        }

        struct file_impl: public file::iface {

            core                  &core_;
            mutable client_type    client_;
            proto::handle          hdl_;

            file_impl( core &ccore,
                       const std::string &path,
                       const std::string &mode,
                       bool as_device )
                :core_(ccore)
                ,client_(core_.get_client( ).create_channel( ), true)
                ,hdl_(open_file(client_, path, mode, as_device ))
            { }

            file_impl( core &ccore,
                       vtrc::common::rpc_channel *chan,
                       const std::string &path, const std::string &mode,
                       bool as_device )
                :core_(ccore)
                ,client_(chan, true)
                ,hdl_(open_file(client_, path, mode, as_device ))
            { }

            ~file_impl( )
            {
                try {
                    client_.channel( )->set_flags( nw_flag );
                    close_impl( );
                } catch( ... ) {  }
            }

            vtrc::common::rpc_channel *channel( ) override
            {
                return client_.channel( );
            }

            const vtrc::common::rpc_channel *channel( ) const override
            {
                return client_.channel( );
            }

            void close_impl( )
            {
                client_.call_request( &stub_type::close, &hdl_ );
            }

            int64_t seek( int64_t pos, file::seek_whence whence ) const override
            {
                fproto::file_set_position   req;
                fproto::file_position       res;
                req.mutable_hdl( )->set_value( hdl_.value( ) );
                req.set_position( pos );
                req.set_whence( whence );
                client_.call( &stub_type::seek, &req, &res );

                return res.position( );
            }

            int64_t tell( ) const override
            {
                fproto::file_position res;
                client_.call( &stub_type::tell, &hdl_, &res );
                return res.position( );
            }

            void flush( ) const override
            {
                client_.call_request( &stub_type::flush, &hdl_ );
            }

            void ioctl( unsigned code, uint64_t param ) const
            {
                fproto::ioctl_req req;
                req.mutable_hdl( )->set_value( hdl_.value( ) );
                req.set_code( code );
                req.set_parameter( param );
                client_.call_request( &stub_type::ioctl, &req );
            }

            size_t read_impl( void *data, size_t length,
                              bool from, uint64_t pos, bool seek_back ) const
            {
                if( 0 == length ) {
                    return 0;
                }
                fproto::file_data_block req_res;

                if( from ) {
                    req_res.mutable_cust_pos( )->set_position( pos );
                    req_res.mutable_cust_pos( )->set_whence(file::POS_SEEK_SET);
                    req_res.mutable_cust_pos( )->set_set_back( seek_back );
                }

                req_res.mutable_hdl( )->set_value( hdl_.value( ) );
                req_res.set_length( length );

                client_.call( &stub_type::read, &req_res, &req_res );

                if( req_res.data( ).size( ) == 0 ) {
                    return 0;
                }

                if( length > req_res.data( ).size( ) ) {
                    length = req_res.data( ).size( );
                }

                memcpy( data, &req_res.data( )[0], length );

                return length;
            }

            size_t write_impl( const void *data, size_t length,
                               bool to, uint64_t pos, bool seek_back ) const
            {
                if( 0 == length ) {
                    return 0;
                }
                if( length > 44000 ) {
                    length = 44000;
                }

                fproto::file_data_block req_res;

                if( to ) {
                    req_res.mutable_cust_pos( )->set_position( pos );
                    req_res.mutable_cust_pos( )->set_whence(file::POS_SEEK_SET);
                    req_res.mutable_cust_pos( )->set_set_back( seek_back );
                }

                req_res.mutable_hdl( )->set_value( hdl_.value( ) );
                req_res.set_data( data, length );
                client_.call( &stub_type::write, &req_res, &req_res );

                return req_res.length( );
            }

            size_t read( void *data, size_t length ) const override
            {
                return read_impl( data, length, false, 0, false );
            }

            size_t write( const void *data, size_t length ) const  override
            {
                return write_impl( data, length, false, 0, false );
            }

            size_t read_from( void *data, size_t length, uint64_t pos,
                              bool seek_back  ) const override
            {
                return read_impl( data, length, true, pos, seek_back );
            }

            size_t write_to( const void *data, size_t length, uint64_t pos,
                             bool seek_back ) const override
            {
                return write_impl( data, length, true, pos, seek_back );
            }

        };
    }


    namespace file {

        iface_ptr create(core &cl,
                      const std::string &path, const std::string &mode,
                      bool as_device )
        {
            return new file_impl( cl, path, mode, as_device );
        }
    }

}}}
