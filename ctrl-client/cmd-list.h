#ifndef CMDLIST_H
#define CMDLIST_H

#include "cmd-iface.h"

namespace ta { namespace cc { namespace cmd {

    using ta::cc::cmd_sptr;

    namespace ctrl   { cmd_sptr create( ); }

}}}

#endif // CMDLIST_H
