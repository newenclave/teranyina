#include <iostream>
#include "client-core/ta-client.h"

#include "vtrc-client/vtrc-client.h"

namespace ta { namespace client {

    struct core::impl: public vtrc::client::vtrc_client {
        impl( vtrc::common::pool_pair &pp )
            :vtrc::client::vtrc_client(pp)
        { }
    };

    core::core( vtrc::common::pool_pair &pp )
        :impl_(new impl(pp))
    { }

    core::~core( )
    {
        delete impl_;
    }

}}
