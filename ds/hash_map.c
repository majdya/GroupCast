#include "hash_map.h"
#include <stdlib.h>

typedef struct HashNode HashNode;

struct HashNode {
    void* key;
    void* value;
    HashNode* next;
};

struct HashMap {
    HashNode** buckets;
    size_t size;
    size_t capacity;
    HashFunction hashFunc;
    EqualityFunction keysEqualFunc;
};

static int is_prime(size_t n)
{
    if (n < 2) return 0;
    for (size_t i = 2; i <= n / i; i++) {
        if (n % i == 0) return 0;
    }
    return 1;
}

static size_t next_prime(size_t n)
{
    while (!is_prime(n)) {
        if (n == (size_t)-1) return n;
        ++n;
    }
    return n;
}

HashMap* HashMap_Create(size_t _capacity, HashFunction _hashFunc, EqualityFunction _keysEqualFunc)
{
    if (!_hashFunc || !_keysEqualFunc) return NULL;
    HashMap* map = (HashMap*)malloc(sizeof(HashMap));
    if (!map) return NULL;
    map->capacity = next_prime(_capacity < 3 ? 3 : _capacity);
    map->buckets = (HashNode**)calloc(map->capacity, sizeof(HashNode*));
    if (!map->buckets) {
        free(map);
        return NULL;
    }
    map->size = 0;
    map->hashFunc = _hashFunc;
    map->keysEqualFunc = _keysEqualFunc;
    return map;
}

void HashMap_Destroy(HashMap** _map, void (*_keyDestroy)(void* _key), void (*_valDestroy)(void* _value))
{
    if (!_map || !*_map) return;
    HashMap* map = *_map;
    for (size_t i = 0; i < map->capacity; i++) {
        HashNode* cur = map->buckets[i];
        while (cur) {
            HashNode* next = cur->next;
            if (_keyDestroy) _keyDestroy(cur->key);
            if (_valDestroy) _valDestroy(cur->value);
            free(cur);
            cur = next;
        }
    }
    free(map->buckets);
    free(map);
    *_map = NULL;
}

Map_Result HashMap_Insert(HashMap* _map, const void* _key, const void* _value)
{
    if (!_map) return MAP_UNINITIALIZED_ERROR;
    if (!_key) return MAP_KEY_NULL_ERROR;

    size_t idx = _map->hashFunc((void*)_key) % _map->capacity;
    HashNode* cur = _map->buckets[idx];
    while (cur) {
        if (_map->keysEqualFunc(cur->key, (void*)_key))
            return MAP_KEY_DUPLICATE_ERROR;
        cur = cur->next;
    }

    HashNode* node = (HashNode*)malloc(sizeof(HashNode));
    if (!node) return MAP_ALLOCATION_ERROR;
    node->key = (void*)_key;
    node->value = (void*)_value;
    node->next = _map->buckets[idx];
    _map->buckets[idx] = node;
    _map->size++;
    return MAP_SUCCESS;
}

Map_Result HashMap_Remove(HashMap* _map, const void* _searchKey, void** _pKey, void** _pValue)
{
    if (!_map) return MAP_UNINITIALIZED_ERROR;
    if (!_searchKey) return MAP_KEY_NULL_ERROR;

    size_t idx = _map->hashFunc((void*)_searchKey) % _map->capacity;
    HashNode** prev = &_map->buckets[idx];
    HashNode* cur = _map->buckets[idx];
    while (cur) {
        if (_map->keysEqualFunc(cur->key, (void*)_searchKey)) {
            *prev = cur->next;
            if (_pKey) *_pKey = cur->key;
            if (_pValue) *_pValue = cur->value;
            free(cur);
            _map->size--;
            return MAP_SUCCESS;
        }
        prev = &cur->next;
        cur = cur->next;
    }
    return MAP_KEY_NOT_FOUND_ERROR;
}

Map_Result HashMap_Find(const HashMap* _map, const void* _key, void** _pValue)
{
    if (!_map) return MAP_UNINITIALIZED_ERROR;
    if (!_key) return MAP_KEY_NULL_ERROR;

    size_t idx = _map->hashFunc((void*)_key) % _map->capacity;
    HashNode* cur = _map->buckets[idx];
    while (cur) {
        if (_map->keysEqualFunc(cur->key, (void*)_key)) {
            if (_pValue) *_pValue = cur->value;
            return MAP_SUCCESS;
        }
        cur = cur->next;
    }
    return MAP_KEY_NOT_FOUND_ERROR;
}

size_t HashMap_Size(const HashMap* _map)
{
    if (!_map) return 0;
    return _map->size;
}

Map_Result HashMap_Rehash(HashMap* _map, size_t newCapacity)
{
    if (!_map) return MAP_UNINITIALIZED_ERROR;

    size_t newCap = next_prime(newCapacity < 3 ? 3 : newCapacity);
    HashNode** newBuckets = (HashNode**)calloc(newCap, sizeof(HashNode*));
    if (!newBuckets) return MAP_ALLOCATION_ERROR;

    for (size_t i = 0; i < _map->capacity; i++) {
        HashNode* cur = _map->buckets[i];
        while (cur) {
            HashNode* next = cur->next;
            size_t idx = _map->hashFunc(cur->key) % newCap;
            cur->next = newBuckets[idx];
            newBuckets[idx] = cur;
            cur = next;
        }
    }

    free(_map->buckets);
    _map->buckets = newBuckets;
    _map->capacity = newCap;
    return MAP_SUCCESS;
}

size_t HashMap_ForEach(const HashMap* _map, KeyValueActionFunction _action, void* _context)
{
    if (!_map || !_action) return 0;
    size_t count = 0;
    for (size_t i = 0; i < _map->capacity; i++) {
        HashNode* cur = _map->buckets[i];
        while (cur) {
            if (!_action(cur->key, cur->value, _context)) return count + 1;
            count++;
            cur = cur->next;
        }
    }
    return count;
}

#ifndef NDEBUG

Map_Stats HashMap_GetStatistics(const HashMap* _map)
{
    Map_Stats stats = {0, 0, 0, 0};
    if (!_map) return stats;
    stats.numberOfBuckets = _map->capacity;
    size_t totalChainLen = 0;
    for (size_t i = 0; i < _map->capacity; i++) {
        size_t len = 0;
        HashNode* cur = _map->buckets[i];
        while (cur) {
            len++;
            cur = cur->next;
        }
        if (len > 0) {
            stats.numberOfChains++;
            if (len > stats.maxChainLength) stats.maxChainLength = len;
            totalChainLen += len;
        }
    }
    stats.averageChainLength = stats.numberOfChains > 0
        ? totalChainLen / stats.numberOfChains : 0;
    return stats;
}

#endif /* NDEBUG */
