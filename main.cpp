#include "cnf/cnf_parser.hpp"
#include "solver/sat_solver.hpp"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cerr << "Bad usage of the program";
        return 1;
    }

    std::string path = argv[1];

    bool quiet = false;
    for (int i = 2; i < argc; i++)
    {
        std::string arg = argv[i];
        if (arg == "-q" || arg == "--quiet")
            quiet = true;
    }

    CNF cnf;
    int variables = 0;
    int clauses = 0;
    
    if (!load_cnf(path, cnf, variables, clauses))
        return 1;

    if (!quiet)
    {
        std::cout << "\n===== MINISAT ====="
                  << "\nSolving:\t" << path
                  << "\nVariables:\t" << variables
                  << "\nClauses:\t" << clauses
                  << std::endl;
    }

    auto start = std::chrono::high_resolution_clock::now();
    SolveResult result = solve_sat(cnf, variables);
    auto end = std::chrono::high_resolution_clock::now();

    bool sat = result.satisfiable;

    double seconds = std::chrono::duration<double>(end - start).count();
    std::cout << std::fixed << std::setprecision(6)
              << "Time needed:\t" << seconds << " s\n"
              << "Result:\t\t" << (sat ? "SAT" : "UNSAT") << std::endl;

    if (!quiet && sat)
    {
        std::cout << "Assignment:" << std::endl;
        for (size_t i = 1; i < result.assignment.size(); i++)
        {
            if (result.assignment[i] == 1)
                std::cout << "x" << i << "=T\n";
            else if (result.assignment[i] == 0)
                std::cout << "x" << i << "=F\n";
            else
                std::cout << "x" << i << "=U\n";
        }
    }

    return 0;
}
