#pragma once
#ifndef PLAID_JSON_DOM_H_
#define PLAID_JSON_DOM_H_

#include "chunked_pool.h"
#include "value.h"

namespace plaid::json {

class dom : public value {
public:
  dom(const char *file);

private:
  chunked_pool pool_;
};

} // namespace plaid::json

#endif
