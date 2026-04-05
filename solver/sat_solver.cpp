#include "solver/sat_solver.hpp"
#include "solver/activity_heap/activity_heap.hpp"

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <queue>
#include <utility>

class SATSolver {
public:
    //////////////////////////////////////////////////////////////////////////////////////////
    //
    // === Constructor ===
    //

    explicit SATSolver(CNF clauses, int num_vars)
        : vars(num_vars),
          clauses(clauses),
          watches(2 * (num_vars + 1)),
          var_activity(num_vars + 1, 0.0),
          var_q(&var_activity, num_vars),
          var_inc(1.0),
          var_decay(0.95),
          assigns(num_vars + 1, -1),
          level(num_vars + 1, 0),
          tries(num_vars + 1, 0) {
        for (Clause &cl : this->clauses) {
            watches[neg(cl[0])].push_back(&cl);
            if (cl.size() > 1)
                watches[neg(cl[1])].push_back(&cl);
            else
                enqueue(cl[0]);
        }
    }

    SATSolver(const SATSolver &) = delete;
    SATSolver &operator=(const SATSolver &) = delete;

    //////////////////////////////////////////////////////////////////////////////////////////
    //
    // === Solve ===
    //

    SolveResult solve() {
        bool sat = search();

        SolveResult result;
        result.satisfiable = sat;
        result.assignment.assign(this->vars + 1, -1);

        for (int v = 1; v <= this->vars; v++)
            result.assignment[v] = assigns[v];

        return result;
    }

private:
    /////////////////////////////////////////////////////////
    //
    // === Global data ===
    //

    int vars;    // number of variables in the formula
    CNF clauses; // clauses in the formula

    std::vector<std::vector<Clause *>> watches; // watches[l] is vector of clauses,
                                                // in which neg(l) is watched literal

    std::queue<int> prop_q; // a queue of literals, we assigned to 1 and that wait to be
                            // addressed in propagation

    std::vector<double> var_activity; // var_activity[v] is the activity of variable v
    VarActivityHeap var_q;            // variable queue
    double var_inc;                   // the increment with which we increment the activities of vars in conflicts
    double var_decay;                 // the decay we use to calculate var_inc

    std::vector<int> assigns; // assings[v] is the assignment of variable v
    std::vector<int> level;   // level[v] is the level of variable v
    std::vector<int> tries;   // tries[v] is the number of assignments we tried for variable v in the current branch

    std::vector<Lit> trail;             // stack containing the literals assigned to 1
    std::vector<std::size_t> trail_lim; // stack of indices to trail, on these indices are decision variables

    /////////////////////////////////////////////////////////
    //
    // === Functions for literals ===
    //

    int value(Lit l) {
        int assigned = assigns[var(l)];
        if (assigned == -1) return -1;
        return sgn(l) ? assigned : neg(assigned);
    }

    int var(Lit l) { return l / 2; }
    int neg(Lit l) { return l ^ 1; }
    bool sgn(Lit l) { return (l & 1) == 0; }

    /////////////////////////////////////////////////////////
    //
    // === Search function ===
    //

    bool search() {
        while (true) {
            if (!propagate()) {
                if (!backtrack()) {
                    return false;
                }
                continue;
            }
            if (all_assigned()) {
                return true;
            }
            assume(pick_decision_literal());
        }
    }

    bool all_assigned() { return trail.size() == vars; }

    /////////////////////////////////////////////////////////
    //
    // === Assume & Enqueue ===
    //

    bool assume(Lit p) {
        trail_lim.push_back(trail.size());
        return enqueue(p);
    }

    bool enqueue(Lit p) {
        int val = value(p);

        // Already assigned with value
        if (val == 1) return true;
        if (val == 0) return false;

        // We need to assign
        int v = var(p);
        assigns[v] = sgn(p) ? 1 : 0;
        level[v] = trail_lim.size();
        tries[v]++;
        var_q.remove(v); // remove from var_order so we don't assume it further down

        prop_q.push(p);
        trail.push_back(p);

        return true;
    }

    /////////////////////////////////////////////////////////
    //
    // === Propagation ===
    //

    /// @brief  While prop_q is not empty, we pop literals
    ///         and propagate clauses further down the tree,
    ///         if propagation fails, we need to backtrack
    /// @return true, if propagation was successful, false otherwise
    bool propagate() {
        while (!prop_q.empty()) {
            Lit p = prop_q.front();
            prop_q.pop();

            // Clauses we need to address,
            // also we remove all elements from watches[p]
            std::vector<Clause *> tmp = std::move(watches[p]);

            for (size_t i = 0; i < tmp.size(); ++i) {
                // While the propagation is successful,
                // we continue with next clause from tmp
                if (propagate_clause(*tmp[i], p)) continue;

                // We are in conflict, so we have to push the unhandled
                // clauses back to watches[p]
                for (size_t k = i + 1; k < tmp.size(); ++k)
                    watches[p].push_back(tmp[k]);

                // Increase the activitiy of all variables in the clause
                bump_clause(*tmp[i]);

                // Reseting queue
                prop_q = std::queue<int>();

                // Signal conflict to main loop, we need to backtrack
                return false;
            }
        }
        return true;
    }

    bool propagate_clause(Clause &cl, Lit p) {
        // In case it is a unit clause
        if (cl.size() == 1) {
            watches[p].push_back(&cl);
            return enqueue(cl[0]);
        }

        // We ensure that the negation of the literal
        // is on index 1
        if (cl[1] != neg(p))
            std::swap(cl[0], cl[1]);

        // If on first index is a TRUE literal, we
        // don't have to do anything since the clause is
        // still true, just push the clause back to watches[p]
        if (value(cl[0]) == 1) {
            watches[p].push_back(&cl);
            return true;
        }

        // We want to find a literal further in the clause that we could
        // swap with c[1], so that we have a NON-FALSE
        // literal on the first index
        for (size_t i = 2; i < cl.size(); ++i) {
            if (value(cl[i]) == 0) continue;

            // We find it, so we swap and push it to watches[neg(c[1])]
            std::swap(cl[1], cl[i]);
            watches[neg(cl[1])].push_back(&cl);

            // Since we were successful with propagation, we return true
            return true;
        }

        // If value(c[0]) is FALSE, than we correctly detect
        // conflict in enqueue, since all the literals in
        // this clause are false. Else enqueue returns true
        watches[p].push_back(&cl);
        return enqueue(cl[0]);
    }

    /////////////////////////////////////////////////////////
    //
    // === Backtracking ===
    //

    /// @brief Cancels the assignment of the last assigned literal
    void undo_one() {
        Lit p = trail.back();
        trail.pop_back();
        int v = var(p);
        assigns[v] = -1;
        level[v] = 0;
        tries[v] = 0;
        var_q.insert(v);
    }

    bool backtrack() {
        while (!trail_lim.empty()) {
            // Pop all non-decisive variables from trail
            while (trail.size() > trail_lim.back() + 1)
                undo_one();

            // This is the decision variable
            Lit p = trail.back();
            trail.pop_back();
            int decision_var = var(p);

            // Reseting the assignment of the decision variable
            assigns[decision_var] = -1;

            // If we have a try left, we enqueue the negation
            // of p, this will always return true since we just
            // reseted the assignment of the decision variable
            if (tries[decision_var] < 2) return enqueue(neg(p));

            // Else we tried both assignments, we have to continue
            // with backtracking to the next decision variable
            trail_lim.pop_back();
            level[decision_var] = 0;
            tries[decision_var] = 0;

            var_q.insert(decision_var);
        }
        return false;
    }

    /////////////////////////////////////////////////////////
    //
    // === Picking decision literal ===
    //

    Lit pick_decision_literal() {
        while (!var_q.empty()) {
            int v = var_q.top();
            if (assigns[v] == -1)
                return 2 * v;

            // In case the literal is assigned, we want to remove it
            var_q.remove(v);
        }

        abort();
    }

    /////////////////////////////////////////////////////////
    //
    // === Activity handling functions ===
    //

    void rescale_activity_if_needed() {
        if (var_inc < 1e100)
            return;

        double factor = 1e-100;
        for (int v = 1; v <= vars; ++v)
            var_activity[v] *= factor;

        var_inc *= factor;
    }

    void bump_clause(const Clause &cl) {
        for (Lit l : cl)
            bump_variable(var(l));

        var_inc /= var_decay;
    }

    void bump_variable(int v) {
        var_activity[v] += var_inc;
        var_q.increase(v);
        rescale_activity_if_needed();
    }
};

SolveResult solve_sat(const CNF &cnf, int num_variables) {
    SATSolver solver(cnf, num_variables);
    return solver.solve();
}
