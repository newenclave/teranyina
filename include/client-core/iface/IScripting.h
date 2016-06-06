#ifndef TA_ISCRIPTING_H
#define TA_ISCRIPTING_H

#include <string>
#include "vtrc-function.h"
#include "IBaseIface.h"

namespace ta { namespace client {

    class core;

namespace interfaces { namespace scripting {

    struct iface: public interfaces::base {
        virtual ~iface( ) { }
        virtual void init( ) = 0;
        virtual void execute_buffer( const std::string &buf,
                                     const std::string &name ) = 0;
        virtual void execute_file( const std::string &path ) = 0;
    };

    typedef iface * iface_ptr;
    iface_ptr create( core &cl );

}}

}}

#endif // ISCRIPTING_H

