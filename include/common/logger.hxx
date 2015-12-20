#ifndef TA_LOGGER_HXX
#define TA_LOGGER_HXX

#include <sstream>
#include <stdint.h>

namespace ta { namespace common {

    enum log_level {
         zero    = 0
        ,error   = 1
        ,warning = 2
        ,info    = 3
        ,debug   = 4
    };

    class logger {

    private:

        log_level level_;

        struct string_accumulator {

            logger                 &parent_;
            log_level               level_;
            std::ostringstream      oss_;
            bool                    act_;

            string_accumulator( logger &parent, log_level lev )
                :parent_(parent)
                ,level_(lev)
                ,act_(true)
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
                    parent_.send_data( level_, oss_.str( ) );
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

        virtual void send_data( log_level lev, const std::string &data ) = 0;

        string_accumulator operator ( )( log_level lev )
        {
            string_accumulator res( *this, lev );
            return res;
        }

        string_accumulator operator ( )( )
        {
            string_accumulator res( *this, level_ );
            return res;
        }

        logger( log_level lev )
            :level_(lev)
        { }

        virtual ~logger( )
        { }

        log_level get_level( ) const
        {
            return level_;
        }

        void set_level( log_level lev )
        {
            level_ = lev;
        }

    };

}}

#endif // LOGGER_HXX

