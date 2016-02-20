#include <iostream>
#include "client-core/ta-client.h"

#include "vtrc-client/vtrc-client.h"

namespace ta { namespace client {

    namespace vclient = vtrc::client;
    namespace vcomm   = vtrc::common;

    struct core::impl {
        std::shared_ptr<vclient::vtrc_client> client_;
        impl( vcomm::pool_pair &pp )
            :client_(vclient::vtrc_client::create(pp))
        { }
    };

    core::core( vtrc::common::pool_pair &pp )
        :impl_(new impl(pp))
    { }

    core::~core( )
    {
        delete impl_;
    }

    vtrc::client::vtrc_client &core::get_client( )
    {
        return *impl_->client_;
    }

    const vtrc::client::vtrc_client &core::get_client( ) const
    {
        return *impl_->client_;
    }

}}
