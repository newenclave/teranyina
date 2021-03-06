#ifndef TA_FILEIFACE_H
#define TA_FILEIFACE_H

#include "vtrc-stdint.h"

#include <string>

namespace ta { namespace agent {

    struct file_iface {

        enum seek_whence {
             F_SEEK_SET = SEEK_SET
            ,F_SEEK_CUR = SEEK_CUR
            ,F_SEEK_END = SEEK_END
        };

        virtual ~file_iface( ) { }

        virtual vtrc::int64_t seek( vtrc::int64_t offset,
                                     seek_whence whence ) = 0;
        virtual vtrc::int64_t tell( ) const = 0;

        virtual void ioctl( int code, unsigned long data ) = 0;
        virtual void ioctl( int code, void *data ) = 0;

        virtual size_t write( const void *data, size_t length ) = 0;
        virtual size_t read(        void *data, size_t length ) = 0;

        virtual void flush( ) = 0;

        virtual int handle( ) const = 0;
    };

    typedef file_iface * file_ptr;

    namespace file {
#ifndef _WIN32
        file_ptr create( std::string const &path, int flags );
        file_ptr create( std::string const &path, int flags, mode_t mode );
#endif
        file_ptr create( std::string const &path, const std::string &mode );
    }

    namespace device {
#ifndef _WIN32
        file_ptr create( std::string const &path, int flags );
        file_ptr create( std::string const &path, int flags, mode_t mode );
#endif
        file_ptr create( std::string const &path, const std::string &mode );
    }

}}

#endif // FR_FILEIFACE_H
