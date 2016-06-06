#ifndef TA_INTERFACE_FILESYSTEM_H
#define TA_INTERFACE_FILESYSTEM_H

#include "IBaseIface.h"

#include <string>
#include "vtrc-stdint.h"


namespace ta { namespace client {

    class core;

namespace interfaces { namespace filesystem {

    struct info_data {
        bool is_exist;
        bool is_directory;
        bool is_empty;
        bool is_regular;
        bool is_symlink;
    };

    struct iterator_value {
        std::string path;
    };

    inline bool operator == ( const iterator_value &l, const iterator_value &r )
    { return  l.path == r.path; }

    struct directory_iterator_impl: public interfaces::base {

        virtual ~directory_iterator_impl( ) { }

        virtual void next( ) = 0;

        virtual bool end( ) const = 0;

        virtual directory_iterator_impl *clone( ) const = 0;

        virtual iterator_value &get( ) = 0;
        virtual const iterator_value &get( ) const = 0;

        virtual const info_data &info( ) const = 0;

    };

    typedef std::iterator < std::input_iterator_tag,
                            iterator_value
                          > directory_iterator_traits;


    class directory_iterator: public directory_iterator_traits {

        directory_iterator_impl *impl_;

    public:

        directory_iterator( directory_iterator_impl *impl );

        typedef directory_iterator_traits::value_type value_type;

        directory_iterator( );

        directory_iterator& operator = ( directory_iterator &other );
        ~directory_iterator( );

        directory_iterator& operator++ ( );
        directory_iterator& operator++ ( int );

        bool operator == (const directory_iterator& rhs) const;
        bool operator != (const directory_iterator& rhs) const;

        const value_type& operator *( ) const;
        const value_type* operator -> ( ) const;

        const info_data& info( ) const;

    };

    struct iface: public interfaces::base {

        virtual ~iface( ) { }

        virtual bool exists( const std::string &path ) const = 0;

        virtual void info( const std::string &path, info_data &data ) const = 0;

        virtual vtrc::uint64_t file_size( const std::string &path ) const = 0;

        virtual void rename( const std::string &old_path,
                             const std::string &new_path ) const = 0;

        virtual void cd( const std::string &path ) = 0;
        virtual const std::string &pwd(  ) const = 0;

        virtual void mkdir( const std::string &path )       const = 0;
        virtual void del( const std::string &path )         const = 0;
        virtual void remove_all( const std::string &path )  const = 0;

        virtual directory_iterator_impl *begin_iterate(
                                const std::string &path ) const = 0;
        virtual directory_iterator_impl *begin_iterate( ) const = 0;
        virtual directory_iterator begin( ) const = 0;
        virtual directory_iterator end( ) const = 0;

        virtual size_t read_file( const std::string &path,
                                  void *data, size_t max ) const = 0;
        virtual size_t write_file( const std::string &path,
                                   const void *data, size_t max ) const = 0;

    };

    typedef iface* iface_ptr;
    iface_ptr create( core &cl, const std::string &path );


}}}}

#endif // FR_INTERFACE_FILESYSTEM_H
