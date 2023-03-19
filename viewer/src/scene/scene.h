#pragma once
#ifndef PLAID_VIEWER_SCENE_H_
#define PLAID_VIEWER_SCENE_H_

#include <memory>

#include "mesh.h"
#include "node.h"

namespace plaid_viewer {

class scene : node {

public:

  void render() const;

private:
  std::unique_ptr<plaid_viewer::node[]> m_nodes;
  std::unique_ptr<plaid_viewer::mesh[]> m_meshs;
};

} // namespace plaid_viewer

#endif
