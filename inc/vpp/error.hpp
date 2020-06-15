/**
 *
 * @file      error.hpp
 *
 * @brief     These are all the error and status codes for the library
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

#include "customisation/error.hpp"

/** Customisation error codes reuse macro */
#define CustomisationError(x) static_cast<int>(Customisation::Error::x)

namespace Error {

using Type = int;

static constexpr int NOT_READY       = 2;
static constexpr int RETRY           = 1;
static constexpr int OK              = 0;
static constexpr int NONE            = CustomisationError(NONE);
static constexpr int TYPE_MISMATCH   = CustomisationError(TYPE_MISMATCH);
static constexpr int INVALID_REQUEST = CustomisationError(INVALID_REQUEST);
static constexpr int INVALID_VALUE   = CustomisationError(INVALID_VALUE);
static constexpr int INVALID_RANGE   = CustomisationError(INVALID_RANGE);
static constexpr int NOT_EXISTING    = CustomisationError(NOT_EXISTING);
static constexpr int UNDEFINED       = CustomisationError(UNDEFINED);
static constexpr int UNKNOWN         = CustomisationError(UNKNOWN);

}  // namespace Error

