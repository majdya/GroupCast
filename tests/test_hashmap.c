#include "hash_map.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

static int passed = 0, failed = 0;

#define TEST(name) do { printf("  %s ... ", name); } while(0)
#define PASS() do { printf("PASS\n"); passed++; } while(0)
#define FAIL(msg) do { printf("FAIL: %s\n", msg); failed++; } while(0)
#define ASSERT(cond, msg) do { if (!(cond)) { FAIL(msg); return; } } while(0)

static size_t hash_int(void* key)
{
    return (size_t)(intptr_t)key;
}

static int eq_int(void* a, void* b)
{
    (void)a; (void)b;
    return (intptr_t)a == (intptr_t)b;
}

static size_t hash_str(void* key)
{
    size_t h = 0;
    unsigned char* p = (unsigned char*)key;
    while (*p) { h = h * 31 + *p++; }
    return h;
}

static int eq_str(void* a, void* b)
{
    return strcmp((const char*)a, (const char*)b) == 0;
}

static void test_create_destroy(void)
{
    TEST("HashMap_Create returns non-NULL");
    HashMap* m = HashMap_Create(10, hash_int, eq_int);
    ASSERT(m != NULL, "create returned NULL");
    PASS();

    TEST("HashMap_Destroy sets pointer to NULL");
    HashMap_Destroy(&m, NULL, NULL);
    ASSERT(m == NULL, "pointer not NULL after destroy");
    PASS();

    TEST("HashMap_Destroy on NULL is safe");
    HashMap_Destroy(NULL, NULL, NULL);
    PASS();
}

static void test_insert_find(void)
{
    HashMap* m = HashMap_Create(10, hash_int, eq_int);

    TEST("Insert returns MAP_SUCCESS");
    ASSERT(HashMap_Insert(m, (void*)1, (void*)"one") == MAP_SUCCESS, "insert failed");
    PASS();

    TEST("Find returns inserted value");
    void* val;
    ASSERT(HashMap_Find(m, (void*)1, &val) == MAP_SUCCESS, "find failed");
    ASSERT(val == (void*)"one", "wrong value");
    PASS();

    TEST("Size returns 1 after one insert");
    ASSERT(HashMap_Size(m) == 1, "wrong size");
    PASS();

    TEST("Insert duplicate returns MAP_KEY_DUPLICATE_ERROR");
    ASSERT(HashMap_Insert(m, (void*)1, (void*)"dup") == MAP_KEY_DUPLICATE_ERROR, "should reject duplicate");
    PASS();

    TEST("Find non-existent returns MAP_KEY_NOT_FOUND_ERROR");
    ASSERT(HashMap_Find(m, (void*)99, &val) == MAP_KEY_NOT_FOUND_ERROR, "should not find");
    PASS();

    HashMap_Destroy(&m, NULL, NULL);
}

static void test_remove(void)
{
    HashMap* m = HashMap_Create(10, hash_int, eq_int);
    HashMap_Insert(m, (void*)1, (void*)"one");
    HashMap_Insert(m, (void*)2, (void*)"two");

    TEST("Remove returns removed key and value");
    void* key = NULL, *val = NULL;
    ASSERT(HashMap_Remove(m, (void*)1, &key, &val) == MAP_SUCCESS, "remove failed");
    ASSERT(key == (void*)1, "wrong key");
    ASSERT(val == (void*)"one", "wrong value");
    PASS();

    TEST("Size decremented after remove");
    ASSERT(HashMap_Size(m) == 1, "wrong size");
    PASS();

    TEST("Remove non-existent returns MAP_KEY_NOT_FOUND_ERROR");
    ASSERT(HashMap_Remove(m, (void*)99, &key, &val) == MAP_KEY_NOT_FOUND_ERROR, "should not find");
    PASS();

    HashMap_Destroy(&m, NULL, NULL);
}

static void test_rehash(void)
{
    HashMap* m = HashMap_Create(3, hash_int, eq_int);
    HashMap_Insert(m, (void*)1, (void*)"a");
    HashMap_Insert(m, (void*)2, (void*)"b");
    HashMap_Insert(m, (void*)3, (void*)"c");
    ASSERT(HashMap_Size(m) == 3, "size should be 3 before rehash");

    TEST("Rehash increases capacity and preserves entries");
    ASSERT(HashMap_Rehash(m, 20) == MAP_SUCCESS, "rehash failed");
    ASSERT(HashMap_Size(m) == 3, "size should remain 3");

    void* val;
    ASSERT(HashMap_Find(m, (void*)1, &val) == MAP_SUCCESS && val == (void*)"a", "entry 1 lost");
    ASSERT(HashMap_Find(m, (void*)2, &val) == MAP_SUCCESS && val == (void*)"b", "entry 2 lost");
    ASSERT(HashMap_Find(m, (void*)3, &val) == MAP_SUCCESS && val == (void*)"c", "entry 3 lost");
    PASS();

    HashMap_Destroy(&m, NULL, NULL);
}

static void test_null_key(void)
{
    HashMap* m = HashMap_Create(10, hash_int, eq_int);

    TEST("Insert with NULL key returns MAP_KEY_NULL_ERROR");
    ASSERT(HashMap_Insert(m, NULL, (void*)"val") == MAP_KEY_NULL_ERROR, "should reject");
    PASS();

    TEST("Find with NULL key returns MAP_KEY_NULL_ERROR");
    void* val;
    ASSERT(HashMap_Find(m, NULL, &val) == MAP_KEY_NULL_ERROR, "should reject");
    PASS();

    TEST("Remove with NULL key returns MAP_KEY_NULL_ERROR");
    ASSERT(HashMap_Remove(m, NULL, NULL, NULL) == MAP_KEY_NULL_ERROR, "should reject");
    PASS();

    HashMap_Destroy(&m, NULL, NULL);
}

static int count_each(const void* key, void* value, void* ctx);

static void test_foreach(void)
{
    HashMap* m = HashMap_Create(10, hash_str, eq_str);
    HashMap_Insert(m, (void*)"k1", (void*)"v1");
    HashMap_Insert(m, (void*)"k2", (void*)"v2");
    HashMap_Insert(m, (void*)"k3", (void*)"v3");

    size_t cnt = 0;
    TEST("ForEach visits all entries");
    size_t n = HashMap_ForEach(m, count_each, &cnt);
    ASSERT(n == 3, "should visit 3 entries");
    ASSERT(cnt == 3, "callback should be called 3 times");
    PASS();

    HashMap_Destroy(&m, NULL, NULL);
}

static int count_each(const void* key, void* value, void* ctx)
{
    (void)key; (void)value;
    (*(size_t*)ctx)++;
    return 1;
}

static void test_foreach_action(void)
{
    HashMap* m = HashMap_Create(10, hash_str, eq_str);
    HashMap_Insert(m, (void*)"k1", (void*)"v1");
    HashMap_Insert(m, (void*)"k2", (void*)"v2");
    HashMap_Insert(m, (void*)"k3", (void*)"v3");

    size_t count = 0;
    TEST("ForEach with action counts all entries");
    HashMap_ForEach(m, count_each, &count);
    ASSERT(count == 3, "should count 3");
    PASS();

    HashMap_Destroy(&m, NULL, NULL);
}

static void test_statistics(void)
{
    HashMap* m = HashMap_Create(10, hash_int, eq_int);
    HashMap_Insert(m, (void*)1, (void*)"a");
    HashMap_Insert(m, (void*)2, (void*)"b");

#ifndef NDEBUG
    Map_Stats s = HashMap_GetStatistics(m);
    TEST("GetStatistics returns non-zero values");
    ASSERT(s.numberOfBuckets >= 10, "wrong bucket count");
    ASSERT(s.numberOfChains > 0, "should have chains");
    PASS();
#else
    TEST("GetStatistics (NDEBUG not defined)");
    PASS();
#endif

    HashMap_Destroy(&m, NULL, NULL);
}

int main(void)
{
    printf("hash_map tests:\n");
    test_create_destroy();
    test_insert_find();
    test_remove();
    test_rehash();
    test_null_key();
    test_foreach();
    test_foreach_action();
    test_statistics();

    printf("\n%d passed, %d failed out of %d\n", passed, failed, passed + failed);
    return failed > 0 ? 1 : 0;
}
