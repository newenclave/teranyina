#ifndef FR_INTERFACE_FILE_H
#define FR_INTERFACE_FILE_H

#include <string>
#include <stdint.h>

#include "vtrc-function.h"
#include "IBaseIface.h"

namespace ta { namespace client {

    class core;

namespace interfaces { namespace file {

    enum seek_whence {
         POS_SEEK_SET = 0
        ,POS_SEEK_CUR = 1
        ,POS_SEEK_END = 2
    };

    static
    inline seek_whence whence_value2enum( unsigned value )
    {
        switch ( value ) {
        case file::POS_SEEK_SET:
        case file::POS_SEEK_CUR:
        case file::POS_SEEK_END:
            return static_cast<seek_whence>(value);
        }
        return file::POS_SEEK_SET;
    }

    struct base_file_iface: public interfaces::base {
        virtual ~base_file_iface( ) { }
        virtual size_t  read( void *data,       size_t length  ) const = 0;
        virtual size_t write( const void *data, size_t length  ) const = 0;
        virtual void   ioctl( unsigned code,    uint64_t param ) const = 0;
    };

    struct iface: public base_file_iface {
        virtual ~iface( ) { }

        virtual int64_t seek( int64_t pos, seek_whence whence ) const = 0;
        virtual int64_t tell( )  const = 0;
        virtual void    flush( ) const = 0;

        virtual size_t read_from( void *data, size_t length, uint64_t pos,
                                  bool seek_back = false ) const = 0;
        virtual size_t write_to( const void *data, size_t length, uint64_t pos,
                                 bool seek_back = false ) const = 0;

    };

    typedef iface* iface_ptr;

    /// fopen
    iface_ptr create( core &cl,
                      const std::string &path, const std::string &mode,
                      bool as_device );

}}}}

#endif // IFILE_H
