#pragma once
/**
 * @file Sorting.h
 * @brief Manual Implementation of Generic MergeSort for Ranking Disaster Zones by Severity.
 * 
 * Fundamentals of Data Structures (FDS) Specification:
 * - Algorithm: MergeSort (Divide-and-Conquer).
 * - Stability: Preserves relative order of equally-scored disaster zones.
 * - Time Complexity: O(N log N) in best, worst, and average cases.
 * - Auxiliary Space Complexity: O(N) for merge scratch buffer.
 * - Purpose in SIH Scenario: Automatically ranks disaster zones (Zone A, B, C, D)
 *   according to casualty count and urgency score to optimize operational resource allocation.
 */

#include <vector>
#include <functional>

namespace structures {

class Sorting {
private:
    template <typename T, typename Compare>
    static void merge(std::vector<T>& arr, size_t left, size_t mid, size_t right, Compare comp) {
        size_t n1 = mid - left + 1;
        size_t n2 = right - mid;

        std::vector<T> L(arr.begin() + left, arr.begin() + left + n1);
        std::vector<T> R(arr.begin() + mid + 1, arr.begin() + mid + 1 + n2);

        size_t i = 0, j = 0, k = left;
        while (i < n1 && j < n2) {
            // comp(a, b) returns true if a should precede b
            if (comp(L[i], R[j])) {
                arr[k++] = std::move(L[i++]);
            } else {
                arr[k++] = std::move(R[j++]);
            }
        }

        while (i < n1) {
            arr[k++] = std::move(L[i++]);
        }
        while (j < n2) {
            arr[k++] = std::move(R[j++]);
        }
    }

    template <typename T, typename Compare>
    static void mergeSortHelper(std::vector<T>& arr, size_t left, size_t right, Compare comp) {
        if (left < right) {
            size_t mid = left + (right - left) / 2;
            mergeSortHelper(arr, left, mid, comp);
            mergeSortHelper(arr, mid + 1, right, comp);
            merge(arr, left, mid, right, comp);
        }
    }

public:
    /**
     * @brief Sorts a vector in-place using manual MergeSort.
     * @param arr Vector of elements to sort.
     * @param comp Comparator function returning true if first argument precedes second.
     * Time Complexity: O(N log N)
     * Space Complexity: O(N)
     */
    template <typename T, typename Compare>
    static void mergeSort(std::vector<T>& arr, Compare comp) {
        if (arr.size() <= 1) return;
        mergeSortHelper(arr, 0, arr.size() - 1, comp);
    }
};

} // namespace structures
