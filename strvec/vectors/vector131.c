#include "vector131.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*Initializes all of the fields of a vector struct
 *Arguments:
 * - v: a pointer to a vector_t
 * - type: a size_t indicating the size of the type of this vector's elements
 *Return value: none
 */
void initialize_vector(vector_t *v, size_t type)
{
    // TODO: implement
    if (v != NULL && type != 0) {
        v->array = malloc(type * 1);
        v->ele_size = type;
        v->capacity = 1;
        v->length = 0;
    }
}

/*Frees this vector
 *Arguments:
 * - v: a pointer to the vector which needs to be freed
 *Return value: none
 */
void destroy_vector(vector_t *v)
{
    // TODO: implement
    free(v->array);
}

/*Gets the size of a given vector
 *Arguments:
 * - v: the vector whose size is desired
 *Return value: an integer containing the size of the vector
 */
size_t vector_size(vector_t *v)
{
    // TODO: implement
    return v->length;
}

/*Gets the element at a desired position within a vector
 *Arguments:
 * - v: a pointer to a vector_t
 * - index: the index of the desired element in v (with 0 indexing)
 *Return value: a void pointer to the element at index (to be casted
 *appropriately by the caller)
 */
void *vector_get(vector_t *v, int index)
{
    // TODO: implement
    size_t ret_index = (size_t)index * v->ele_size;
   
    return v->array + ret_index;
}

/*Adds an element to the back of a vector, doubling the capacity of the vector
 *if needed Arguments:
 * - v: a pointer to a vector_t
 * - ele: a pointer to the element to be copied into v
 * Return value: none
 */
void vector_add_back(vector_t *v, void *ele)
{
    // TODO: implement
    size_t n = v->length;
    size_t m = v->capacity;
    size_t end_index = m * v->ele_size;
    size_t curr_index = n * v->ele_size;

    if (curr_index == end_index) {
        v->array = (void *)realloc(v->array, v->ele_size * m * 2); 
        v->capacity = 2 * m;
    }
    void *tmp = v->array + curr_index;
    memcpy(tmp, ele, v->ele_size);
    v->length++;
}

/*Removes the last element in a vector
 *Arguments:
 * - v: a pointer to a vector_t
 *Return value: none
 */
void vector_delete_back(vector_t *v)
{
    // TODO: implement
    if (v->length != 0) {
        v->length--;
        size_t index = v->length * v->ele_size;
        void *tmp = v->array + index;
        memset(tmp, '\x00', v->ele_size);
    }
}

/*Adds an element to a specified index in a vector, double its capacity if
 *needed Arguments:
 * - v: a pointer to a vector_t
 * - ele: a pointer to the element to be copied into v
 * - index: the desired index for ele in the vector, v (using 0 indexing)
 *Return value: none
 */
void vector_add(vector_t *v, void *ele, int index)
{
    // TODO: implement
    size_t n = v->length;
    size_t m = v->capacity;
    size_t end_index = m * v->ele_size;
    size_t curr_index = n * v->ele_size;
    size_t insert_index = (size_t)index * v->ele_size;
    size_t unit = v->ele_size;
    
    if (curr_index == end_index) {
        v->array = (void *)realloc(v->array, v->ele_size * m * 2); 
        v->capacity = 2 * m;
    }

    int k = (int)((curr_index - insert_index) / v->ele_size);
    for (int i = 0; i < k; i++) {
        void *old = v->array + curr_index - unit;
        void *new = v->array + curr_index;
        memcpy(new, old, v->ele_size);
        curr_index -= unit;
    } 
    
    void *tmp = v->array + insert_index;
    memcpy(tmp, ele, v->ele_size);
    v->length++;
}

/*Deletes an element from the specified position in a vector
 *Arguments:
 * - v: a pointer to a vector_t
 * - index: the index of the element to be deleted from v (using 0 indexing)
 *Return value: none
 */
void vector_delete(vector_t *v, int index)
{
    // TODO: implement
    size_t n = v->length - 1;
    //size_t m = v->capacity;
    //size_t end_index = m * v->ele_size;
    size_t curr_index = n * v->ele_size;
    size_t delete_index = (size_t)index * v->ele_size;
    size_t unit = v->ele_size;
 
    if (delete_index == curr_index) {
        vector_delete_back(v);         
    } else {
        int k = (int)(curr_index - delete_index) / (int)(v->ele_size);
        for (int i = 0; i < k; i++) {
             void *old = v->array + delete_index;
             void *new = v->array + delete_index + unit;
             memcpy(old, new, v->ele_size);
             delete_index += 8;
        } 
        void *tmp = v->array + delete_index;
        memset(tmp, '\x00', v->ele_size);
        v->length--;
    }
}
