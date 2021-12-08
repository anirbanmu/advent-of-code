#ifndef _UTILS_H_
#define _UTILS_H_

#include <charconv>
#include <stdexcept>
#include <string>

namespace Utils {
    unsigned long long str_to_ull(const std::string_view &str) {
        unsigned long long number = 0;
        auto [ptr, err] = std::from_chars(str.data(), str.data() + str.size(), number);
        if (err != std::errc{} || ptr != str.data() + str.size()) {
            throw std::runtime_error{"malformed_input"};
        }
        return number;
    }
} // namespace Utils

#endif
