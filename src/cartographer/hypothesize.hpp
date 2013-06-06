#pragma once

#include "util.hpp"
#include "structure.hpp"

namespace pomagma
{

float hypothesize_entropy (
        Structure & structure,
        const std::unordered_map<std::string, float> & language,
        const std::pair<Ob, Ob> & equation,
        float reltol = 1e-2f);

} // namespace pomagma