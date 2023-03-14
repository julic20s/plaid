#pragma once

#include <istream>

#include "header.h"

namespace gltf {
void parse(std::istream &, header &);
}
