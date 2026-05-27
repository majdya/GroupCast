#include "gen_dlist.h"
#include <stdlib.h>

typedef struct Node Node;

struct Node {
    void* m_data;
    Node* m_next;
    Node* m_prev;
};

struct List {
    Node m_head;
    Node m_tail;
};

static void List_Init(List* _list)
{
    _list->m_head.m_data = NULL;
    _list->m_head.m_next = &_list->m_tail;
    _list->m_head.m_prev = NULL;
    _list->m_tail.m_data = NULL;
    _list->m_tail.m_next = NULL;
    _list->m_tail.m_prev = &_list->m_head;
}

List* ListCreate(void)
{
    List* list = (List*)malloc(sizeof(List));
    if (!list) return NULL;
    List_Init(list);
    return list;
}

void ListDestroy(List** _pList, void (*_elementDestroy)(void* _item))
{
    if (!_pList || !*_pList) return;
    List* list = *_pList;
    Node* cur = list->m_head.m_next;
    while (cur != &list->m_tail) {
        Node* next = cur->m_next;
        if (_elementDestroy) _elementDestroy(cur->m_data);
        free(cur);
        cur = next;
    }
    free(list);
    *_pList = NULL;
}

ListItr ListPushHead(List* _list, void* _item)
{
    if (!_list) return NULL;
    Node* node = (Node*)malloc(sizeof(Node));
    if (!node) return NULL;
    node->m_data = _item;
    node->m_prev = &_list->m_head;
    node->m_next = _list->m_head.m_next;
    _list->m_head.m_next->m_prev = node;
    _list->m_head.m_next = node;
    return (ListItr)node;
}

ListItr ListPushTail(List* _list, void* _item)
{
    if (!_list) return NULL;
    Node* node = (Node*)malloc(sizeof(Node));
    if (!node) return NULL;
    node->m_data = _item;
    node->m_next = &_list->m_tail;
    node->m_prev = _list->m_tail.m_prev;
    _list->m_tail.m_prev->m_next = node;
    _list->m_tail.m_prev = node;
    return (ListItr)node;
}

void* ListPopHead(List* _list)
{
    if (!_list || _list->m_head.m_next == &_list->m_tail) return NULL;
    Node* node = _list->m_head.m_next;
    void* data = node->m_data;
    _list->m_head.m_next = node->m_next;
    node->m_next->m_prev = &_list->m_head;
    free(node);
    return data;
}

void* ListPopTail(List* _list)
{
    if (!_list || _list->m_tail.m_prev == &_list->m_head) return NULL;
    Node* node = _list->m_tail.m_prev;
    void* data = node->m_data;
    _list->m_tail.m_prev = node->m_prev;
    node->m_prev->m_next = &_list->m_tail;
    free(node);
    return data;
}

size_t ListSize(const List* _list)
{
    if (!_list) return 0;
    size_t count = 0;
    Node* cur = _list->m_head.m_next;
    while (cur != &_list->m_tail) {
        count++;
        cur = cur->m_next;
    }
    return count;
}

int ListIsEmpty(const List* _list)
{
    if (!_list) return 1;
    return _list->m_head.m_next == &_list->m_tail;
}

ListItr ListItrBegin(const List* _list)
{
    if (!_list) return (ListItr)&_list->m_tail;
    return (ListItr)_list->m_head.m_next;
}

ListItr ListItrEnd(const List* _list)
{
    if (!_list) return NULL;
    return (ListItr)&_list->m_tail;
}

ListItr ListItrNext(ListItr _itr)
{
    if (!_itr) return NULL;
    Node* node = (Node*)_itr;
    return (ListItr)node->m_next;
}

ListItr ListItrPrev(ListItr _itr)
{
    if (!_itr) return NULL;
    Node* node = (Node*)_itr;
    return (ListItr)node->m_prev;
}

void* ListItrGet(ListItr _itr)
{
    if (!_itr) return NULL;
    Node* node = (Node*)_itr;
    return node->m_data;
}

void* ListItrSet(ListItr _itr, void* _element)
{
    if (!_itr) return NULL;
    Node* node = (Node*)_itr;
    void* old = node->m_data;
    node->m_data = _element;
    return old;
}

ListItr ListItrInsertBefore(ListItr _itr, void* _element)
{
    if (!_itr) return NULL;
    Node* pos = (Node*)_itr;
    Node* node = (Node*)malloc(sizeof(Node));
    if (!node) return NULL;
    node->m_data = _element;
    node->m_prev = pos->m_prev;
    node->m_next = pos;
    pos->m_prev->m_next = node;
    pos->m_prev = node;
    return (ListItr)node;
}

void* ListItrRemove(ListItr _itr)
{
    if (!_itr) return NULL;
    Node* node = (Node*)_itr;
    node->m_prev->m_next = node->m_next;
    node->m_next->m_prev = node->m_prev;
    void* data = node->m_data;
    free(node);
    return data;
}

ListItr ListItrForEach(ListItr _begin, ListItr _end, ListActionFunction _action, void* _context)
{
    if (!_begin || !_end || !_action) return _end ? _end : NULL;
    Node* cur = (Node*)_begin;
    Node* end = (Node*)_end;
    while (cur != end) {
        if (!_action(cur->m_data, _context)) return (ListItr)cur;
        cur = cur->m_next;
    }
    return (ListItr)end;
}
