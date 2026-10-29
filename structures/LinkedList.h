#pragma once
/**
 * @file LinkedList.h
 * @brief Manual Implementation of a Generic Singly Linked List.
 * 
 * Fundamentals of Data Structures (FDS) Specification:
 * - Data Structure: Singly Linked List with Head and Tail pointers.
 * - Node Structure: Contains element payload and a Next pointer.
 * - Purpose in Project: Primary storage and management for emergency relief inventory.
 * - Dynamic Node Allocation: Every resource entry is maintained in dynamic memory.
 * 
 * Time Complexities:
 * - append(val):      O(1) (leveraging Tail pointer)
 * - prepend(val):     O(1) (leveraging Head pointer)
 * - find(predicate):  O(N) (linear traversal)
 * - remove(predicate):O(N) (pointer rewiring and deallocation)
 * - update(predicate):O(N) (linear traversal and in-place mutation)
 * - size():           O(1) (cached counter)
 * - clear():          O(N) (deallocates each node)
 * - toVector():       O(N) (iterative collection)
 * 
 * Space Complexity: O(N) where N is the number of nodes.
 */

#include <stdexcept>
#include <vector>
#include <functional>
#include <utility>

namespace structures {

template <typename T>
class LinkedList {
public:
    struct Node {
        T data;
        Node* next;
        explicit Node(const T& val) : data(val), next(nullptr) {}
        explicit Node(T&& val) : data(std::move(val)), next(nullptr) {}
    };

private:
    Node* head;
    Node* tail;
    size_t count;

public:
    LinkedList() : head(nullptr), tail(nullptr), count(0) {}

    ~LinkedList() {
        clear();
    }

    // Copy Constructor (Deep Copy)
    LinkedList(const LinkedList& other) : head(nullptr), tail(nullptr), count(0) {
        Node* current = other.head;
        while (current != nullptr) {
            append(current->data);
            current = current->next;
        }
    }

    // Copy Assignment Operator
    LinkedList& operator=(const LinkedList& other) {
        if (this != &other) {
            clear();
            Node* current = other.head;
            while (current != nullptr) {
                append(current->data);
                current = current->next;
            }
        }
        return *this;
    }

    // Move Constructor
    LinkedList(LinkedList&& other) noexcept 
        : head(other.head), tail(other.tail), count(other.count) {
        other.head = nullptr;
        other.tail = nullptr;
        other.count = 0;
    }

    // Move Assignment Operator
    LinkedList& operator=(LinkedList&& other) noexcept {
        if (this != &other) {
            clear();
            head = other.head;
            tail = other.tail;
            count = other.count;
            other.head = nullptr;
            other.tail = nullptr;
            other.count = 0;
        }
        return *this;
    }

    /**
     * @brief Appends a new element at the end of the list in O(1) time.
     */
    void append(const T& val) {
        Node* newNode = new Node(val);
        if (tail == nullptr) {
            head = tail = newNode;
        } else {
            tail->next = newNode;
            tail = newNode;
        }
        ++count;
    }

    /**
     * @brief Appends a new element using move semantics in O(1) time.
     */
    void append(T&& val) {
        Node* newNode = new Node(std::move(val));
        if (tail == nullptr) {
            head = tail = newNode;
        } else {
            tail->next = newNode;
            tail = newNode;
        }
        ++count;
    }

    /**
     * @brief Prepends a new element at the beginning of the list in O(1) time.
     */
    void prepend(const T& val) {
        Node* newNode = new Node(val);
        newNode->next = head;
        head = newNode;
        if (tail == nullptr) {
            tail = head;
        }
        ++count;
    }

    /**
     * @brief Finds the first element satisfying a predicate.
     * @return Pointer to element if found, or nullptr.
     * Time Complexity: O(N)
     */
    template <typename Predicate>
    T* find(Predicate predicate) {
        Node* current = head;
        while (current != nullptr) {
            if (predicate(current->data)) {
                return &(current->data);
            }
            current = current->next;
        }
        return nullptr;
    }

    template <typename Predicate>
    const T* find(Predicate predicate) const {
        Node* current = head;
        while (current != nullptr) {
            if (predicate(current->data)) {
                return &(current->data);
            }
            current = current->next;
        }
        return nullptr;
    }

    /**
     * @brief Finds and modifies the first element satisfying predicate.
     * Time Complexity: O(N)
     */
    template <typename Predicate, typename Updater>
    bool update(Predicate predicate, Updater updater) {
        T* item = find(predicate);
        if (item != nullptr) {
            updater(*item);
            return true;
        }
        return false;
    }

    /**
     * @brief Removes the first element satisfying predicate.
     * Time Complexity: O(N)
     */
    template <typename Predicate>
    bool remove(Predicate predicate) {
        if (head == nullptr) return false;

        if (predicate(head->data)) {
            Node* temp = head;
            head = head->next;
            if (head == nullptr) {
                tail = nullptr;
            }
            delete temp;
            --count;
            return true;
        }

        Node* prev = head;
        Node* current = head->next;
        while (current != nullptr) {
            if (predicate(current->data)) {
                prev->next = current->next;
                if (current == tail) {
                    tail = prev;
                }
                delete current;
                --count;
                return true;
            }
            prev = current;
            current = current->next;
        }
        return false;
    }

    /**
     * @brief Returns total number of nodes.
     * Time Complexity: O(1)
     */
    size_t size() const {
        return count;
    }

    /**
     * @brief Returns true if list has no nodes.
     * Time Complexity: O(1)
     */
    bool isEmpty() const {
        return count == 0;
    }

    /**
     * @brief Deallocates all nodes in list.
     * Time Complexity: O(N)
     */
    void clear() {
        while (head != nullptr) {
            Node* temp = head;
            head = head->next;
            delete temp;
        }
        tail = nullptr;
        count = 0;
    }

    /**
     * @brief Returns elements in order as a std::vector.
     * Time Complexity: O(N)
     */
    std::vector<T> toVector() const {
        std::vector<T> result;
        result.reserve(count);
        Node* current = head;
        while (current != nullptr) {
            result.push_back(current->data);
            current = current->next;
        }
        return result;
    }
};

} // namespace structures
