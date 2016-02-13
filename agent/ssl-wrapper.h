#ifndef SSL_WRAPPER_H
#define SSL_WRAPPER_H

#include <string>
#include <sstream>
#include <stdexcept>
#include <functional>

#include "openssl/ssl.h"
#include "openssl/err.h"
#include "openssl/rsa.h"

namespace ta { namespace utilities {

    class ssl_exception: public std::runtime_error {

        std::string stack_;

        static
        std::string set_what_stack( const char *delimeter = "\n" )
        {
            const char *file;
            int line;
            char buf[512];
            std::ostringstream oss;
            unsigned long err = ERR_get_error_line( &file, &line );
            while( err ) {
                ERR_error_string_n( err, buf, 512 );
                oss << buf << delimeter;
                err = ERR_get_error_line( &file, &line );
            }
            //std::cout << oss.str( );
            return oss.str( );
        }

        static
        std::string set_what( const std::string &arg )
        {
            //return set_what_stack( );
            std::string err;
            if( !arg.empty( ) ) {
                err.append( arg.begin( ), arg.end( ) ).append( ": " );
            }
            size_t final = err.size( );
            err.resize( final + 1024 );
            int error_code = ERR_get_error( );
            ERR_error_string_n( error_code, &err[final], err.size( ) - final );
            return err;
        }

        static
        std::string set_what( const std::string &arg, SSL *s, int res )
        {
            //return set_what_stack( );
            std::string err;
            if( !arg.empty( ) ) {
                err.append( arg.begin( ), arg.end( ) ).append( ": " );
            }
            size_t final = err.size( );
            err.resize( final + 1024 );
            int error_code = SSL_get_error( s, res );
            ERR_error_string_n( error_code, &err[final], err.size( ) - final );
            return err;
        }

    public:
        ssl_exception( )
            :runtime_error(set_what(NULL).c_str( ))
        { }

        ssl_exception( const std::string &arg )
            :runtime_error(set_what(arg).c_str( ))
        { }

        ssl_exception( SSL *s, int res )
            :runtime_error(set_what(NULL, s, res).c_str( ))
        { }

        ssl_exception( const std::string &arg, SSL *s, int res )
            :runtime_error(set_what(arg, s, res).c_str( ))
        { }

        static void raise( )
        {
            throw ssl_exception( );
        }

        static void raise( const std::string &ex )
        {
            throw ssl_exception(ex);
        }

        static void raise( SSL *s, int n )
        {
            throw ssl_exception( s, n );
        }

        static void raise( const std::string &ex, SSL *s, int n )
        {
            throw ssl_exception(ex, s, n);
        }
    };

    class bio_wrapper {
        BIO * b_;
        bool  own_;

        void free_bio( )
        {
            if( own_ && b_ ) {
                BIO_free_all(b_);
            }
        }

        BIO *make_new(BIO_METHOD *meth)
        {
            BIO *b = BIO_new(meth);
            if( !b ) {
                ssl_exception::raise("BIO_new");
            }
            return b;
        }

        static bool error_fatal( int err )
        {
            switch (err) {
            case SSL_ERROR_WANT_READ:
            case SSL_ERROR_WANT_WRITE:
            case SSL_ERROR_NONE:
                return false;
            }
            return true;
        }

    public:

        bio_wrapper(BIO_METHOD *meth)
            :b_(make_new(meth))
            ,own_(true)
        { }

        bio_wrapper( )
            :b_(NULL)
            ,own_(false)
        { }

        bio_wrapper( BIO *b, bool o = true )
            :b_(b)
            ,own_(o)
        { }

        void swap( bio_wrapper &other )
        {
            std::swap( b_,   other.b_   );
            std::swap( own_, other.own_ );
        }

        BIO *release( )
        {
            BIO
            *b = b_;
            b_ = NULL;
            return b;
        }

        BIO *give( )
        {
            own_ = false;
            return b_;
        }

        void reset( BIO * b, bool own = true )
        {
            if( b != b_ ) {
                free_bio(  );
            }
            b_ = b;
            own_ = own;
        }

        void append( BIO * b )
        {
            BIO_push( b_, b );
        }

        void append( bio_wrapper &other )
        {
            BIO_push( b_, other.give( ) );
        }

        BIO *remove( bool owned = true )
        {
            own_ = owned;
            return BIO_pop( b_ );
        }

        void hold( BIO * b )
        {
            reset( b, false );
        }

        void own( BIO * b )
        {
            reset( b, true );
        }

        bool owned( ) const
        {
            return own_;
        }

        BIO *get( )
        {
            return b_;
        }

        const BIO *get( ) const
        {
            return b_;
        }

        ~bio_wrapper( )
        {
            free_bio( );
        }

        void flush( )
        {
            (void)BIO_flush( b_ );
        }

        size_t size( ) const
        {
            char *_;
            size_t len = BIO_get_mem_data( b_, &_ );
            return len;
        }

        char *ptr( )
        {
            char *ptr;
            BIO_get_mem_data( b_, &ptr );
            return ptr;
        }

        size_t write( const std::string &data )
        {
            return write( data.c_str( ), data.size( ) );
        }

        size_t write( const void *data, size_t max )
        {
            const char *ptr = static_cast<const char *>(data);
            int n = 0;
            size_t total = 0;
            while( total < max ) {
                n = BIO_write( b_, &ptr[total], max - total );
                if( n < 0 ) {
                    int err = ERR_get_error( );
                    if( !error_fatal( err ) ) {
                        break;
                    }
                    ssl_exception::raise( "BIO_write" );
                }
                total += n;
            }
            return total;
        }

        size_t read( void *data, size_t max )
        {
            char *ptr = static_cast<char *>(data);
            int n = 0;
            size_t total = 0;
            while( total < max ) {
                n = BIO_read( b_, &ptr[total], max - total );
                if( n < 0 ) {
                    int err = ERR_get_error( );
                    if( !error_fatal( err ) ) {
                        break;
                    }
                    ssl_exception::raise( "BIO_read" );
                }
                total += n;
            }
            return total;
        }

        std::string read( size_t max )
        {
            static const size_t buf_size = 4096;
            std::string res;
            res.reserve(buf_size);
            char buf[buf_size];
            int n = 0;
            while( max > 0 ) {
                n = BIO_read( b_, buf, std::min( buf_size, max) );
                if( n <= 0 ) {
                    int err = ERR_get_error( );
                    if( !error_fatal( err ) ) {
                        break;
                    }
                    ssl_exception::raise( "SSL_read" );
                }
                max -= n;
                res.append( buf, buf + n );
            }
            return res;
        }

        std::string read_all( )
        {
            static const size_t buf_size = 4096;
            std::string res;
            res.reserve(buf_size);
            char buf[buf_size];
            int n = 0;
            while( true ) {
                n = BIO_read( b_, buf, buf_size );
                if( n <= 0 ) {
                    int err = ERR_get_error( );
                    if( !error_fatal( err ) ) {
                        break;
                    }
                    ssl_exception::raise( "SSL_read" );
                }
                res.append( buf, buf + n );
            }
            return res;
        }
    };

    class rsa_wrapper {

        RSA *rsa_;

    public:
        typedef std::function<void(int, int)>              gen_callback;
        typedef std::function<std::string (bool, size_t)>  pem_callback;
    private:

        struct pem_cb {
            rsa_wrapper::pem_callback cb_;
            pem_cb(rsa_wrapper::pem_callback cb)
                :cb_(cb)
            { }

            static int cb( char *buf, int size, int rwflag, void *userptr )
            {
                pem_cb *thiz    = reinterpret_cast<pem_cb *>(userptr);
                std::string res = thiz->cb_(!!rwflag, size);
                size_t len_res  = std::min( res.size( ), size_t(size) );
                memcpy( buf, res.c_str( ), len_res );
                return static_cast<int>(len_res);
            }
        };

    public:


        rsa_wrapper( RSA *rsa )
            :rsa_(rsa)
        { }

        ~rsa_wrapper( )
        {
            if( rsa_ ) {
                RSA_free( rsa_ );
            }
        }

        RSA *get( )
        {
            return rsa_;
        }

        const RSA *get( ) const
        {
            return rsa_;
        }

        bool check(  ) const
        {
            return check( get( ) );
        }

        static bool check( const RSA *rsa )
        {
            int res = RSA_check_key( rsa );
            if( -1 == res ) {
                ssl_exception::raise( "RSA_check_key" );
            }
            return !!res;
        }

        static RSA *from_pem_pubkey_nothrow( const std::string &pubkey,
                                             pem_callback pc = pem_callback( ) )
        {
            int r = 0;
            void *data = pubkey.empty( ) ? (void *)&r : (void *)&pubkey[0];

            RSA *key = NULL;

            bio_wrapper b(BIO_new_mem_buf( data, pubkey.size( ) ));
            pem_cb pcb(pc);
            PEM_read_bio_RSAPublicKey( b.get( ), &key,
                                       pc ? &pem_cb::cb : NULL, &pcb );
            return key;
        }

        static RSA *from_pem_prikey_nothrow( const std::string &prikey,
                                             pem_callback pc = pem_callback( ) )
        {
            int r = 0;
            void *data = prikey.empty( ) ? (void *)&r : (void *)&prikey[0];
            bio_wrapper b(BIO_new_mem_buf( data, prikey.size( ) ));

            RSA *key = NULL;

            pem_cb pcb(pc);
            PEM_read_bio_RSAPrivateKey( b.get( ), &key,
                                        pc ? &pem_cb::cb : NULL, &pcb );

            return key;
        }

        static RSA *from_pem_pubkey( const std::string &prikey,
                                     pem_callback pc = pem_callback( ) )
        {
            RSA *key = from_pem_pubkey_nothrow( prikey, pc );
            if( !key ) {
                ssl_exception::raise( "PEM_read_bio_RSAPublicKey" );
            }
            return key;
        }

        static RSA *from_pem_prikey( const std::string &prikey,
                                     pem_callback pc = pem_callback( ) )
        {
            RSA *key = from_pem_prikey_nothrow( prikey, pc );
            if( !key ) {
                ssl_exception::raise( "PEM_read_bio_RSAPrivateKey" );
            }
            return key;
        }

        std::string pem_pubkey( )
        {
            bio_wrapper b(BIO_s_mem( ));
            if( !PEM_write_bio_RSAPublicKey( b.get( ), rsa_ ) ) {
                ssl_exception::raise( "PEM_write_bio_RSAPublicKey" );
            }

            std::string res( BIO_pending(b.get( )), '\0' );
            BIO_read( b.get( ), &res[0], res.size( ) );

            return res;
        }

        std::string pem_prikey( )
        {
            bio_wrapper b(BIO_s_mem( ));

            if( !PEM_write_bio_RSAPrivateKey( b.get( ), rsa_, NULL,
                                              NULL, 0,
                                              NULL, NULL ) )
            {
                ssl_exception::raise("write_bio_RSAPrivateKey");
            }

            std::string res( BIO_pending(b.get( )), '\0' );
            BIO_read( b.get( ), &res[0], res.size( ) );

            return res;
        }

        std::string pem_prikey( const EVP_CIPHER *cyph, pem_callback pc )
        {

            bio_wrapper b(BIO_s_mem( ));

            pem_cb pcb(pc);
            if( !PEM_write_bio_RSAPrivateKey( b.get(), rsa_, cyph,
                                              NULL, 0,
                                              pc ? &pem_cb::cb : NULL,
                                              &pcb ) )
            {
                ssl_exception::raise("write_bio_RSAPrivateKey");
            }

            std::string res( BIO_pending(b.get( )), '\0' );
            BIO_read( b.get( ), &res[0], res.size( ) );

            return res;
        }

        std::string pem_prikey( const EVP_CIPHER *cyph, std::string &pass )
        {

            bio_wrapper b(BIO_s_mem( ));

            unsigned char *key = pass.empty( )
                               ? NULL
                               : reinterpret_cast<unsigned char *>( &pass[0] );

            if( !PEM_write_bio_RSAPrivateKey( b.get( ), rsa_, cyph,
                                              key, pass.size( ),
                                              NULL, NULL ) )
            {
                ssl_exception::raise("write_bio_RSAPrivateKey");
            }

            std::string res( BIO_pending(b.get( )), '\0' );
            BIO_read( b.get( ), &res[0], res.size( ) );

            return res;
        }


        static RSA *generate_keys( size_t bits, unsigned long e = RSA_F4,
                                   gen_callback gcback = gen_callback( ) )
        {

            RSA *res = generate_keys_nothrow( bits, e, gcback );
            if( !res ) {
                ssl_exception::raise( "RSA_generate_key" );
            }
            return res;
        }

        /// use ERR_get_error for opbtener th eerror code
        static RSA *generate_keys_nothrow( size_t bits,
                                       unsigned long e = RSA_F4,
                                       gen_callback gcback = gen_callback( ) )
        {
            struct gen_cb {
                gen_callback cb_;
                gen_cb(gen_callback cb)
                    :cb_(cb)
                { }

                static void cb( int p1, int p2, void *userdata )
                {
                    gen_cb *thiz = reinterpret_cast<gen_cb *>( userdata );
                    thiz->cb_( p1, p2 );
                }
            };

            gen_cb gcb(gcback);
            return RSA_generate_key(bits, e, gcback ? &gen_cb::cb : NULL, &gcb);
        }

    };

    class ssl_bio_wrapper {

        SSL         *ssl_;
        SSL_CTX     *ctx_;
        bio_wrapper  in_;
        bio_wrapper  out_;

        static bool error_fatal( int err )
        {
            switch (err) {
            case SSL_ERROR_WANT_READ:
            case SSL_ERROR_WANT_WRITE:
            case SSL_ERROR_NONE:
                return false;
            default:
                break;
            }
            return true;
        }

        int check_raise_fatal( int res, const char *arg )
        {
            int err = SSL_ERROR_NONE;
            if( res <= 0  ) {
                err = SSL_get_error( ssl_, res );
                if( error_fatal(err) ) {
                    ssl_exception::raise( arg, ssl_, err );
                }
            }
            return err;
        }

    public:

        ssl_bio_wrapper( const SSL_METHOD *meth )
            :ssl_(NULL)
            ,ctx_(SSL_CTX_new(meth))
            ,in_(BIO_s_mem( ))
            ,out_(BIO_s_mem( ))
        {
            if ( !ctx_ ) {
                ssl_exception::raise( "SSL_CTX_new" );
            }
        }

        virtual ~ssl_bio_wrapper( )
        {
            if( ssl_ ) {
                SSL_free( ssl_ );
            }
            if( ctx_ ) {
                SSL_CTX_free( ctx_ );
            }
        }

    public:

        virtual void init_context( ) = 0;
        virtual void init_ssl( )     = 0;

        void init( )
        {
            init_context( );

            ssl_ = SSL_new( ctx_ );

            if( !ssl_ ) {
                ssl_exception::raise( "SSL_new" );
            }

            SSL_set_bio( ssl_, in_.give( ), out_.give( ) );

            init_ssl( );
        }

        bool init_finished( ) const
        {
            return SSL_is_init_finished(ssl_);
        }

        SSL *get_ssl( )
        {
            return ssl_;
        }

        const SSL *get_ssl( ) const
        {
            return ssl_;
        }

        SSL_CTX *get_context( )
        {
            return ctx_;
        }

        const SSL_CTX *get_context( ) const
        {
            return ctx_;
        }

        size_t write( const void *data, size_t max )
        {
            const char *ptr = static_cast<const char *>(data);
            size_t total = 0;

            int n = SSL_write( ssl_, &ptr[total], max - total );
            check_raise_fatal( n, "SSL_write" );
            total += n;

            return total;
        }

        size_t write_all( const void *data, size_t max )
        {
            const char *ptr = static_cast<const char *>(data);
            int n = 0;
            size_t total = 0;
            while( total < max ) {
                n = SSL_write( ssl_, &ptr[total], max - total );
                check_raise_fatal( n, "SSL_write" );
                total += n;
            }
            return total;
        }

        std::string read_all( )
        {
            std::string res;
            res.reserve( 4096 );
            char buf[4096];
            int n = 0;
            while( true ) {
                n = SSL_read( ssl_, buf, sizeof(buf) );
                if( n <= 0 ) {
                    check_raise_fatal( n, "SSL_read" );
                    break;
                }
                res.append( buf, buf + n );
            }
            return res;
        }

        size_t read( void *data, size_t max )
        {
            char *ptr = static_cast<char *>(data);
            int n = 0;
            size_t total = 0;
            while( total < max ) {
                n = SSL_read( ssl_, &ptr[total], max - total );
                if( n <= 0 ) {
                    check_raise_fatal( n, "SSL_read" );
                    break;
                }
                total += n;
            }
            return total;
        }

        bio_wrapper &get_in( )
        {
            return in_;
        }

        const bio_wrapper &get_in( ) const
        {
            return in_;
        }

        bio_wrapper &get_out( )
        {
            return out_;
        }

        const bio_wrapper &get_out( ) const
        {
            return out_;
        }

        std::string do_handshake( )
        {
            return do_handshake( NULL, 0 );
        }

        std::string do_handshake( const char *data, size_t length )
        {
            if( length ) {
                in_.write( data, length );
            }

            int n = SSL_do_handshake( ssl_ );
            check_raise_fatal( n, "SSL_do_handshake" );

            return out_.read_all( );
        }

        std::string encrypt( const std::string &data )
        {
            return encrypt( data.c_str( ), data.size( ) );
        }

        std::string encrypt( const char *data, size_t length )
        {
            write_all( data, length );
            return out_.read_all( );
        }

        std::string decrypt( const std::string &data )
        {
            return decrypt( data.c_str( ), data.size( ) );
        }

        std::string decrypt( const char *data, size_t length )
        {
            if( 0 == length ) {
                return std::string( );
            }

            in_.write( data, length );
            return read_all( );
        }

    };

    class ssl_wrapper_server: public ssl_bio_wrapper {
    public:
        ssl_wrapper_server(const SSL_METHOD *meth)
            :ssl_bio_wrapper(meth)
        { }
    private:
        void init_ssl( )
        {
            SSL_set_accept_state( get_ssl( ) );
        }
    };

    class ssl_wrapper_client: public ssl_bio_wrapper {
    public:
        ssl_wrapper_client(const SSL_METHOD *meth)
            :ssl_bio_wrapper(meth)
        { }
    private:
        void init_ssl( )
        {
            SSL_set_connect_state(  get_ssl( ) );
        }
    };

    class ssl_wrapper_common: public ssl_bio_wrapper {
        bool inbound_;
    public:
        ssl_wrapper_common(const SSL_METHOD *meth, bool inbound)
            :ssl_bio_wrapper(meth)
            ,inbound_(inbound)
        { }

    private:

        void init_ssl( )
        {
            if(inbound_) {
                SSL_set_accept_state( get_ssl( ) );
            } else {
                SSL_set_connect_state(  get_ssl( ) );
            }
        }
    };

}} /// ta::utilities


#endif // SSL_WRAPPER_H

