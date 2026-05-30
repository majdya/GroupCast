#ifndef GEN_DLIST_H
#define GEN_DLIST_H

/* =========================================================
 * gen_dlist.h - Generic doubly-linked list interface
 * ========================================================= */

#include <stddef.h>

typedef struct List List;
typedef void* ListItr;

/* Action callback – return 0 to stop iteration, non-zero to continue */
typedef int (*ListActionFunction)(void* _element, void* _context);

/* Create an empty list.  Returns NULL on allocation failure. */
List* ListCreate(void);

/* Destroy the list and optionally every element inside it. */
void ListDestroy(List** _pList, void (*_elementDestroy)(void* _item));

/* Add an element to the head / tail.
 * Returns an iterator to the new node, or NULL on failure. */
ListItr ListPushHead(List* _list, void* _item);
ListItr ListPushTail(List* _list, void* _item);

/* Remove and return the element at the head / tail.
 * Returns NULL if the list is empty. */
void* ListPopHead(List* _list);
void* ListPopTail(List* _list);

/* Iterators */
ListItr ListItrBegin(const List* _list);
ListItr ListItrEnd  (const List* _list);
ListItr ListItrNext (ListItr _itr);
ListItr ListItrPrev (ListItr _itr);

/* Get / set the data at an iterator position */
void*   ListItrGet(ListItr _itr);
void*   ListItrSet(ListItr _itr, void* _element);

/* Insert a new node before the iterator position.
 * Returns an iterator to the new node, or NULL on failure. */
ListItr ListItrInsertBefore(ListItr _itr, void* _element);

/* Remove the node at the iterator position.
 * Returns the removed data. */
void* ListItrRemove(ListItr _itr);

/* Number of elements.  O(n). */
size_t ListSize(const List* _list);

/* Returns non-zero if the list is empty. */
int ListIsEmpty(const List* _list);

/* Apply _action to every element in [_begin, _end).
 * Stops early if _action returns 0.
 * Returns the iterator where iteration stopped (may be _end). */
ListItr ListItrForEach(ListItr _begin, ListItr _end,
                       ListActionFunction _action, void* _context);

#endif /* GEN_DLIST_H */
