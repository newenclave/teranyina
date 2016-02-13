#ifndef TA_LOGGER_HXX
#define TA_LOGGER_HXX

#include <sstream>
#include <stdint.h>

namespace ta { namespace common {

    class logger {

    public:

        enum class level {
             zero    = 0
            ,error   = 1
            ,warning = 2
            ,info    = 3
            ,debug   = 4
        };

    private:

        level level_;

        struct string_accumulator {

            logger                 &parent_;
            level                   level_;
            std::ostringstream      oss_;
            bool                    act_;
            bool                    split_;

            string_accumulator( logger &parent, level lev, bool split )
                :parent_(parent)
                ,level_(lev)
                ,act_(true)
                ,split_(split)
            { }

            string_accumulator( string_accumulator &other )
                :parent_(other.parent_)
                ,level_(other.level_)
                ,act_(true)
            {
                other.act_ = false;
            }

            string_accumulator& operator = ( string_accumulator &other )
            {
                level_     = other.level_;
                other.act_ = false;
                return *this;
            }

            ~string_accumulator( )
            {
                if( act_ && ( level_ <= parent_.level_ ) ) {
                    split_ ? parent_.send_data( level_, oss_.str( ) )
                           : parent_.send_data_nosplit( level_, oss_.str( ) );
                }
            }

            template<typename T>
            string_accumulator &operator << ( const T &data )
            {
                if( level_ <= parent_.level_ ) {
                    oss_ << data;
                }
                return *this;
            }
        };

    public:

        virtual void send_data( level lev, const std::string &data ) = 0;
        virtual void send_data_nosplit( level lev,
                                        const std::string &data ) = 0;

        string_accumulator operator ( )( level lev, bool split )
        {
            string_accumulator res( *this, lev, split );
            return res;
        }

        string_accumulator operator ( )( level lev )
        {
            string_accumulator res( *this, lev, true );
            return res;
        }

        string_accumulator operator ( )( bool split )
        {
            string_accumulator res( *this, level_, split );
            return res;
        }

        string_accumulator operator ( )( )
        {
            string_accumulator res( *this, level_, true );
            return res;
        }

        logger( level lev )
            :level_(lev)
        { }

        virtual ~logger( )
        { }

        level get_level( ) const
        {
            return level_;
        }

        void set_level( level lev )
        {
            level_ = lev;
        }

    };

}}

#endif // LOGGER_HXX

