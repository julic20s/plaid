#pragma once
#ifndef PLAID_VIEWER_GLTF_H_
#define PLAID_VIEWER_GLTF_H_

#include <json/dom.h>

namespace plaid_viewer {

class gltf {

public:
  gltf(const char *file);

  

private:
  json::dom json;
};

} // namespace plaid_viewer

#endif
