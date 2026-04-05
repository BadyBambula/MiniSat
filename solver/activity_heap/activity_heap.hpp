#include <iostream>
#include <stdexcept>
#include <utility>
#include <vector>

class VarActivityHeap {
private:
    std::vector<int> heap;                     // heap vector
    std::vector<int> pos;                      // map telling us on which position is given variable (var -> index)
    const std::vector<double> *acts = nullptr; // activities of given solver's variables

    int parent(int idx) const { return (idx - 1) / 2; }
    int left(int idx) const { return (2 * idx) + 1; }
    int right(int idx) const { return (2 * idx) + 2; }

    bool better(int a, int b) const {

        // First we order by activity
        if ((*acts)[a] != (*acts)[b])
            return (*acts)[a] > (*acts)[b];

        // Then by the number of the variable
        return a > b;
    }

    void swap_nodes(int i, int j) {
        std::swap(heap[i], heap[j]);

        pos[heap[i]] = i;
        pos[heap[j]] = j;
    }

    /// @brief bubbles the element on given index up in the tree to it's right position
    /// @param idx index of the element
    void heapify_up(int idx) {
        while (idx > 0) {
            int p = parent(idx);
            if (!better(heap[idx], heap[p]))
                break;

            swap_nodes(idx, p);
            idx = p;
        }
    }

    void heapify_down(int idx) {
        while (true) {
            int l = left(idx);
            int r = right(idx);
            int best = idx;
            int n = heap.size();

            if (l < n && better(heap[l], heap[best]))
                best = l;
            if (r < n && better(heap[r], heap[best]))
                best = r;

            if (best == idx)
                break;

            swap_nodes(idx, best);
            idx = best;
        }
    }

public:
    VarActivityHeap() = default;

    VarActivityHeap(const std::vector<double> *activity_ref, int vars) { reset(activity_ref, vars); }

    bool empty() const { return heap.empty(); }

    bool contains(int var) const {
        return var >= 0 &&
               var < pos.size() &&
               pos[var] != -1;
    }

    int top() const {
        if (empty())
            throw std::runtime_error("Heap is empty.");

        return heap.front();
    }

    void reset(const std::vector<double> *activity_ref, int vars) {
        acts = activity_ref;
        heap.clear();
        heap.reserve(vars);
        pos.assign(vars + 1, -1);

        for (int v = 1; v <= vars; v++) {
            pos[v] = heap.size();
            heap.push_back(v);
        }

        for (int i = heap.size() / 2 - 1; i >= 0; i--)
            heapify_down(i);
    }

    void increase(int var) {
        if (!contains(var))
            return;

        heapify_up(pos[var]);
    }

    void insert(int var) {
        if (contains(var))
            return;

        pos[var] = heap.size();
        heap.push_back(var);

        heapify_up(pos[var]);
    }

    void remove(int var) {
        if (!contains(var))
            return;

        int idx = pos[var];
        int last = heap.size() - 1;

        swap_nodes(idx, last);

        pos[heap.back()] = -1;
        heap.pop_back();

        if (idx < heap.size()) {
            heapify_down(idx);
            heapify_up(idx);
        }
    }
};
