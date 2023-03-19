#include "scene.h"

using namespace plaid_viewer;

static void render_node(const node &target, const plaid::mat4x4 &trans) {
  auto next_trans = target.transition * trans;

  for (auto &c : target.children) {
    render_node(c, next_trans);
  }
}

void scene::render() const {
  render_node(*this, transition);
}
