#pragma once

#include <cstddef>

namespace plaid::json {

class chunked_pool {

public:
  using chunk_pointer = void *;

  chunked_pool() noexcept;

  ~chunked_pool();

  [[nodiscard]] chunk_pointer aquire(std::size_t chunk_size);

private:
  chunk_pointer first_;
};

} // namespace plaid::json
