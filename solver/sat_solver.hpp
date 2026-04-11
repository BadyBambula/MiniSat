#pragma once

#include "cnf/cnf_parser.hpp"
#include "cnf/cnf_types.hpp"

struct SolveResult {
    bool satisfiable;               // SAT or UNSAT
    std::vector<int> true_literals; // vector of true literals (trail)
    std::vector<int> assignment;    // assigns[i] is the assignment of variable i (assigns)
    int unit_props;                 // number of unit propagations
    int dec_vars;                   // number of decision variables in the recursion tree
};

SolveResult solve_sat(const CNF &cnf, int num_variables);
