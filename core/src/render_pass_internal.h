#pragma once

#include <cstddef>
#include <cstdint>

#include <plaid/render_pass.h>

class plaid::render_pass_impl {
public:
  std::byte *attachments[1 << 8];
};


