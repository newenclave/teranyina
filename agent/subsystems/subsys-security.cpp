
#include "subsys-security.h"
#include "../application.h"

#include "common/ssl-wrapper.hpp"
#include "common/utils.h"

#include "vtrc-common/vtrc-random-device.h"

#include "openssl/rand.h"

#define LOG(lev) log_(lev, "security")
#define LOGNS(lev) log_(lev, "security", false)

#define LOGINF     LOG(level::info)
#define LOGINFNS   LOGNS(level::info)
#define LOGDBG     LOG(level::debug)
#define LOGERR     LOG(level::error)
#define LOGWRN     LOG(level::warning)

namespace ta { namespace agent { namespace subsys {

    namespace {

        namespace vcomm = vtrc::common;
        const std::string subsys_name( "security" );

        using level = agent::logger::level;

//        application::service_wrapper_sptr create_service(
//                                      ta::agent::application * /*app*/,
//                                      vtrc::common::connection_iface_wptr cl )
//        {
//            ///auto inst = std::make_shared<impl_type_here>( app, cl );
//            ///return app->wrap_service( cl, inst );

//            return application::service_wrapper_sptr( );
//        }
    }

    struct security::impl {

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

        void init( )
        {
            using namespace ta::utilities;
            LOGINF << "init start";

            SSL_load_error_strings( );
            SSLeay_add_all_algorithms( );

//#if 1 /// valgrind is not happy because of generate_block

            vcomm::random_device rd;

            auto seed = rd.generate_block( 128 );
            RAND_seed( seed.c_str( ), seed.size( ));
            if( RAND_status( ) ) {
                LOGINF << "The PRNG has been seeded with enough data";
            } else {
                LOGWRN << "The PRNG has not been seeded with enough data";
            }

#if 1
            rsa_wrapper rw(rsa_wrapper::generate_keys( 1024 ));
            EVP_PKEY * pkey = EVP_PKEY_new( );
            EVP_PKEY_assign_RSA( pkey, rw.get( ) );

            bio_wrapper bw;
            EVP_PKEY_print_public( bw.get( ), pkey, EVP_PKEY_RSA, NULL );
            //EVP_PKEY_print_private(bw.get( ), pkey, 6, NULL);

            //bw.flush( );

            LOGINF << "\n" << bw.read_all( ) << "\n";
            LOGINF << "\n" << rw.pem_pubkey( );

            std::string pass = "123";

            LOGINF << "\n" << rw.pem_prikey( EVP_aes_256_cbc( ), pass );
#endif
        }

    };

    security::security( application *app )
        :impl_(new impl(app))
    { }

    security::~security( )
    {
        delete impl_;
    }

    /// static
    security::shared_type security::create( application *app )
    {
        shared_type new_inst(new security(app));
        return new_inst;
    }

    const std::string &security::name( )  const NOEXCEPT
    {
        return subsys_name;
    }

    void security::init( )
    {
        impl_->init( );
    }

    void security::start( )
    {
        impl_->LOGINF << "Started";
    }

    void security::stop( )
    {
        impl_->LOGINF << "Stopped";
    }


}}}

    
