#include "render_pass_internal.h"

using namespace plaid;

render_pass::render_pass(const create_info &) {
  ptr = new render_pass_impl;
}

render_pass::~render_pass() {
  if (ptr) {
    delete ptr;
  }
}
