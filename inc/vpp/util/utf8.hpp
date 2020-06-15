/**
 *
 * @file      vpp/util/utf8.hpp
 *
 * @brief     This is a set of functions for handling UTF8 strings
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

#pragma once

#include <string>

namespace Util {
namespace UTF8 {

void toASCII(std::string &str);

} // namespace UTF8
} // namespace Util
