#pragma once

#include "cnf/cnf_types.hpp"

struct SolveResult {
    bool satisfiable;
    std::vector<int> assignment;
};

SolveResult solve_sat(const CNF &cnf, int num_variables);
