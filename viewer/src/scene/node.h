#pragma once
#ifndef PLAID_VIEWER_NODE_H_
#define PLAID_VIEWER_NODE_H_

#include <cstddef>

#include <span>

#include <plaid/mat.h>

#include "mesh.h"

namespace plaid_viewer {

/// 场景树节点
struct node {
  plaid::mat4x4 transition;
  std::span<node> children;
  mesh *mesh;
};

}

#endif
