#include "cnf/cnf_parser.hpp"

#include <fstream>
#include <iostream>

#include "utils/text_utils.hpp"

bool load_cnf(const std::string &path, CNF &cnf, int &variables, int &clauses)
{
    std::ifstream infile(path);
    if (!infile.is_open())
    {
        std::cerr << "Cannot open CNF file: " << path << "\n";
        return false;
    }

    std::string line;

    // Skip comments and empty lines until header.
    while (std::getline(infile, line))
    {
        if (line.empty())
            continue;
        if (line[0] != 'c')
            break;
    }

    std::vector<std::string> tokens = split_whitespace(line);
    if (tokens.size() < 4)
    {
        std::cerr << "Invalid CNF header: expected 'p cnf <variables> <clauses>'.\n"
                  << "Line: " << line << "\n";
        return false;
    }

    try
    {
        variables = std::stoi(tokens[2]);
        clauses = std::stoi(tokens[3]);
    }
    catch (const std::invalid_argument &e)
    {
        std::cerr << "Invalid CNF header: expected 'p cnf <variables> <clauses>'.\n"
                  << "Details: " << e.what() << "\n"
                  << "Line: " << line << "\n";
        return false;
    }
    catch (const std::out_of_range &e)
    {
        std::cerr << "CNF header contains a number outside int range.\n"
                  << "Details: " << e.what() << "\n"
                  << "Line: " << line << "\n";
        return false;
    }

    while (std::getline(infile, line))
    {
        std::vector<std::string> literals = split_whitespace(line);
        if (literals.empty())
            continue;

        Clause clause;
        for (size_t i = 0; i < literals.size() && literals[i] != "0"; ++i)
        {
            const std::string &token = literals[i];
            int var;
            try
            {
                var = std::stoi(token);
            }
            catch (const std::invalid_argument &e)
            {
                std::cerr << "Invalid literal token in clause line: '" << token << "'.\n"
                          << "Details: " << e.what() << "\n";
                return false;
            }
            catch (const std::out_of_range &e)
            {
                std::cerr << "Literal token is outside int range: '" << token << "'.\n"
                          << "Details: " << e.what() << "\n";
                return false;
            }

            int var_num = std::abs(var);
            Lit lit = var > 0 ? 2*var_num : 2*var_num + 1;
            clause.push_back(lit);
        }
        cnf.push_back(clause);
    }

    return true;
}
