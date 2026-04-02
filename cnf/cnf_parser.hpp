#pragma once

#include <string>

#include "cnf/cnf_types.hpp"

bool load_cnf(const std::string &path, CNF &cnf, int &variables, int &clauses);
