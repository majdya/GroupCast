#include "gen_dlist.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int passed = 0, failed = 0;

#define TEST(name) do { printf("  %s ... ", name); } while(0)
#define PASS() do { printf("PASS\n"); passed++; } while(0)
#define FAIL(msg) do { printf("FAIL: %s\n", msg); failed++; } while(0)
#define ASSERT(cond, msg) do { if (!(cond)) { FAIL(msg); return; } } while(0)

static void test_create_destroy(void)
{
    TEST("ListCreate returns non-NULL");
    List* l = ListCreate();
    ASSERT(l != NULL, "ListCreate returned NULL");
    PASS();

    TEST("ListDestroy sets pointer to NULL");
    ListDestroy(&l, NULL);
    ASSERT(l == NULL, "pointer not NULL after destroy");
    PASS();

    TEST("ListDestroy on NULL is safe");
    ListDestroy(NULL, NULL);
    PASS();
}

static void test_push_pop_head(void)
{
    List* l = ListCreate();

    TEST("ListPushHead returns valid iterator");
    ListItr it = ListPushHead(l, (void*)42);
    ASSERT(it != NULL, "PushHead returned NULL");
    PASS();

    TEST("ListPopHead returns pushed value");
    void* v = ListPopHead(l);
    ASSERT(v == (void*)42, "wrong value");
    PASS();

    TEST("ListPopHead on empty returns NULL");
    v = ListPopHead(l);
    ASSERT(v == NULL, "should be NULL");
    PASS();

    ListDestroy(&l, NULL);
}

static void test_push_pop_tail(void)
{
    List* l = ListCreate();

    TEST("ListPushTail returns valid iterator");
    ListItr it = ListPushTail(l, (void*)99);
    ASSERT(it != NULL, "PushTail returned NULL");
    PASS();

    TEST("ListPopTail returns pushed value");
    void* v = ListPopTail(l);
    ASSERT(v == (void*)99, "wrong value");
    PASS();

    TEST("ListPopTail on empty returns NULL");
    v = ListPopTail(l);
    ASSERT(v == NULL, "should be NULL");
    PASS();

    ListDestroy(&l, NULL);
}

static void test_size_empty(void)
{
    List* l = ListCreate();

    TEST("new list is empty");
    ASSERT(ListIsEmpty(l) != 0, "should be empty");
    PASS();

    TEST("new list size is 0");
    ASSERT(ListSize(l) == 0, "size should be 0");
    PASS();

    ListPushHead(l, (void*)1);
    ListPushHead(l, (void*)2);
    ListPushHead(l, (void*)3);

    TEST("list with 3 elements is not empty");
    ASSERT(ListIsEmpty(l) == 0, "should not be empty");
    PASS();

    TEST("list size is 3");
    ASSERT(ListSize(l) == 3, "size should be 3");
    PASS();

    ListPopHead(l);
    ListPopHead(l);
    ListPopHead(l);

    TEST("list empty after removing all");
    ASSERT(ListIsEmpty(l) != 0, "should be empty");
    PASS();

    ListDestroy(&l, NULL);
}

static void test_order(void)
{
    List* l = ListCreate();
    ListPushTail(l, (void*)"a");
    ListPushTail(l, (void*)"b");
    ListPushTail(l, (void*)"c");

    TEST("FIFO order via PopHead");
    ASSERT(ListPopHead(l) == (void*)"a", "first should be a");
    ASSERT(ListPopHead(l) == (void*)"b", "second should be b");
    ASSERT(ListPopHead(l) == (void*)"c", "third should be c");
    PASS();

    ListPushHead(l, (void*)"x");
    ListPushHead(l, (void*)"y");

    TEST("LIFO order via PopHead");
    ASSERT(ListPopHead(l) == (void*)"y", "first should be y");
    ASSERT(ListPopHead(l) == (void*)"x", "second should be x");
    PASS();

    ListDestroy(&l, NULL);
}

static void test_iterators(void)
{
    List* l = ListCreate();
    ListItr end = ListItrEnd(l);

    TEST("begin == end on empty list");
    ASSERT(ListItrBegin(l) == end, "begin should equal end");
    PASS();

    ListPushTail(l, (void*)"a");
    ListPushTail(l, (void*)"b");
    ListPushTail(l, (void*)"c");

    TEST("iterating forward yields correct order");
    ListItr it = ListItrBegin(l);
    ASSERT(ListItrGet(it) == (void*)"a", "first element");
    it = ListItrNext(it);
    ASSERT(ListItrGet(it) == (void*)"b", "second element");
    it = ListItrNext(it);
    ASSERT(ListItrGet(it) == (void*)"c", "third element");
    it = ListItrNext(it);
    ASSERT(it == end, "should be at end");
    PASS();

    TEST("iterating backward yields reverse order");
    it = ListItrPrev(end);
    ASSERT(ListItrGet(it) == (void*)"c", "last element");
    it = ListItrPrev(it);
    ASSERT(ListItrGet(it) == (void*)"b", "second to last");
    it = ListItrPrev(it);
    ASSERT(ListItrGet(it) == (void*)"a", "first element");
    PASS();

    ListDestroy(&l, NULL);
}

static void test_insert_before(void)
{
    List* l = ListCreate();
    ListPushTail(l, (void*)"b");
    ListPushTail(l, (void*)"d");
    ListItr it = ListItrBegin(l);

    TEST("insert before first element");
    ListItr inserted = ListItrInsertBefore(it, (void*)"a");
    ASSERT(inserted != NULL, "insert returned NULL");
    ASSERT(ListItrGet(ListItrBegin(l)) == (void*)"a", "a should be first");
    PASS();

    it = ListItrNext(ListItrBegin(l));  // at b
    it = ListItrNext(it);               // at d
    TEST("insert before last element");
    ListItrInsertBefore(it, (void*)"c");
    ASSERT(ListItrGet(ListItrPrev(it)) == (void*)"c", "c should be before d");
    PASS();

    TEST("final order after inserts");
    ASSERT(ListPopHead(l) == (void*)"a", "a");
    ASSERT(ListPopHead(l) == (void*)"b", "b");
    ASSERT(ListPopHead(l) == (void*)"c", "c");
    ASSERT(ListPopHead(l) == (void*)"d", "d");
    PASS();

    ListDestroy(&l, NULL);
}

static void test_remove(void)
{
    List* l = ListCreate();
    ListPushTail(l, (void*)"a");
    ListItr mid = ListPushTail(l, (void*)"b");
    ListPushTail(l, (void*)"c");

    TEST("removing middle element returns its data");
    void* removed = ListItrRemove(mid);
    ASSERT(removed == (void*)"b", "removed wrong data");
    PASS();

    TEST("order correct after remove");
    ASSERT(ListPopHead(l) == (void*)"a", "first should be a");
    ASSERT(ListPopHead(l) == (void*)"c", "second should be c");
    PASS();

    ListDestroy(&l, NULL);
}

static int count_action(void* _element, void* _context)
{
    (void)_element;
    (*(size_t*)_context)++;
    return 1;
}

static int stop_action(void* _element, void* _context)
{
    if (_element == (void*)"stop") return 0;
    (*(size_t*)_context)++;
    return 1;
}

static void test_foreach(void)
{
    List* l = ListCreate();
    ListPushTail(l, (void*)"a");
    ListPushTail(l, (void*)"b");
    ListPushTail(l, (void*)"c");

    TEST("ListItrForEach visits all elements");
    size_t count = 0;
    ListItr end = ListItrForEach(ListItrBegin(l), ListItrEnd(l), count_action, &count);
    ASSERT(count == 3, "should visit 3 elements");
    ASSERT(end == ListItrEnd(l), "should end at end");
    PASS();

    ListDestroy(&l, NULL);

    l = ListCreate();
    ListPushTail(l, (void*)"a");
    ListPushTail(l, (void*)"stop");
    ListPushTail(l, (void*)"c");

    TEST("ListItrForEach stops when action returns 0");
    count = 0;
    end = ListItrForEach(ListItrBegin(l), ListItrEnd(l), stop_action, &count);
    ASSERT(count == 1, "should visit 1 element before stop");
    ASSERT(end != ListItrEnd(l), "should stop before end");
    PASS();

    ListDestroy(&l, NULL);
}

static void test_set(void)
{
    List* l = ListCreate();
    ListPushTail(l, (void*)"old");

    TEST("ListItrSet returns old value");
    void* old = ListItrSet(ListItrBegin(l), (void*)"new");
    ASSERT(old == (void*)"old", "wrong old value");
    PASS();

    TEST("ListItrGet returns new value");
    ASSERT(ListItrGet(ListItrBegin(l)) == (void*)"new", "wrong new value");
    PASS();

    ListDestroy(&l, NULL);
}

int main(void)
{
    printf("gen_dlist tests:\n");
    test_create_destroy();
    test_push_pop_head();
    test_push_pop_tail();
    test_size_empty();
    test_order();
    test_iterators();
    test_insert_before();
    test_remove();
    test_foreach();
    test_set();

    printf("\n%d passed, %d failed out of %d\n", passed, failed, passed + failed);
    return failed > 0 ? 1 : 0;
}
