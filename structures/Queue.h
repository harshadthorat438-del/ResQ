#pragma once
/**
 * @file Queue.h
 * @brief Manual Implementation of a Generic First-In First-Out (FIFO) Linked Queue.
 * 
 * Fundamentals of Data Structures (FDS) Specification:
 * - Data Structure: Singly Linked Queue with Front and Rear pointers.
 * - Discipline: First-In, First-Out (FIFO).
 * - Enqueue: Appends to Rear pointer in O(1) time.
 * - Dequeue: Removes from Front pointer in O(1) time.
 * - Memory Management: Dynamic heap allocation per node with explicit deallocation.
 * 
 * Time Complexities:
 * - enqueue(val): O(1)
 * - dequeue():     O(1)
 * - peek():        O(1)
 * - isEmpty():     O(1)
 * - size():        O(1)
 * - clear():       O(N)
 * - toVector():    O(N)
 * 
 * Space Complexity: O(N) where N is the number of elements queued.
 */

#include <stdexcept>
#include <vector>
#include <utility>

namespace structures {

template <typename T>
class Queue {
private:
    struct Node {
        T data;
        Node* next;
        explicit Node(const T& val) : data(val), next(nullptr) {}
        explicit Node(T&& val) : data(std::move(val)), next(nullptr) {}
    };

    Node* frontNode;
    Node* rearNode;
    size_t count;

public:
    Queue() : frontNode(nullptr), rearNode(nullptr), count(0) {}

    ~Queue() {
        clear();
    }

    // Copy Constructor (Deep Copy)
    Queue(const Queue& other) : frontNode(nullptr), rearNode(nullptr), count(0) {
        Node* current = other.frontNode;
        while (current != nullptr) {
            enqueue(current->data);
            current = current->next;
        }
    }

    // Copy Assignment Operator
    Queue& operator=(const Queue& other) {
        if (this != &other) {
            clear();
            Node* current = other.frontNode;
            while (current != nullptr) {
                enqueue(current->data);
                current = current->next;
            }
        }
        return *this;
    }

    // Move Constructor
    Queue(Queue&& other) noexcept 
        : frontNode(other.frontNode), rearNode(other.rearNode), count(other.count) {
        other.frontNode = nullptr;
        other.rearNode = nullptr;
        other.count = 0;
    }

    // Move Assignment Operator
    Queue& operator=(Queue&& other) noexcept {
        if (this != &other) {
            clear();
            frontNode = other.frontNode;
            rearNode = other.rearNode;
            count = other.count;
            other.frontNode = nullptr;
            other.rearNode = nullptr;
            other.count = 0;
        }
        return *this;
    }

    /**
     * @brief Adds an element to the rear of the queue.
     * @param val Item to enqueue.
     * Time Complexity: O(1)
     */
    void enqueue(const T& val) {
        Node* newNode = new Node(val);
        if (rearNode == nullptr) {
            frontNode = rearNode = newNode;
        } else {
            rearNode->next = newNode;
            rearNode = newNode;
        }
        ++count;
    }

    /**
     * @brief Adds an element to the rear using move semantics.
     */
    void enqueue(T&& val) {
        Node* newNode = new Node(std::move(val));
        if (rearNode == nullptr) {
            frontNode = rearNode = newNode;
        } else {
            rearNode->next = newNode;
            rearNode = newNode;
        }
        ++count;
    }

    /**
     * @brief Removes and returns the element at the front of the queue.
     * @return T Dequeued element.
     * @throws std::runtime_error if queue is empty.
     * Time Complexity: O(1)
     */
    T dequeue() {
        if (isEmpty()) {
            throw std::runtime_error("Cannot dequeue from an empty Queue.");
        }
        Node* temp = frontNode;
        T result = std::move(temp->data);
        frontNode = frontNode->next;
        if (frontNode == nullptr) {
            rearNode = nullptr;
        }
        delete temp;
        --count;
        return result;
    }

    /**
     * @brief Returns a reference to the front element without removing it.
     * @throws std::runtime_error if queue is empty.
     * Time Complexity: O(1)
     */
    const T& peek() const {
        if (isEmpty()) {
            throw std::runtime_error("Cannot peek into an empty Queue.");
        }
        return frontNode->data;
    }

    /**
     * @brief Returns true if the queue contains no elements.
     * Time Complexity: O(1)
     */
    bool isEmpty() const {
        return count == 0;
    }

    /**
     * @brief Returns the total number of queued elements.
     * Time Complexity: O(1)
     */
    size_t size() const {
        return count;
    }

    /**
     * @brief Deallocates all nodes and resets the queue.
     * Time Complexity: O(N)
     */
    void clear() {
        while (frontNode != nullptr) {
            Node* temp = frontNode;
            frontNode = frontNode->next;
            delete temp;
        }
        rearNode = nullptr;
        count = 0;
    }

    /**
     * @brief Exports elements in FIFO order (from front to rear).
     * Time Complexity: O(N)
     */
    std::vector<T> toVector() const {
        std::vector<T> result;
        result.reserve(count);
        Node* current = frontNode;
        while (current != nullptr) {
            result.push_back(current->data);
            current = current->next;
        }
        return result;
    }
};

} // namespace structures
