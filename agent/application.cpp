#include "application.h"

namespace ta { namespace agent {

    struct application::impl {
        impl( )
        {

        }
    };

    application::application( )
        :impl_(new impl)
    { }

}}
