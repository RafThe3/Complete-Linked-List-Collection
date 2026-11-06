# Perfect-Linked-List-Collection

This project demonstrates a "perfect" template linked list collection, involving many Abstract Data Type (ADT) operations, such as list, stack, and queue operations. The collection includes STL libraries, basic exception handling, copying and moving semantics, and file serialization. To test the collection, first create a .cpp executable, then test every function in the header file. It is recommended print the vector after each function call, and it is optional to document the compute time of the sorting and searching functions to demonstrate time complexity of different algorithms.

Note (1): This collection does include duplicate values. To disallow this, add duplicate checks in the insertion operations before inserting values.

Note (2): This collection does not include a tail pointer. Inserting at the end takes $O(n)$ time instead of $O(1)$.

Note (3): By default, the QuickSort() algorithm is private implemented but not used officially in the sort() function. Benchmark this algorithm independently and determine to what collection size does it fit best.
