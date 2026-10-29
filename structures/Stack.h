#pragma once
/**
 * @file Stack.h
 * @brief Manual Implementation of a Generic Last-In First-Out (LIFO) Linked Stack.
 * 
 * Fundamentals of Data Structures (FDS) Specification:
 * - Data Structure: Singly Linked Stack with a Top pointer.
 * - Discipline: Last-In, First-Out (LIFO).
 * - Push: Inserts a new node at the Top in O(1) time.
 * - Pop: Removes and retrieves the node at the Top in O(1) time.
 * - Memory Management: Dynamic heap allocation per node with explicit deallocation.
 * 
 * Time Complexities:
 * - push(val): O(1)
 * - pop():      O(1)
 * - peek():     O(1)
 * - isEmpty():  O(1)
 * - size():     O(1)
 * - clear():    O(N)
 * - toVector(): O(N) (top-to-bottom / recent-to-oldest)
 * 
 * Space Complexity: O(N) where N is the number of elements pushed.
 */

#include <stdexcept>
#include <vector>
#include <utility>

namespace structures {

template <typename T>
class Stack {
private:
    struct Node {
        T data;
        Node* next;
        explicit Node(const T& val) : data(val), next(nullptr) {}
        explicit Node(T&& val) : data(std::move(val)), next(nullptr) {}
    };

    Node* topNode;
    size_t count;

public:
    Stack() : topNode(nullptr), count(0) {}

    ~Stack() {
        clear();
    }

    // Copy Constructor (Deep Copy)
    Stack(const Stack& other) : topNode(nullptr), count(0) {
        // Collect in bottom-to-top order then push
        std::vector<T> items = other.toVector(); // items[0] is top, items.back() is bottom
        for (auto it = items.rbegin(); it != items.rend(); ++it) {
            push(*it);
        }
    }

    // Copy Assignment Operator
    Stack& operator=(const Stack& other) {
        if (this != &other) {
            clear();
            std::vector<T> items = other.toVector();
            for (auto it = items.rbegin(); it != items.rend(); ++it) {
                push(*it);
            }
        }
        return *this;
    }

    // Move Constructor
    Stack(Stack&& other) noexcept 
        : topNode(other.topNode), count(other.count) {
        other.topNode = nullptr;
        other.count = 0;
    }

    // Move Assignment Operator
    Stack& operator=(Stack&& other) noexcept {
        if (this != &other) {
            clear();
            topNode = other.topNode;
            count = other.count;
            other.topNode = nullptr;
            other.count = 0;
        }
        return *this;
    }

    /**
     * @brief Pushes an item onto the top of the stack.
     * @param val Item to push.
     * Time Complexity: O(1)
     */
    void push(const T& val) {
        Node* newNode = new Node(val);
        newNode->next = topNode;
        topNode = newNode;
        ++count;
    }

    /**
     * @brief Pushes an item onto the top of the stack using move semantics.
     */
    void push(T&& val) {
        Node* newNode = new Node(std::move(val));
        newNode->next = topNode;
        topNode = newNode;
        ++count;
    }

    /**
     * @brief Removes and returns the top element of the stack.
     * @return T Popped element.
     * @throws std::runtime_error if stack is empty.
     * Time Complexity: O(1)
     */
    T pop() {
        if (isEmpty()) {
            throw std::runtime_error("Cannot pop from an empty Stack.");
        }
        Node* temp = topNode;
        T result = std::move(temp->data);
        topNode = topNode->next;
        delete temp;
        --count;
        return result;
    }

    /**
     * @brief Returns a reference to the top element without removing it.
     * @throws std::runtime_error if stack is empty.
     * Time Complexity: O(1)
     */
    const T& peek() const {
        if (isEmpty()) {
            throw std::runtime_error("Cannot peek into an empty Stack.");
        }
        return topNode->data;
    }

    /**
     * @brief Returns true if the stack contains no elements.
     * Time Complexity: O(1)
     */
    bool isEmpty() const {
        return count == 0;
    }

    /**
     * @brief Returns the total number of items on the stack.
     * Time Complexity: O(1)
     */
    size_t size() const {
        return count;
    }

    /**
     * @brief Deallocates all nodes and resets the stack.
     * Time Complexity: O(N)
     */
    void clear() {
        while (topNode != nullptr) {
            Node* temp = topNode;
            topNode = topNode->next;
            delete temp;
        }
        count = 0;
    }

    /**
     * @brief Exports elements in LIFO order (from top/most recent to bottom/oldest).
     * Time Complexity: O(N)
     */
    std::vector<T> toVector() const {
        std::vector<T> result;
        result.reserve(count);
        Node* current = topNode;
        while (current != nullptr) {
            result.push_back(current->data);
            current = current->next;
        }
        return result;
    }
};

} // namespace structures
