#ifndef BINEX_MAIN_HPP
#define BINEX_MAIN_HPP

#include <lak/architecture.hpp>

#include "git.hpp"
#define APP_VERSION GIT_TAG "-" GIT_HASH
#define APP_NAME    "binex " STRINGIFY(LAK_ARCH) " " APP_VERSION

#define LAK_BASIC_PROGRAM_IMGUI_WINDOW_IMPL
#include <lak/basic_program.inl>

#endif
