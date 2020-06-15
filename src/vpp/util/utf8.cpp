/**
 *
 * @file      vpp/util/utf8.cpp
 *
 * @brief     This is the implementation of functions for handling UTF8 strings
 *
 *            This file is part of the VPP framework (see link).
 *
 * @author    Olivier Stoltz-Douchet <ezdayo@gmail.com>
 *
 * @copyright (c) 2019-2020 Olivier Stoltz-Douchet
 * @license   http://opensource.org/licenses/MIT MIT
 * @link      https://github.com/ezdayo/vpp
 *
 **/

#include "vpp/util/utf8.hpp"

namespace Util {
namespace UTF8 {

/* Removing diacritics */
void toASCII(std::string &str) {
    /* Non-accentuated replacement latin ASCII characters */
    static const char *tr = "AAAAAAECEEEEIIIIDNOOOOOx0UUUUYPsaaaaaaeceeeeiiii"
                            "Onooooo/0uuuuypy";

    char *from  = const_cast<char *>(str.c_str());
    char *to    = from;
    auto sz     = str.length();
    auto new_sz = sz;

    while (sz > 0) {
        auto ch = static_cast<unsigned char>(*from);
        if (ch == 0xc3) {
            ++from;
            --new_sz;
            ch = static_cast<unsigned char>(*from);
            *to = tr[ch-128];
        } else {
            *to = *from;
        }
        ++from;
        ++to;
        --sz;
    }
    *to = '\0';
    str.resize(new_sz);
}

} // namespace UTF8
} // namespace Util
