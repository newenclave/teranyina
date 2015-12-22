#ifndef TA_PROTOCOL_CONSTANTS_H
#define TA_PROTOCOL_CONSTANTS_H

#include <string>

namespace ta { namespace proto {

namespace multicast {

/*******************************************************************************

multicast address:

+------------------------------------------------------------------------------+
|                    |          |                   IPv4                       |
|  1: Scope          | 2: IPv6  |--------------+-------------------------------|
|                    |    scope | 3: TTL scope |   4:  Administrative scope    |
|--------------------+----------+--------------+-------------------------------|
| Interface-local    |     1    |      0       |                               |
| Link-local         |     2    |      1       |   224.0.0.0 - 224.0.0.255     |
| Site-local         |     5    |     <32      | 239.255.0.0 - 239.255.255.255 |
| Organization-local |     8    |              | 239.192.0.0 - 239.195.255.255 |
| Global             |    14    |    <=255     |   224.0.1.0 - 238.255.255.255 |
+------------------------------------------------------------------------------+

 IPv6 multicast structure:
 1 byte             : 0xff
 1 byte
      \ high  4 bits: flags
      \ low   4 bits: IPv6 scope
 80 bytes           : zeros
 Last 4 bytes       : group ID

*******************************************************************************/

    static const std::string default_address_v4 = "239.193.37.11";
    static const std::string default_address_v6 = "ff08::37";
    static const unsigned short default_port    =  38711;

}

}}

#endif // TA_PROTOCOL_CONSTANTS_H
