#include "cnf/cnf_parser.hpp"
#include "solver/sat_solver.hpp"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>

void check_solution(const CNF& cnf, const std::vector<int>& assignment) {
    for (size_t i = 0; i < cnf.size(); i++) {
        const Clause& clause = cnf[i];

        bool satisfied = false;

        for (int lit : clause) {
            bool val = assignment[var(lit)];
            bool lit_true = sgn(lit) ? val : !val;
            if (lit_true) {
                satisfied = true;
                break;
            }
        }

        if (!satisfied) {
            std::cerr << "❌ Clause " << i << ": ";
            for (int lit : clause) {
                bool val = assignment[var(lit)];
                std::cerr << lit << "(" << val << ") ";
            }
            std::cerr << "\n";

            std::abort();
        }
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cerr << "Bad usage of the program";
        return 1;
    }

    std::string path = argv[1];
    CNF cnf;
    int variables = 0;
    int clauses = 0;

    // === Loading CNF ===
    auto parser_start = std::chrono::high_resolution_clock::now();
    if (!load_cnf(path, cnf, variables, clauses)) return 1;
    auto parser_end = std::chrono::high_resolution_clock::now();

    // === Solving CNF ===
    auto solver_start = std::chrono::high_resolution_clock::now();
    SolveResult result = solve_sat(cnf, variables);
    auto solver_end = std::chrono::high_resolution_clock::now();

    // === First line - satisfiability ===
    bool sat = result.satisfiable;
    std::cout << (sat ? "SAT" : "UNSAT") << std::endl;

    // === Second line - sorted trail ===
    if (sat) {
        size_t size = result.true_literals.size();
        std::cout << "[";
        for (size_t i = 0; i < result.true_literals.size(); ++i) {
            std::cout << encode_literal(result.true_literals[i]);
            if (i + 1 < result.true_literals.size())
                std::cout << ", ";
        }
        std::cout << "]";
        check_solution(cnf, result.assignment);
    }
    std::cout << std::endl;

    // === Third line - time needed for initialization ===
    double parser_seconds = std::chrono::duration<double>(parser_end - parser_start).count();
    std::cout << std::fixed << std::setprecision(6) << parser_seconds << std::endl;

    // === Fourth line - time needed for computing ===
    double solver_seconds = std::chrono::duration<double>(solver_end - solver_start).count();
    std::cout << std::fixed << std::setprecision(6) << solver_seconds << std::endl;

    // === Fifth line - number of unit propagations ===
    std::cout << result.unit_props << std::endl;

    // === Sixth line - number of decision variables ===
    std::cout << result.dec_vars << std::endl;

    return 0;
}
