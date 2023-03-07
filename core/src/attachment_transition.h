#pragma once
#ifndef PLAID_ATTACHMENT_TRANSITION_H_
#define PLAID_ATTACHMENT_TRANSITION_H_

#include <cstddef>

#include <plaid/format.h>

namespace plaid {

using attachment_transition_function = void(const std::byte *, std::byte *);

attachment_transition_function *match_attachment_transition_function(format src, format dst);

void RGB32f_to_BGRA8u(const std::byte *src, std::byte *dst);

void RGBA32f_to_BGRA8u(const std::byte *src, std::byte *dst);

void RGBA32u_to_BGRA8u(const std::byte *src, std::byte *dst);

} // namespace plaid

#endif
