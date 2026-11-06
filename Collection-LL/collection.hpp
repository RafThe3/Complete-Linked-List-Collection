#pragma once // Prevents this header from being included more than once.
#include <sstream> // Provides std::stringstream for building a std::string representation.
#include <string> // Provides std::string.
#include <cstddef> // Provides std::size_t.
#include <limits> // Provides numeric_limits for the NOT_FOUND sentinel.
#include <utility> // Provides std::swap().
#include <fstream> // Provides file input and output streams.
#include <iomanip> // Provides std::quoted() and std::setprecision().
#include <type_traits> // Provides compile-time type traits used to specialize file serialization.
#include <stdexcept> // Provides standard exception type declarations.
#include "exceptions.hpp" // Defines IndexOutOfBounds.

// A singly linked-list collection that stores values of type T.
template <typename T>
class Collection
{
    public:

    // Constructor / Destructor
    // Creates an empty collection.
    Collection();
    // Releases all nodes owned by the collection.
    ~Collection();

    // Copy constructor / copy assignment operator
    // Creates an independent deep copy of another collection.
    Collection(const Collection<T>& source);
    // Replaces this collection with a deep copy of another collection.
    Collection<T>& operator=(const Collection<T>& source);

    // Move constructor / move assignment operator
    // Takes ownership of another collection's nodes.
    Collection(Collection<T>&& source) noexcept;
    // Releases the current nodes and takes ownership of another collection's nodes.
    Collection<T>& operator=(Collection<T>&& source) noexcept;

    // Insertion
    // Inserts a value at the beginning; returns whether insertion succeeded.
    bool push_front(const T& data);
    // Inserts a value at the end; returns whether insertion succeeded.
    bool push_back(const T& data);
    // Inserts a value at an index from 0 through getSize(); returns false for an invalid index.
    bool insert(const T& data, std::size_t index);
    // Inserts a value in sorted position; returns false if the collection is not marked sorted.
    bool insertSorted(const T& data);

    // Deletion
    // Removes the first occurrence of a value; returns false if it is not found.
    bool remove(const T& data);
    // Removes the value at an index; returns false if the index is out of range.
    bool removeAt(std::size_t index);
    // Removes the first value and stores it in out; returns false if the collection is empty.
    bool pop_front(T& out);
    // Removes the last value and stores it in out; returns false if the collection is empty.
    bool pop_back(T& out);
    // Removes the first value without returning it; returns false if the collection is empty.
    bool pop_front();
    // Removes the last value without returning it; returns false if the collection is empty.
    bool pop_back();
    // Removes every value; returns true only if the collection contained at least one value.
    bool clear();

    // Replacement
    // Replaces the first occurrence of oldVal with newVal; returns false if oldVal is absent.
    bool replace(const T& newVal, const T& oldVal);
    // Replaces the value at index; returns false if the index is out of range.
    bool replaceAt(const T& data, std::size_t index);

    // Swapping
    // Swaps the first occurrences of two values; returns false if either value is absent.
    bool swap(const T& newVal, const T& oldVal);
    // Swaps the first occurrence of data with the value at index; returns false if either is invalid.
    bool swapAt(const T& data, std::size_t index);

    // Searching
    // Reports whether at least one occurrence of data is in the collection.
    bool contains(const T& data) const;
    // Returns a read-only reference to the value at index; throws if the index is invalid.
    const T& at(std::size_t index) const;
    // Returns the first matching index, or NOT_FOUND when data is absent.
    std::size_t search(const T& data) const;

    // Sorting
    // Sorts the collection when needed; returns true if a sorting operation was performed.
    bool sort();

    // Saving and Loading
    // Writes the collection to a file; returns false if opening, writing, or closing fails.
    bool save(const std::string& filename) const;
    // Loads values from a file; returns false on file or parsing failure without replacing existing contents.
    bool load(const std::string& filename);

    // Global
    // Returns a read-only reference to the first value; throws if the collection is empty.
    const T& front() const;
    // Returns a read-only reference to the last value; throws if the collection is empty.
    const T& back() const;
    // Reports whether the collection contains no nodes.
    bool isEmpty() const;
    // Reports the collection's tracked sortedness state.
    bool sorted() const;
    // Returns the number of nodes in the collection.
    std::size_t getSize() const;
    // Returns the collection values as a string separated by arrows.
    std::string toString() const;

    private:

    // Value returned by search() when no matching element is found.
    static constexpr std::size_t NOT_FOUND = std::numeric_limits<std::size_t>::max();

    // Each node stores one value and a pointer to the next node.
    struct Node
    {
        T data;
        Node* next;

        // Initialize the stored value and mark this as the last node for now.
        Node(const T& data) : data(data), next(nullptr) {}
    };

    Node* head; // First node; nullptr means the list is empty.
    std::size_t size; // Number of nodes currently in the collection.
    // Tracks whether the collection is known to be in nondecreasing order.
    bool isSorted = true; // Keeps track if the collection is sorted or not.

    // Non-const versions: Allow modification of the nodes.
    // Returns a modifiable pointer to the last node; throws if the collection is empty.
    Node* traverseToTail();
    // Returns a modifiable pointer to the node at index; throws for an invalid index.
    Node* traverseToIndex(std::size_t index);

    // Const versions: Prevent modification of the nodes.
    // Returns a read-only pointer to the last node; throws if the collection is empty.
    const Node* traverseToTail() const;
    // Returns a read-only pointer to the node at index; throws for an invalid index.
    const Node* traverseToIndex(std::size_t index) const;

    // Sorting
    // Sorts the node values using insertion sort.
    void InsertionSort();
    // Sorts the inclusive node range from low through high using quicksort.
    void QuickSort(Node* low, Node* high);
    // Recursively sorts the sublist beginning at start and updates its head pointer.
    void MergeSort(Node*& start);
    // Merges two sorted sublists by relinking their existing nodes.
    Node* Merge(Node* left, Node* right);
};

#pragma region Constructor / Destructor

// ------------------------Constructor / Destructor---------------------------

// Default construction creates an empty collection.
template <typename T>
inline Collection<T>::Collection() : head(nullptr), size(0), isSorted(true) {}

// Destroying a collection must release all nodes it owns.
template <typename T>
inline Collection<T>::~Collection()
{
    clear();
}

#pragma endregion

#pragma region Copy Constructor / Copy Assignment Operator

// --------------Copy Constructor / Copy Assignment Operator--------------------

// Deep-copy constructor: create independent nodes containing the source values.
template <typename T>
inline Collection<T>::Collection(const Collection<T>& source) : head(nullptr), size(0), isSorted(source.sorted())
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

// Copy assignment uses copy-and-swap so the destination receives an independent copy.
template <typename T>
inline Collection<T>& Collection<T>::operator=(const Collection<T>& source)
{
    // Check whether the source and destination are the same object.
    // If they are, return immediately to avoid unnecessary copying.
    if (this == &source)
    {
        return *this;
    }

    // Create a temporary deep copy of the source collection.
    // This allocates independent nodes without modifying the destination.
    Collection<T> temp(source);

    // Exchange the destination's head pointer with the temporary's head pointer.
    // The destination now owns the newly copied nodes.
    std::swap(head, temp.head);

    // Exchange the node counts to keep both collections consistent.
    std::swap(size, temp.size);

    std::swap(isSorted, temp.isSorted);

    // Return the current collection to support assignment chaining.
    return *this;
}

#pragma endregion

#pragma region Move Constructor / Move Assignment Operator

// --------------Move Constructor / Move Assignment Operator--------------------

// Move construction transfers the source nodes and leaves the source empty.
template <typename T>
inline Collection<T>::Collection(Collection<T>&& source) noexcept : head(source.head), size(source.size), isSorted(source.sorted())
{
    // Transfer ownership of the source collection's nodes.
    // The destination now points to the source's original linked list.

    // Copy the source's node count into the destination.
    // This keeps the destination's size consistent with its nodes.

    // Reset the source head because it no longer owns the transferred nodes.
    source.head = nullptr;

    // Reset the source size to represent an empty collection.
    source.size = 0;

    // Reset the source's sorted-state flag to represent a sorted collection.
    source.isSorted = true;
}

// Move assignment releases the current nodes before taking ownership of the source nodes.
template <typename T>
inline Collection<T>& Collection<T>::operator=(Collection<T>&& source) noexcept
{
    // Check whether the source and destination are the same object.
    // If they are, return immediately to avoid modifying the collection.
    if (this == &source)
    {
        return *this;
    }
    
    // Release the destination's existing nodes before taking ownership
    // of the source collection's nodes.
    clear();

    // Transfer ownership of the source's linked-list nodes.
    head = source.head;

    // Transfer the source's node count to the destination.
    size = source.size;

    isSorted = source.sorted();

    // Reset the source head because ownership has been transferred.
    source.head = nullptr;

    // Reset the source size to represent an empty collection.
    source.size = 0;

    // Reset the source's sorted-state flag to represent a sorted collection.
    source.isSorted = true;

    // Return the destination collection to support assignment chaining.
    return *this;
}

#pragma endregion

#pragma region Insertion

// -------------------------------Insertion------------------------------------

// Add a value at the beginning.
template <typename T>
inline bool Collection<T>::push_front(const T& data)
{
    return insert(data, 0);
}

// Add a value at the end by inserting at the current size.
template <typename T>
inline bool Collection<T>::push_back(const T& data)
{
    return insert(data, size);
}

// Insert data at index; index == size means append to the end.
template <typename T>
inline bool Collection<T>::insert(const T& data, std::size_t index)
{
    // Validate that index is non-negative and less than or equal to size
    if (index > size)
    {
        return false;
    }

    // Create new node and initialize data and next pointer
    // Allocate a new node to hold the inserted value.
    // Keep track of the node immediately preceding the insertion point.
    Node* newNode = new Node(data);
    Node* prevNode = nullptr;

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
        prevNode = currentNode;
        currentNode->next = newNode; // Insert the new node after currentNode.
    }

    // If the collection is marked sorted, check whether the changed value preserves local order.
    if (sorted())
    {
        bool orderedBefore = index == 0 || prevNode->data <= data;

        Node* nextNode = newNode->next;
        bool orderedAfter = nextNode == nullptr || data <= nextNode->data;

        isSorted = orderedBefore && orderedAfter;
    }

    // Increase list size
    size++;

    return true;
}

// Insert a value into its correct position in an already-sorted collection.
template <typename T>
inline bool Collection<T>::insertSorted(const T& data)
{
    // Insertion is only allowed when the collection is already sorted.
    if (!sorted())
    {
        return false;
    }

    // Pointers used to locate the insertion position.
    Node* currentNode = head;
    Node* prevNode = nullptr;

    // Traverse while existing values are less than or equal to data.
    // This places duplicate values after existing occurrences.
    while (currentNode != nullptr && currentNode->data <= data)
    {
        prevNode = currentNode;
        currentNode = currentNode->next;
    }

    // Create a new node and connect it to the node currently at the insertion position.
    Node* newNode = new Node(data);
    newNode->next = currentNode;

    // If no previous node exists, the new node becomes the head.
    if (prevNode == nullptr)
    {
        head = newNode;
    }
    else
    {
        // Otherwise, connect the previous node to the new node.
        prevNode->next = newNode;
    }

    // Update the collection's size after successful insertion.
    size++;

    return true;
}

#pragma endregion

#pragma region Deletion

// --------------------------------Deletion-------------------------------------

// Find the value's index and remove it if it exists.
// Remove the first matching value by locating its index and delegating to removeAt().
template <typename T>
inline bool Collection<T>::remove(const T& data)
{
    std::size_t index = search(data);

    if (index != NOT_FOUND)
    {
        removeAt(index);
        return true;
    }
    
    return false;
}

// Remove the node at a zero-based index.
// Remove the node at index and update the tracked sortedness state when the size becomes small.
template <typename T>
inline bool Collection<T>::removeAt(std::size_t index)
{
    // Validate that index is non-negative and less than size
    if (index >= size)
    {
        return false;
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

    // Consider a collection of 0 or 1 elements to be sorted.
    if (size <= 1)
    {
        isSorted = true;
    }

    return true;
}

// Copy the first value into out, then remove the first node.
template <typename T>
inline bool Collection<T>::pop_front(T& out)
{
    if (head == nullptr)
    {
        return false;
    }

    // Return data at index
    out = head->data;
    removeAt(0);
    return true;
}

// Remove the last node and copy its value into out.
template <typename T>
inline bool Collection<T>::pop_back(T& out)
{
    // Check if list is not empty
    if (isEmpty())
    {
        return false;
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
        for (std::size_t i = 0; i < size - 1; i++)
        {
            prevNode = currentNode;
            currentNode = currentNode->next;
        }

        out = currentNode->data; // Preserve the value before deleting the node.
        prevNode->next = nullptr; // The predecessor becomes the new tail.
        delete currentNode; // Release the removed tail node.
    }

    size--;

    // Consider a collection of 0 or 1 elements to be sorted.
    if (size <= 1)
    {
        isSorted = true;
    }

    return true;
}

// Remove the first node.
// Remove the head node without copying its value to an output parameter.
template <typename T>
inline bool Collection<T>::pop_front()
{
    if (head == nullptr)
    {
        return false;
    }

    removeAt(0);
    return true;
}

// Remove the last node.
// Remove the tail node without copying its value to an output parameter.
template <typename T>
inline bool Collection<T>::pop_back()
{
    // Check if list is not empty
    if (isEmpty())
    {
        return false;
    }

    // currentNode moves through the list; prevNode follows one node behind.
    Node* currentNode = head;
    Node* prevNode = nullptr;

    // Special case: a one-node list has no predecessor.
    if (currentNode->next == nullptr)
    {
        head = nullptr;
        delete currentNode;
    }
    else
    {
        // Advance until currentNode is the tail and prevNode is its predecessor.
        for (std::size_t i = 0; i < size - 1; i++)
        {
            prevNode = currentNode;
            currentNode = currentNode->next;
        }

        prevNode->next = nullptr; // The predecessor becomes the new tail.
        delete currentNode; // Release the removed tail node.
    }

    size--;

    // Consider a collection of 0 or 1 elements to be sorted.
    if (size <= 1)
    {
        isSorted = true;
    }

    return true;
}

// Delete all nodes and reset the collection to an empty state.
template <typename T>
inline bool Collection<T>::clear()
{
    // Traversal node
    Node* currentNode = head;
    std::size_t originalSize = getSize();

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
    isSorted = true;

    return getSize() < originalSize;
}

#pragma endregion

#pragma region Replacement

// -----------------------------Replacement------------------------------------

// Replace the first occurrence of oldVal while checking neighboring values when the list is marked sorted.
template <typename T>
inline bool Collection<T>::replace(const T& newVal, const T& oldVal)
{
    std::size_t oldIndex = search(oldVal);
    bool oldValExists = oldIndex != NOT_FOUND;

    if (!oldValExists)
    {
        return false;
    }

    Node* currentNode = head;
    Node* prevNode = nullptr;

    for (std::size_t i = 0; i < oldIndex; i++)
    {
        prevNode = currentNode;
        currentNode = currentNode->next;
    }

    // If the collection is marked sorted, check whether the changed value preserves local order.
    if (sorted())
    {
        bool orderedBefore = oldIndex == 0 || prevNode->data <= newVal;

        Node* nextNode = currentNode->next;
        bool orderedAfter = nextNode == nullptr || newVal <= nextNode->data;

        isSorted = orderedBefore && orderedAfter;
    }

    currentNode->data = newVal;
    return true;
}

// Replace the value at index while checking neighboring values when the list is marked sorted.
template <typename T>
inline bool Collection<T>::replaceAt(const T& data, std::size_t index)
{
    if (index >= size)
    {
        return false;
    }

    Node* currentNode = traverseToIndex(index);
    Node* prevNode = nullptr;
    
    prevNode = index > 0 ? traverseToIndex(index - 1) : nullptr;

    // If the collection is marked sorted, check whether the changed value preserves local order.
    if (sorted())
    {
        bool orderedBefore = index == 0 || prevNode->data <= data;

        Node* nextNode = currentNode->next;
        bool orderedAfter = nextNode == nullptr || data <= nextNode->data;

        isSorted = orderedBefore && orderedAfter;
    }

    currentNode->data = data;
    return true;
}

#pragma endregion

#pragma region Swapping

// -------------------------------Swapping--------------------------------------

// Swap the stored values at the first matching positions without relinking nodes.
template <typename T>
inline bool Collection<T>::swap(const T& newVal, const T& oldVal)
{
    std::size_t newIndex = search(newVal);
    std::size_t oldIndex = search(oldVal);

    if (newIndex == NOT_FOUND || oldIndex == NOT_FOUND)
    {
        return false;
    }

    if (newIndex == oldIndex)
    {
        return true;
    }
    else
    {
        Node* newIndexNode = traverseToIndex(newIndex);
        Node* oldIndexNode = traverseToIndex(oldIndex);

        std::swap(newIndexNode->data, oldIndexNode->data);
        isSorted = false;
        return true;
    }
}

// Swap the first occurrence of data with the value stored at the requested index.
template <typename T>
inline bool Collection<T>::swapAt(const T& data, std::size_t index)
{
    std::size_t foundIndex = search(data);

    if (foundIndex == NOT_FOUND || index >= size)
    {
        return false;
    }

    if (foundIndex == index)
    {
        return true;
    }
    else
    {
        Node* foundNode = traverseToIndex(foundIndex);
        Node* indexNode = traverseToIndex(index);

        std::swap(foundNode->data, indexNode->data);
        isSorted = false;
        return true;
    }
}

#pragma endregion

#pragma region Searching

// -------------------------------Searching------------------------------------

// Returns either true or false based if the data exists in the collection.
template <typename T>
inline bool Collection<T>::contains(const T& data) const
{
    // Search for the data in the collection.
    std::size_t index = search(data);

    // Returns whether it exists.
    return index != NOT_FOUND;
}

// Return the value at index after validating the requested position.
template <typename T>
inline const T& Collection<T>::at(std::size_t index) const
{
    // Validate that index is non-negative and less than size
    if (index >= size)
    {
        throw IndexOutOfBounds();
    }

    // Traversal node
    const Node* currentNode = traverseToIndex(index);

    // Return data at index
    return currentNode->data;
}

// Return the first matching index, or NOT_FOUND if there is no match.
template <typename T>
inline std::size_t Collection<T>::search(const T& data) const
{
    // Traversal node
    // Begin at the head and inspect nodes one by one.
    const Node* currentNode = head;

    // Index of the node currently being inspected.
    std::size_t index = 0;

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

#pragma endregion

#pragma region Sorting

// --------------------------------Sorting--------------------------------------

// Sorts the collection by choosing a sorting algorithm based on collection size.
template <typename T>
inline bool Collection<T>::sort()
{
    if (head == nullptr)
    {
        isSorted = true;
        return false;
    }

    if (sorted())
    {
        return false;
    }

    if (size <= 50)
    {
        InsertionSort();
    }
    else
    {
        MergeSort(head);
    }

    isSorted = true;
    return true;
}

// Insertion sort rearranges values in place by growing a sorted prefix of the list.
template <typename T>
inline void Collection<T>::InsertionSort()
{
    // Start at the head of the collection.
    Node* currentNode = head;

    // Begin at index 1 because the first element is already considered sorted.
    for (std::size_t i = 1; i < size; i++)
    {
        // Advance to the next element in the unsorted portion.
        currentNode = currentNode->next;
        
        // Store the element that needs to be inserted into the sorted portion.
        T temp = currentNode->data;

        // Start searching from the beginning of the sorted portion.
        Node* traversalNode = head;
        
        // Find the first node containing a value greater than temp.
        // Continue traversing while values are less than or equal to temp.
        while (traversalNode != currentNode && traversalNode->data <= temp)
        {
            traversalNode = traversalNode->next;
        }

        // Shift values forward to make room for the element being inserted.
        while (traversalNode != currentNode)
        {
            // Preserve the value currently stored in the node.
            T displace = traversalNode->data;

            // Place the value being inserted into the current node.
            traversalNode->data = temp;

            // Carry the displaced value forward to the next node.
            temp = displace;

            // Advance to the next node.
            traversalNode = traversalNode->next;
        }

        // Handle the final node in the shifting process.
        currentNode->data = temp;
    }
}

// Quicksort partitions the specified node range around its final pivot position.
template <typename T>
inline void Collection<T>::QuickSort(Node* low, Node* high)
{
    // Base case: Stop if the section is empty, invalid, or contains only one node.
    if (low == nullptr || high == nullptr || low == high)
    {
        return;
    }

    // Choose the last element in the current section as the pivot.
    T pivot = high->data;

    // i tracks the boundary of the section containing values smaller than the pivot.
    // nullptr represents the position immediately before the first node.
    Node* i = nullptr;

    // j traverses the current section, examining each node before the pivot.
    Node* j = low;

    // Traverse the section until reaching the pivot node.
    while (j != high)
    {
        // Check whether the current value belongs in the smaller-elements section.
        if (j->data < pivot)
        {
            // Advance the smaller-elements boundary.
            // If this is the first smaller element, initialize i at low.
            i = i == nullptr ? low : i->next;

            // Move the smaller value into the section before the pivot.
            std::swap(i->data, j->data);
        }

        // Advance to the next node regardless of whether a swap occurred.
        j = j->next;
    }
    
    // Store the node where the pivot will be placed in its final position.
    Node* pivotNode = nullptr;

    // If no elements were smaller than the pivot, place it at the beginning.
    if (i == nullptr)
    {
        pivotNode = low;

        // Exchange the first node's value with the pivot's value.
        std::swap(pivotNode->data, high->data);
    }
    else
    {
        // Otherwise, place the pivot immediately after the smaller-elements section.
        pivotNode = i->next;

        // Exchange the destination node's value with the pivot's value.
        std::swap(pivotNode->data, high->data);
    }

    // Recursively sort the section containing values smaller than the pivot.
    // The pivot is excluded because it is already in its final position.
    if (i != nullptr)
    {
        QuickSort(low, i);
    }
    
    // Recursively sort the section containing values greater than
    // or equal to the pivot.
    // Start immediately after the pivot to exclude it from recursion.
    if (pivotNode != high)
    {
        QuickSort(pivotNode->next, high);
    }
}

// Merge sort divides the sublist, recursively sorts each half, and merges the results.
template <typename T>
inline void Collection<T>::MergeSort(Node*& start)
{
    // Base case: A sublist containing zero or one node is already sorted.
    if (start == nullptr || start->next == nullptr)
    {
        return;
    }
    
    // Initialize two pointers to locate the midpoint of the sublist.
    // The slow pointer advances one node at a time.
    Node* slowNode = start;

    // The fast pointer advances two nodes at a time.
    Node* fastNode = start->next;
    
    // Continue traversing while the fast pointer and its next node exist.
    while (fastNode != nullptr && fastNode->next != nullptr)
    {
        // Advance the slow pointer by one node.
        slowNode = slowNode->next;
        
        // Temporarily store the node immediately after fastNode.
        Node* fasterNode = fastNode->next;

        // Advance the fast pointer by two nodes.
        fastNode = fasterNode->next;
    }
    
    // Preserve the beginning of the right sublist.
    Node* rightHead = slowNode->next;

    // Disconnect the two sublists by terminating the left sublist.
    slowNode->next = nullptr;

    // Recursively sort the left sublist.
    // The reference allows start to be updated to its new head.
    MergeSort(start);

    // Recursively sort the right sublist.
    // The reference allows rightHead to be updated to its new head.
    MergeSort(rightHead);

    // Merge the two sorted sublists and update start
    // to point to the beginning of the newly merged list.
    start = Merge(start, rightHead);
}

// Merge two already-sorted sublists into one sorted chain of existing nodes.
template <typename T>
inline typename Collection<T>::Node* Collection<T>::Merge(Node* left, Node* right)
{
    // Base case: If the left sublist is empty, return the right sublist.
    if (left == nullptr)
    {
        return right;
    }
    
    // Base case: If the right sublist is empty, return the left sublist.
    if (right == nullptr)
    {
        return left;
    }

    // Pointers used to track the beginning and end of the merged list.
    Node* mergedHead = nullptr;
    Node* mergedTail = nullptr;

    // Select the smaller of the first nodes from the two sublists.
    // If values are equal, select from the left to preserve stable sorting.
    if (right->data < left->data)
    {
        mergedHead = right;
    }
    else
    {
        mergedHead = left;
    }
    
    // Initialize the tail to the first node of the merged list.
    mergedTail = mergedHead;
    
    // Advance the sublist pointer from which the first node was selected.
    // This prevents selecting the same node again.
    if (mergedHead == left)
    {
        left = left->next;
    }
    else
    {
        right = right->next;
    }

    // Continue merging while both sublists still contain nodes.
    while (left != nullptr && right != nullptr)
    {
        // Select the smaller node and connect it to the end of the merged list.
        // When values are equal, select from the left sublist.
        mergedTail->next = left->data <= right->data ? left : right;

        // Advance the pointer belonging to the selected sublist.
        if (mergedTail->next == left)
        {
            left = left->next;
        }
        else
        {
            right = right->next;
        }

        // Advance the tail to the newly appended node.
        mergedTail = mergedTail->next;
    }
    
    // Attach the remaining nodes from whichever sublist is not empty.
    // The remaining sublist is already sorted, so no further comparisons are needed.
    mergedTail->next = left != nullptr ? left : right;

    // Return the beginning of the fully merged list.
    return mergedHead;
}

#pragma endregion

#pragma region Saving and Loading

// -------------------------Saving and Loading----------------------------------

// Save each node to a separate line, quoting string values so whitespace can be preserved.
template <typename T>
inline bool Collection<T>::save(const std::string& filename) const
{
    std::ofstream file(filename);

    if (!file)
    {
        return false;
    }

    // max_digits10 preserves enough significant digits to reload floating-point values accurately.
    if constexpr (std::is_floating_point_v<T>)
    {
        file << std::setprecision(std::numeric_limits<T>::max_digits10);
    }
    
    // Traverse the list and serialize every node, including duplicate values.
    const Node* currentNode = head;
    
    while (currentNode != nullptr)
    {
        if constexpr (std::is_same_v<T, std::string>)
        {
            file << std::quoted(currentNode->data) << '\n';
        }
        else
        {
            file << currentNode->data << '\n';
        }
        currentNode = currentNode->next;
    }

    // Closing flushes buffered output, so check the stream state after closing.
    file.close();

    if (!file)
    {
        return false;
    }

    return true;
}

// Load into a temporary collection so a failed read leaves the current collection unchanged.
template <typename T>
inline bool Collection<T>::load(const std::string& filename)
{
    std::ifstream file(filename);

    if (!file)
    {
        return false;
    }
    
    // Store parsed values separately until the entire file has been read successfully.
    Collection<T> temp;
    T tempVal;
    
    // previous code...

    // Read values until the end of the file, preserving every occurrence.
    while (true)
    {
        if (file.eof())
        {
            break;
        }

        // Consume whitespace before the second EOF check so trailing whitespace is handled as normal end-of-file.
        file >> std::ws;

        if (file.bad())
        {
            return false;
        }

        if (file.eof())
        {
            break;
        }

        if (!file)
        {
            return false;
        }

        // Read strings with quotation handling; extract other element types normally.
        if constexpr (std::is_same_v<T, std::string>)
        {
            if (!(file >> std::quoted(tempVal)))
            {
                return false;
            }
        }
        else
        {
            if (!(file >> tempVal))
            {
                return false;
            }
        }
        
        // Append every successfully read value; duplicates are intentionally retained.
        temp.push_back(tempVal);
    }
    
    // Close the input stream and verify that closing did not report a failure.
    file.close();
    
    if (!file)
    {
        return false;
    }
    
    // Determine whether the loaded values are in nondecreasing order.
    if (temp.head != nullptr)
    {
        Node* currentNode = temp.head;
        Node* nextNode = currentNode->next;
        temp.isSorted = true;
        
        // Compare adjacent nodes and stop as soon as an inversion is found.
        while (currentNode != nullptr && nextNode != nullptr && temp.sorted())
        {
            if (currentNode->data > nextNode->data)
            {
                temp.isSorted = false;
            }
            
            currentNode = currentNode->next;
            nextNode = currentNode->next;
        }
    }
    
    // Commit the loaded nodes only after reading and sortedness detection succeed.
    *this = std::move(temp);
    return true;
}

#pragma endregion

#pragma region Global

// ---------------------------------Global---------------------------------------

// Return the first value without removing it.
template <typename T>
inline const T& Collection<T>::front() const
{
    if (head == nullptr)
    {
        throw EmptyList();
    }

    return head->data;
}

// Find and return the final value in the list.
template <typename T>
inline const T& Collection<T>::back() const
{
    const Node* currentNode = traverseToTail();
    return currentNode->data;
}

// Report whether the collection contains zero nodes.
template <typename T>
inline bool Collection<T>::isEmpty() const
{
    return size == 0;
}

// Report whether the collection is sorted.
// Return the tracked sortedness flag without traversing the collection.
template <typename T>
inline bool Collection<T>::sorted() const
{
    return isSorted;
}

// Return the stored node count.
template <typename T>
inline std::size_t Collection<T>::getSize() const
{
    return size;
}

// Convert the list into text, placing arrows between adjacent values.
template <typename T>
inline std::string Collection<T>::toString() const
{
    std::stringstream ss; // Accumulate the output efficiently.

    const Node* currentNode = head; // Traverse from the first node.

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

#pragma endregion

#pragma region Private Traversal Helpers

// ---------------------------Private Traversal Helpers-------------------------

// Locate the final node; an empty collection has no tail to return.
template <typename T>
inline typename Collection<T>::Node* Collection<T>::traverseToTail()
{
    if (isEmpty())
    {
        throw EmptyList();
    }

    // Starting at the head, follow size - 1 links to reach the tail.
    Node* currentNode = head;

    for (std::size_t i = 0; i < size - 1; i++)
    {
        currentNode = currentNode->next;
    }

    return currentNode;
}

// Locate the final node; an empty collection has no tail to return.
template <typename T>
inline const typename Collection<T>::Node* Collection<T>::traverseToTail() const
{
    if (isEmpty())
    {
        throw EmptyList();
    }

    // Starting at the head, follow size - 1 links to reach the tail.
    const Node* currentNode = head;

    for (std::size_t i = 0; i < size - 1; i++)
    {
        currentNode = currentNode->next;
    }

    return currentNode;
}

// Locate the node at index; reject an index outside the valid range.
template <typename T>
inline typename Collection<T>::Node* Collection<T>::traverseToIndex(std::size_t index)
{
    if (index >= size)
    {
        throw IndexOutOfBounds();
    }

    Node* currentNode = head;

    // Follow next pointers index times to reach the requested list node.
    for (std::size_t i = 0; i < index; i++)
    {
        currentNode = currentNode->next;
    }

    return currentNode;
}

// Locate the node at index; reject an index outside the valid range.
template <typename T>
inline const typename Collection<T>::Node* Collection<T>::traverseToIndex(std::size_t index) const
{
    if (index >= size)
    {
        throw IndexOutOfBounds();
    }

    const Node* currentNode = head;

    // Follow next pointers index times to reach the requested list node.
    for (std::size_t i = 0; i < index; i++)
    {
        currentNode = currentNode->next;
    }

    return currentNode;
}

#pragma endregion