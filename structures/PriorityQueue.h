#pragma once
/**
 * @file PriorityQueue.h
 * @brief Manual Implementation of a Generic Binary Max-Heap Priority Queue for Triage Management.
 * 
 * Fundamentals of Data Structures (FDS) Specification:
 * - Data Structure: Binary Max-Heap array representation.
 * - Discipline: Highest-priority element is always served first (Root at index 0).
 * - Priority Calculation: Higher numerical score = Higher triage urgency.
 * - Purpose in SIH Scenario: Evaluates casualty counts, hazard levels, and vulnerability
 *   to ensure highest-risk disaster zones (e.g., Zone C with 40 people) are triaged before lower-risk zones.
 * 
 * Time Complexities:
 * - insert(item, priority): O(log N) (percolate up)
 * - extractMax():          O(log N) (percolate down)
 * - peek():                O(1)
 * - isEmpty():             O(1)
 * - size():                O(1)
 * - clear():               O(1)
 * 
 * Space Complexity: O(N)
 */

#include <vector>
#include <stdexcept>
#include <utility>
#include <algorithm>

namespace structures {

template <typename T>
class PriorityQueue {
public:
    struct Element {
        T data;
        double priority; // Higher value = higher priority

        Element(const T& d, double p) : data(d), priority(p) {}
        Element(T&& d, double p) : data(std::move(d)), priority(p) {}
    };

private:
    std::vector<Element> heap;

    void siftUp(size_t index) {
        while (index > 0) {
            size_t parent = (index - 1) / 2;
            if (heap[index].priority > heap[parent].priority) {
                std::swap(heap[index], heap[parent]);
                index = parent;
            } else {
                break;
            }
        }
    }

    void siftDown(size_t index) {
        size_t size = heap.size();
        while (true) {
            size_t leftChild = 2 * index + 1;
            size_t rightChild = 2 * index + 2;
            size_t largest = index;

            if (leftChild < size && heap[leftChild].priority > heap[largest].priority) {
                largest = leftChild;
            }
            if (rightChild < size && heap[rightChild].priority > heap[largest].priority) {
                largest = rightChild;
            }

            if (largest != index) {
                std::swap(heap[index], heap[largest]);
                index = largest;
            } else {
                break;
            }
        }
    }

public:
    PriorityQueue() = default;

    /**
     * @brief Inserts an item with an associated triage priority score.
     * Time Complexity: O(log N)
     */
    void insert(const T& item, double priority) {
        heap.emplace_back(item, priority);
        siftUp(heap.size() - 1);
    }

    void insert(T&& item, double priority) {
        heap.emplace_back(std::move(item), priority);
        siftUp(heap.size() - 1);
    }

    /**
     * @brief Removes and returns the highest-priority item.
     * @throws std::runtime_error if priority queue is empty.
     * Time Complexity: O(log N)
     */
    T extractMax() {
        if (isEmpty()) {
            throw std::runtime_error("Cannot extract from an empty Priority Queue.");
        }
        T maxItem = std::move(heap[0].data);
        if (heap.size() == 1) {
            heap.pop_back();
        } else {
            heap[0] = std::move(heap.back());
            heap.pop_back();
            siftDown(0);
        }
        return maxItem;
    }

    /**
     * @brief Returns a reference to the highest-priority element without removing it.
     * Time Complexity: O(1)
     */
    const T& peek() const {
        if (isEmpty()) {
            throw std::runtime_error("Cannot peek into an empty Priority Queue.");
        }
        return heap[0].data;
    }

    /**
     * @brief Returns the priority score of the highest-priority element.
     */
    double peekPriority() const {
        if (isEmpty()) {
            throw std::runtime_error("Cannot peek priority of an empty Priority Queue.");
        }
        return heap[0].priority;
    }

    bool isEmpty() const {
        return heap.empty();
    }

    size_t size() const {
        return heap.size();
    }

    void clear() {
        heap.clear();
    }

    /**
     * @brief Returns all elements with their priority scores in descending priority order.
     * Time Complexity: O(N log N)
     */
    std::vector<std::pair<T, double>> toSortedVector() const {
        PriorityQueue<T> copy = *this;
        std::vector<std::pair<T, double>> result;
        result.reserve(copy.size());

        while (!copy.isEmpty()) {
            double p = copy.peekPriority();
            T item = copy.extractMax();
            result.emplace_back(std::move(item), p);
        }
        return result;
    }
};

} // namespace structures
