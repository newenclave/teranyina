#ifndef TERANYINA_RESULT_HPP
#define TERANYINA_RESULT_HPP

#include <system_error>
#include "noexcept.hpp"

namespace ta {

    class error_message {
        const char *mess_;
        public:

        explicit error_message(const char *mess)
            :mess_(mess)
        { }

        const char *message( ) const
        {
            return mess_;
        }

        int value( ) const
        {
            return -1;
        }

    };

    template <typename T, typename Err = error_message>
    struct result_type {
        error_message err_;
        T result_;

    };
}

#endif // RESULT_HPP
