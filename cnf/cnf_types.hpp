#pragma once

#include <vector>

using Lit = int;
using Clause = std::vector<Lit>;
using CNF = std::vector<Clause>;

constexpr int var(Lit l) noexcept { return l / 2; }
constexpr int neg(Lit l) noexcept { return l ^ 1; }
constexpr bool sgn(Lit l) noexcept { return (l & 1) == 0; }
