#ifndef CMDLIST_H
#define CMDLIST_H

#include "cmd-iface.h"

namespace ta { namespace cc { namespace cmd {

    using ta::cc::cmd_sptr;

    namespace fs            { cmd_sptr create( ); }
    namespace ctrl          { cmd_sptr create( ); }
    namespace scripting     { cmd_sptr create( ); }

}}}

#endif // CMDLIST_H
