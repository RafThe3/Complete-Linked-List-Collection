#pragma once // Prevents this header from being included more than once.
#include <sstream> // Provides std::stringstream for building a std::string representation.
#include <string> // Provides std::string.
#include <cstddef> // Provides std::size_t.
#include <limits> // Provides numeric_limits for the NOT_FOUND sentinel.
#include <utility> // Provides std::swap().
#include <fstream>
#include <iomanip>
#include <type_traits>
#include <stdexcept>
#include "exceptions.hpp" // Defines IndexOutOfBounds.

// A singly linked-list collection that stores values of type T.
template <typename T>
class Collection
{
    public:

    // Constructor / Destructor
    Collection();
    ~Collection();

    // Copy constructor / copy assignment operator
    Collection(const Collection<T>& source);
    Collection<T>& operator=(const Collection<T>& source);

    // Move constructor / move assignment operator
    Collection(Collection<T>&& source) noexcept;
    Collection<T>& operator=(Collection<T>&& source) noexcept;

    // Insertion
    void push_front(const T& data);
    void push_back(const T& data);
    void insert(const T& data, std::size_t index);
    bool insertSorted(const T& data);

    // Deletion
    void remove(const T& data);
    void removeAt(std::size_t index);
    void pop_front(T& out);
    void pop_back(T& out);

    // Replacement
    bool replace(const T& newVal, const T& oldVal);
    bool replaceAt(const T& data, std::size_t index);

    // Swapping
    bool swap(const T& newVal, const T& oldVal);
    bool swapAt(const T& data, std::size_t index);

    // Searching
    bool contains(const T& data) const;
    const T& at(std::size_t index) const;
    std::size_t search(const T& data) const;

    // Sorting
    void sort();

    // Global
    const T& front() const;
    const T& back() const;
    void clear();
    bool isEmpty() const;
    std::size_t getSize() const;
    std::string toString() const;

    // Saving and Loading
    bool save(const std::string& filename) const;
    bool load(const std::string& filename);

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
    bool isSorted = true; // Keeps track if the collection is sorted or not.

    // Non-const versions: Allow modification of the nodes.
    Node* traverseToTail();
    Node* traverseToIndex(std::size_t index);

    // Const versions: Prevent modification of the nodes.
    const Node* traverseToTail() const;
    const Node* traverseToIndex(std::size_t index) const;

    // Sorting
    void InsertionSort();
    void QuickSort(Node* low, Node* high);
    void MergeSort(Node*& start);
    Node* Merge(Node* left, Node* right);
};

// ------------------------Constructor / Destructor---------------------------


// Default construction creates an empty collection.
template <typename T>
inline Collection<T>::Collection() : head(nullptr), size(0) {}

// Destroying a collection must release all nodes it owns.
template <typename T>
inline Collection<T>::~Collection()
{
    clear();
}


// --------------Copy Constructor / Copy Assignment Operator--------------------


// Deep-copy constructor: create independent nodes containing the source values.
template <typename T>
inline Collection<T>::Collection(const Collection<T>& source) : head(nullptr), size(0)
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

    // Return the current collection to support assignment chaining.
    return *this;
}


// --------------Move Constructor / Move Assignment Operator--------------------


template <typename T>
inline Collection<T>::Collection(Collection<T>&& source) noexcept : head(source.head), size(source.size)
{
    // Transfer ownership of the source collection's nodes.
    // The destination now points to the source's original linked list.

    // Copy the source's node count into the destination.
    // This keeps the destination's size consistent with its nodes.

    // Reset the source head because it no longer owns the transferred nodes.
    source.head = nullptr;

    // Reset the source size to represent an empty collection.
    source.size = 0;
}

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

    // Reset the source head because ownership has been transferred.
    source.head = nullptr;

    // Reset the source size to represent an empty collection.
    source.size = 0;

    // Return the destination collection to support assignment chaining.
    return *this;
}


// -------------------------------Insertion------------------------------------

// Add a value at the beginning.
template <typename T>
inline void Collection<T>::push_front(const T& data)
{
    insert(data, 0);
}

// Add a value at the end by inserting at the current size.
template <typename T>
inline void Collection<T>::push_back(const T& data)
{
    insert(data, size);
}

// Insert data at index; index == size means append to the end.
template <typename T>
inline void Collection<T>::insert(const T& data, std::size_t index)
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

// Insert a value into its correct position in an already-sorted collection.
template <typename T>
inline bool Collection<T>::insertSorted(const T& data)
{
    // Insertion is only allowed when the collection is already sorted.
    if (!isSorted)
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

// --------------------------------Deletion-------------------------------------

// Find the value's index and remove it if it exists.
template <typename T>
inline void Collection<T>::remove(const T& data)
{
    std::size_t index = search(data);

    if (index != NOT_FOUND)
    {
        removeAt(index);
    }
}

// Remove the node at a zero-based index.
template <typename T>
inline void Collection<T>::removeAt(std::size_t index)
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
}


// -----------------------------Replacement------------------------------------

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

    if (isSorted)
    {
        bool orderedBefore = oldIndex == 0 || prevNode->data <= newVal;

        Node* nextNode = currentNode->next;
        bool orderedAfter = nextNode == nullptr || newVal <= nextNode->data;

        isSorted = orderedBefore && orderedAfter;
    }

    currentNode->data = newVal;
    return true;
}

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

    if (isSorted)
    {
        bool orderedBefore = index == 0 || prevNode->data <= data;

        Node* nextNode = currentNode->next;
        bool orderedAfter = nextNode == nullptr || data <= nextNode->data;

        isSorted = orderedBefore && orderedAfter;
    }

    currentNode->data = data;
    return true;
}

// -------------------------------Swapping--------------------------------------

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


// --------------------------------Sorting--------------------------------------


// Sorts the collection by choosing a sorting algorithm based on collection size.
template <typename T>
inline void Collection<T>::sort()
{
    if (head == nullptr)
    {
        return;
    }

    if (size <= 50)
    {
        InsertionSort();
    }
    else
    {
        MergeSort(head);
    }
}

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

// Report whether the collection contains zero nodes.
template <typename T>
inline bool Collection<T>::isEmpty() const
{
    return size == 0;
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


// -------------------------Saving and Loading----------------------------------

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
    
    const Node* currentNode = head;
    
    while (currentNode != nullptr)
    {
        if constexpr (std::is_same_v(T, std::string))
        {
            file << std::quoted(currentNode->data) << '\n';
        }
        else
        {
            file << currentNode->data << '\n';
        }
        currentNode = currentNode->next;
    }

    file.close();

    if (!file)
    {
        return false;
    }

    return true;
}

template <typename T>
inline bool Collection<T>::load(const std::string& filename)
{
    std::ifstream file(filename);

    if (!file)
    {
        return false;
    }
    
    Collection<T> temp;
    T tempVal;
    
    // previous code...

    while (true)
    {
        if (file.eof())
        {
            break;
        }

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

        if (std::find(loaded.begin(), loaded.end(), tempVal) == loaded.end())
        {
            // code...
        }
    }

    // new code...
}
