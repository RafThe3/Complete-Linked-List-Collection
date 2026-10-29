#pragma once // Prevents this header from being included more than once.
#include <sstream> // Provides stringstream for building a string representation.
#include <string> // Provides std::string.
#include <cstddef> // Provides std::size_t.
#include <limits> // Provides numeric_limits for the NOT_FOUND sentinel.
#include "exceptions.hpp" // Defines IndexOutOfBounds.

using std::size_t;
using std::stringstream;
using std::string;

// A singly linked-list collection that stores values of type T.
template <typename T>
class Collection
{
    public:

    // Constructor / Destructor
    Collection();
    ~Collection();

    // Copy constructor
    Collection(const Collection<T>& source);

    // List
    void insert(const T& data, size_t index);
    void push_back(const T& data);
    void pop_back(T& data);
    void removeValue(const T& data);
    void removeAt(size_t index);
    size_t search(const T& data) const;
    T at(size_t index) const;

    // Stack
    void push_front(const T& data);
    void pop_front(T& out);
    T top() const;

    // Queue
    void enqueue(const T& data);
    void dequeue(T& out);
    T front() const;
    T back() const;

    // Sorting
    void InsertionSort();
    void QuickSort(size_t low, size_t high);
    void MergeSort();

    // Global
    bool isEmpty() const;
    void clear();
    size_t getSize() const;
    string toString() const;

    private:

    // Value returned by search() when no matching element is found.
    static constexpr size_t NOT_FOUND = std::numeric_limits<size_t>::max();

    // Each node stores one value and a pointer to the next node.
    struct Node
    {
        T data;
        Node* next;

        // Initialize the stored value and mark this as the last node for now.
        Node(const T& data) : data(data), next(nullptr) {}
    };

    Node* head; // First node; nullptr means the list is empty.
    size_t size; // Number of nodes currently in the collection.

    // Helpers for locating nodes without changing the collection.
    Node* traverseToTail() const;
    Node* traverseToIndex(size_t index) const;
};

// Default construction creates an empty collection.
template <typename T>
Collection<T>::Collection() : head(nullptr), size(0) {}

// Destroying a collection must release all nodes it owns.
template <typename T>
Collection<T>::~Collection()
{
    clear();
}

// ------------------------Constructors / Destructors---------------------------

// Deep-copy constructor: create independent nodes containing the source values.
template <typename T>
Collection<T>::Collection(const Collection<T>& source) : head(nullptr), size(0)
{
    // Use a pointer to walk through the original list.
    Node* sourceNode = source.head;

    // Copy the first node separately so the destination head can be set.
    if (sourceNode != nullptr)
    {
        Node* newNode = new Node(sourceNode->data);
        head = newNode; // The first copied node becomes the destination head.
        sourceNode = sourceNode->next; // Continue to the next source node.
        size++; // Count the node that was copied.
    }

    // Keep track of the last copied node so appending is straightforward.
    Node* prevNode = head;

    // Copy every remaining source node.
    while (sourceNode != nullptr)
    {
        Node* newNode = new Node(sourceNode->data);
        prevNode->next = newNode; // Link the copied list to the new node.
        prevNode = newNode; // Move the destination tail forward.
        sourceNode = sourceNode->next; // Move through the source list.
        size++; // Keep the copied collection's size accurate.
    }
}

// -----------------------------------------------------------------------

// ---------------------------------List---------------------------------

// Insert data at index; index == size means append to the end.
template <typename T>
inline void Collection<T>::insert(const T& data, size_t index)
{
    // Validate that index is non-negative and less than or equal to size
    if (index > size)
    {
        throw IndexOutOfBounds();
    }

    // Create new node and initialize data and next pointer
    // Allocate a new node to hold the inserted value.
    Node* newNode = new Node(data);

    if (index == 0)
    {
        // Insert on first node
        newNode->next = head; // Preserve the existing list after the new node.
        head = newNode; // Make the new node the first node.
    }
    else
    {
        // Traversal node
        // Find the node immediately before the insertion position.
        // Locate the node immediately before the one being removed.
        Node* currentNode = traverseToIndex(index - 1);

        // Insert data
        newNode->next = currentNode->next; // Keep the rest of the list connected.
        currentNode->next = newNode; // Insert the new node after currentNode.
    }

    // Increase list size
    size++;
}

// Add a value at the end by inserting at the current size.
template <typename T>
inline void Collection<T>::push_back(const T& data)
{
    insert(data, size);
}

// Remove the last node and copy its value into out.
template <typename T>
inline void Collection<T>::pop_back(T& out)
{
    // Check if list is not empty
    if (isEmpty())
    {
        throw EmptyList();
    }

    // currentNode moves through the list; prevNode follows one node behind.
    Node* currentNode = head;
    Node* prevNode = nullptr;

    // Special case: a one-node list has no predecessor.
    if (currentNode->next == nullptr)
    {
        out = currentNode->data;
        head = nullptr;
        delete currentNode;
    }
    else
    {
        // Advance until currentNode is the tail and prevNode is its predecessor.
        for (size_t i = 0; i < size - 1; i++)
        {
            prevNode = currentNode;
            currentNode = currentNode->next;
        }

        out = currentNode->data; // Preserve the value before deleting the node.
        prevNode->next = nullptr; // The predecessor becomes the new tail.
        delete currentNode; // Release the removed tail node.
    }

    size--;
}

// Find the value's index and remove it if it exists.
template <typename T>
inline void Collection<T>::removeValue(const T& data)
{
    size_t index = search(data);

    if (index != NOT_FOUND)
    {
        removeAt(index);
    }
}

// Remove the node at a zero-based index.
template <typename T>
inline void Collection<T>::removeAt(size_t index)
{
    // Validate that index is non-negative and less than size
    if (index >= size)
    {
        // Throw exception
        throw IndexOutOfBounds();
    }

    // Create a node for deleting
    // Holds the node that will be unlinked and deleted.
    Node* deleteNode;
    
    if (index == 0)
    {
        // Delete the first node
        deleteNode = head; // Save the old head.
        head = head->next; // Move head to the next node.
        delete deleteNode; // Delete the old head.
    }
    else
    {
        // Traversal node
        Node* currentNode = traverseToIndex(index - 1);

        // Delete node
        deleteNode = currentNode->next; // Identify the target node.
        currentNode->next = deleteNode->next; // Bypass the target node.
        delete deleteNode; // Release the target node's memory.
    }

    // Decrease list size
    size--;
}

// Return the first matching index, or NOT_FOUND if there is no match.
template <typename T>
inline size_t Collection<T>::search(const T& data) const
{
    // Traversal node
    // Begin at the head and inspect nodes one by one.
    Node* currentNode = head;

    // Index of the node currently being inspected.
    size_t index = 0;

    // While currentNode exists
    while (currentNode != nullptr)
    {
        // If data matches data
        if (currentNode->data == data)
        {
            return index;
        }

        // Traverse list
        currentNode = currentNode->next;

        // Increment index
        index++;
    }

    // Return -1 if data is not found
    return NOT_FOUND;
}

// Return the value at index after validating the requested position.
template <typename T>
inline T Collection<T>::at(size_t index) const
{
    // Validate that index is non-negative and less than size
    if (index >= size)
    {
        throw IndexOutOfBounds();
    }

    // Traversal node
    Node* currentNode = traverseToIndex(index);

    // Return data at index
    return currentNode->data;
}

// -----------------------------------------------------------------------

// -------------------------------Stack-----------------------------------

// Stack-style insertion: add a value at the beginning.
template <typename T>
inline void Collection<T>::push_front(const T& data)
{
    insert(data, 0);
}

// Copy the first value into out, then remove the first node.
template <typename T>
inline void Collection<T>::pop_front(T& out)
{
    if (head == nullptr)
    {
        throw EmptyList();
    }

    // Return data at index
    out = head->data;
    removeAt(0);
}

// Return the first value without removing it.
template <typename T>
inline T Collection<T>::top() const
{
    if (head == nullptr)
    {
        throw EmptyList();
    }

    return head->data;
}

// -----------------------------------------------------------------------

// --------------------------------Queue----------------------------------

// Queue-style insertion: add a value at the back.
template <typename T>
inline void Collection<T>::enqueue(const T& data)
{
    insert(data, size);
}

// Queue-style removal: remove the value at the front.
template <typename T>
inline void Collection<T>::dequeue(T& out)
{
    pop_front(out);
}

// Return the queue's first value without removing it.
template <typename T>
inline T Collection<T>::front() const
{
    return top();
}

// Find and return the final value in the list.
template <typename T>
inline T Collection<T>::back() const
{
    Node* currentNode = traverseToTail();
    return currentNode->data;
}

// -----------------------------------------------------------------------

// -------------------------------Sorting---------------------------------

template <typename T>
void Collection<T>::InsertionSort()
{
    // FIXME: PRETTY MUCH EVERYTHING
    
    Node* currentNode = head;

    // Start at index 1 because the first element is already
    // considered sorted.
    for (size_t i = 1; i < size; i++)
    {
        Node* prevNode = currentNode;
        currentNode = currentNode->next;

        // Store the element that needs to be inserted.
        T temp = currentNode->data;

        // Start with the element immediately before temp.
        size_t j = i - 1;

        // Shift larger elements one position to the right
        // until the correct position for temp is found.
        while (prevNode->data > temp)
        {
            currentNode->data = prevNode->data;
            j--;
        }

        // Insert temp into its correct position.
        currentNode->data = temp;
    }
}

template <typename T>
void QuickSort(size_t low, size_t high)
{
    // Base case: if there is one or zero elements,
    // the portion of the array is already sorted.
    if (low >= high)
    {
        return;
    }

    // Choose the last element as the pivot.
    T pivot = array[high];

    // i keeps track of the position where the next
    // element smaller than the pivot should be placed.
    size_t i = low - 1;

    // Check each element before the pivot.
    for (size_t j = low; j < high; j++)
    {
        // If the current element is smaller than the pivot,
        // move it into the section containing smaller elements.
        if (array[j] < pivot)
        {
            i++;
            swap(array[i], array[j]);
        }
    }

    // Place the pivot between the smaller and larger elements.
    swap(array[i + 1], array[high]);

    // The pivot is now in its final sorted position.
    size_t pivotIndex = i + 1;

    // Recursively sort the elements to the left of the pivot.
    QuickSort(array, low, pivotIndex - 1);

    // Recursively sort the elements to the right of the pivot.
    QuickSort(array, pivotIndex + 1, high);
}

template <typename T>
void MergeSort()
{
    // Base case:
    // An array with 0 or 1 elements is already sorted.
    if (array.size() <= 1)
    {
        return;
    }

    // Temporary vectors used to store the two halves.
    vector<T> left, right;

    // Copy the first half of the array into left.
    for (size_t i = 0; i < array.size() / 2; i++)
    {
        left.push_back(array[i]);
    }

    // Copy the second half of the array into right.
    for (size_t i = array.size() / 2; i < array.size(); i++)
    {
        right.push_back(array[i]);
    }

    // Recursively sort both halves.
    MergeSort(left);
    MergeSort(right);

    // Merge the two sorted halves back into the original array.

    // Index for the left vector.
    size_t leftIndex = 0;

    // Index for the right vector.
    size_t rightIndex = 0;

    // Index for the original array.
    size_t arrayIndex = 0;

    // Continue while both halves still contain elements.
    while (leftIndex < left.size() && rightIndex < right.size())
    {
        // Take the smaller element from the two halves.
        // <= handles equal elements by taking from the left.
        if (left[leftIndex] <= right[rightIndex])
        {
            array[arrayIndex] = left[leftIndex];
            leftIndex++;
        }
        else
        {
            array[arrayIndex] = right[rightIndex];
            rightIndex++;
        }

        // Move to the next position in the original array.
        arrayIndex++;
    }

    // If elements remain in left, copy them into the array.
    while (leftIndex < left.size())
    {
        array[arrayIndex] = left[leftIndex];
        arrayIndex++;
        leftIndex++;
    }

    // If elements remain in right, copy them into the array.
    while (rightIndex < right.size())
    {
        array[arrayIndex] = right[rightIndex];
        arrayIndex++;
        rightIndex++;
    }
}

// -----------------------------------------------------------------------

// -------------------------------Global----------------------------------

// Locate the final node; an empty collection has no tail to return.
template <typename T>
inline typename Collection<T>::Node* Collection<T>::traverseToTail() const
{
    if (isEmpty())
    {
        throw IndexOutOfBounds();
    }

    // Starting at the head, follow size - 1 links to reach the tail.
    Node* currentNode = head;

    for (size_t i = 0; i < size - 1; i++)
    {
        currentNode = currentNode->next;
    }

    return currentNode;
}

// Locate the node at index; reject an index outside the valid range.
template <typename T>
inline typename Collection<T>::Node* Collection<T>::traverseToIndex(size_t index) const
{
    if (index >= size)
    {
        throw IndexOutOfBounds();
    }

    Node* currentNode = head;

    // Follow next pointers index times to reach the requested list node.
    for (size_t i = 0; i < index; i++)
    {
        currentNode = currentNode->next;
    }

    return currentNode;
}

// Report whether the collection contains zero nodes.
template <typename T>
inline bool Collection<T>::isEmpty() const
{
    return size == 0;
}

// Delete all nodes and reset the collection to an empty state.
template <typename T>
inline void Collection<T>::clear()
{
    // Traversal node
    Node* currentNode = head;

    // While currentNode exists, delete all data in list
    while (currentNode != nullptr)
    {
        Node* temp = currentNode->next; // Save the next node before deleting this one.
        delete currentNode; // Release the current node.
        currentNode = temp; // Continue with the saved next node.
    }

    // Initialize head and size
    head = nullptr;
    size = 0;
}

// Return the stored node count.
template <typename T>
inline size_t Collection<T>::getSize() const
{
    return size;
}

// Convert the list into text, placing arrows between adjacent values.
template <typename T>
inline string Collection<T>::toString() const
{
    stringstream ss; // Accumulate the output efficiently.

    Node* currentNode = head; // Traverse from the first node.

    while (currentNode != nullptr)
    {
        ss << currentNode->data;

        // Add a separator only when another node follows this one.
        if (currentNode->next != nullptr)
        {
            ss << " -> ";
        }

        currentNode = currentNode->next;
    }

    return ss.str();
}

// -----------------------------------------------------------------------