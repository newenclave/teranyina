#ifndef TA_APPLICATION_CPP
#define TA_APPLICATION_CPP

#include <memory>

#include "vtrc-server/vtrc-application.h"
#include "vtrc-common/vtrc-signal-declaration.h"

namespace ta { namespace agent {

    class application: public vtrc::server::application {
        struct impl;
        std::unique_ptr<impl> impl_;
    public:
        application( application & ) = delete;
        application & operator = ( application & ) = delete;

        application( );
    };
}}

#endif // APPLICATION_CPP

