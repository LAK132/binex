#ifndef BINEX_MAIN_HPP
#define BINEX_MAIN_HPP

#include <lak/system/architecture.hpp>

#include "git.hpp"
#define APP_VERSION GIT_TAG "-" GIT_HASH
#define APP_NAME    "binex " STRINGIFY(LAK_ARCH) " " APP_VERSION

#endif
