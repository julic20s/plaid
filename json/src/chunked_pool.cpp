#include <json/chunked_pool.h>

using namespace plaid::json;

chunked_pool::chunked_pool() noexcept : first_(nullptr) {}

chunked_pool::~chunked_pool() {
  for (auto it = first_; it;) {
    // 内存块头部放的是下一个内存块的地址
    auto type_chunk = reinterpret_cast<chunk_pointer *>(it);
    auto next = type_chunk[0];
    delete[] type_chunk;
    it = next;
  }
}

void *chunked_pool::aquire(std::size_t chunk_size) {
  auto ptr_place = reinterpret_cast<chunk_pointer *>(
      ::operator new(chunk_size + sizeof(void *))
  );
  ptr_place[0] = first_;
  first_ = ptr_place;
  return ptr_place + 1;
}
