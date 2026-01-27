#include "../include/hash_table.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

#define FREE(ptr) free(ptr); ptr = NULL;
#define MIN(a, b) ((a) < (b)) ? (a) : (b)

const int    EXP_MUL          = 2;
const int    INITIAL_CAPACITY = 8;
const size_t MAX_CAPACITY     = 100000;
const double MAX_LOAD_FACTOR  = 0.75;
const double MIN_LOAD_FACTOR  = 0.25;

static HashNode* HashNodeInit(const void* ukey, size_t ukey_size, 
                              const void* udata, size_t udata_size,
                              HashNode* uprev, HashNode* unext);
static HashTableErr_t HashNodeDestroy(HashNode** node_ptr);

static HashTableErr_t HashTableRealloc(HashTable_t* table, size_t new_capacity);

static int KeyCmp(const void *key1, size_t key1_size, const void *key2, size_t key2_size);

static uint32_t fnv_hash_to_index(const void* ukey, size_t ukey_size, uint32_t table_capacity);
static uint64_t fnv1a_hash(const void* ukey, size_t ukey_size);
/*
static uint32_t murmurhash2_to_index(const void* ukey, size_t ukey_size, uint32_t table_capacity);
static uint32_t murmurhash2(const void* ukey, size_t ukey_size);
*/
/* If the user's ukey_size is incorrect, global-buffer-overflow may occur in the sha256_update src/sha256.c:118 */

HashTable_t* HashTableInit() {
    HashTable_t* table = (HashTable_t*)calloc(1, sizeof(HashTable_t));
    if (table == NULL) {
        return NULL;
    }

    table->buckets_size = 0;
    table->buckets_capacity = INITIAL_CAPACITY;
    table->buckets = (HashNode**)calloc(INITIAL_CAPACITY, sizeof(HashNode*));
    if (table->buckets == NULL) {
        FREE(table);
        table->buckets_capacity = 0;
        return NULL;
    }

    table->element_cnt = 0;

    return table;
}

static HashNode* HashNodeInit(const void* ukey, size_t ukey_size, 
                              const void* udata, size_t udata_size,
                              HashNode* uprev, HashNode* unext) {
    assert( ukey != NULL );
    assert( udata != NULL );

    HashNode* node = (HashNode*)calloc(1, sizeof(HashNode));
    if (node == NULL) {
        return NULL;
    }

    node->next = unext;
    node->prev = uprev;
    node->key_size = ukey_size;
    node->data_size = udata_size;

    if (ukey != NULL) {
        node->key = calloc(1, ukey_size);
        memcpy(node->key, ukey, ukey_size);
    }

    if (udata != NULL) {
        node->data = calloc(1, udata_size);
        memcpy(node->data, udata, udata_size);
    }
    
    return node;
}

static HashTableErr_t HashNodeDestroy(HashNode** node_ptr) {
    assert( node_ptr != NULL );

    HashNode* node = *node_ptr;

    node->next = NULL;
    node->prev = NULL;

    node->data_size = 0;
    FREE(node->data); /* if (node->data) LocalDataDestroy() */

    node->key_size = 0;
    FREE(node->key);

    FREE(*node_ptr);

    return HASH_TABLE_OK;
}

static HashTableErr_t HashTableRealloc(HashTable_t* table, size_t new_capacity) {
    assert( table != NULL );
    assert( INITIAL_CAPACITY <= new_capacity && new_capacity <= MAX_CAPACITY );

    HashNode** new_buckets = (HashNode**)calloc(new_capacity, sizeof(HashNode*));
    if (new_buckets == NULL) {
        fprintf(stderr, "HashTableRehash: new_buckets = NULL\n");
        return HASH_TABLE_BUCKETS_OVERFLOW;
    }

    size_t new_element_cnt = 0;

    for (size_t i = 0; i < table->buckets_capacity; i++) {
        HashNode* cur_node = table->buckets[i];

        while (cur_node != NULL) {
            HashNode* next_node = cur_node->next;

            size_t new_index = fnv_hash_to_index(cur_node->key, cur_node->key_size, 
                                                  (uint32_t)new_capacity);

            cur_node->next = new_buckets[new_index];
            cur_node->prev = NULL;
            new_buckets[new_index] = cur_node;

            if (cur_node->next != NULL) {
                cur_node->next->prev = cur_node;
            }
            
            cur_node = next_node;
            ++new_element_cnt;
        }
    }

    FREE(table->buckets);
    table->buckets = new_buckets;
    table->buckets_capacity = new_capacity;

    if (new_element_cnt != table->element_cnt) {
        fprintf(stderr, "HashTableRehash: new_buckets_size != table->buckets_size\n");
        return HASH_TABLE_NODE_MEMORY_LEAK;
    }

    return HASH_TABLE_OK;
}

HashTableErr_t HashTableDestroy(HashTable_t** table_ptr) {
    assert( table_ptr != NULL );

    HashTable_t* table = *table_ptr;

    for (size_t i = 0; i < table->buckets_capacity; i++) {
        HashNode* cur_node = table->buckets[i];

        while (cur_node != NULL) {
            HashNode* next_node = cur_node->next;

            HashNodeDestroy(&cur_node);

            cur_node = next_node;
        }

        table->buckets[i] = NULL;
    }

    FREE(table->buckets);

    table->buckets_size = 0;
    table->buckets_capacity = 0;
    table->element_cnt = 0;

    FREE(*table_ptr);

    return HASH_TABLE_OK;
}

size_t HashTableSize(HashTable_t* table) {
    assert( table != NULL );

    return table->element_cnt;
}

int HashTableEmpty(HashTable_t* table) {
    assert( table != NULL );

    return table->element_cnt == 0;
}

void* HashTableFind(HashTable_t* table, const void* ukey, size_t ukey_size) {
    assert( table != NULL );
    assert( ukey != NULL );

    size_t key_idx = fnv_hash_to_index(ukey, ukey_size, (uint32_t)table->buckets_capacity);

    HashNode* cur_node = table->buckets[key_idx];

    while (cur_node != NULL) {
        if (KeyCmp(ukey, ukey_size, cur_node->key, cur_node->key_size) == 0) {
            return cur_node->data;
        }

        cur_node = cur_node->next;
    }

    return NULL;
}

HashTableErr_t HashTableInsert(HashTable_t* table, const void* ukey, size_t ukey_size, 
                                                   const void* udata, size_t udata_size) {
    assert( table != NULL );
    assert( ukey != NULL );
    assert( udata != NULL );

    size_t key_idx = fnv_hash_to_index(ukey, ukey_size, (uint32_t)table->buckets_capacity);

    HashNode* prev_node = NULL;
    HashNode* cur_node = table->buckets[key_idx];
    HashNode** cur_node_next_ptr = &table->buckets[key_idx]; /* this strange var is for base case */

    while (cur_node != NULL) {
        if (KeyCmp(ukey, ukey_size, cur_node->key, cur_node->key_size) == 0) {
            FREE(cur_node->data);
            
            cur_node->data = calloc(1, udata_size);
            cur_node->data_size = udata_size;

            memcpy(cur_node->data, udata, udata_size);

            return HASH_TABLE_OK;
        }

        prev_node = cur_node;
        cur_node = cur_node->next;
        cur_node_next_ptr = &prev_node->next;
    }

    *cur_node_next_ptr = HashNodeInit(ukey, ukey_size, udata, udata_size, prev_node, NULL);
    if (*cur_node_next_ptr == NULL) {
        fprintf(stderr, "HashTableInsert: *cur_node_next_ptr == NULL\n");
        return HASH_TABLE_CANT_CALLOC_NODE;
    }

    if (prev_node == NULL && cur_node == NULL) {
        ++table->buckets_size;
    }
    ++table->element_cnt;

    if ((double)table->buckets_size / (double)table->buckets_capacity >= MAX_LOAD_FACTOR) {
        size_t new_capacity = table->buckets_capacity * EXP_MUL;
        if (new_capacity <= MAX_CAPACITY) {
            HashTableRealloc(table, new_capacity);
        }
    }

    return HASH_TABLE_OK;
}

int HashTableErase(HashTable_t* table, const void* ukey, size_t ukey_size) {
    assert( table != NULL );
    assert( ukey != NULL );

    size_t key_idx = fnv_hash_to_index(ukey, ukey_size, (uint32_t)table->buckets_capacity);

    HashNode* cur_node = table->buckets[key_idx];
    HashNode* prev_node = NULL;
    HashNode** prev_node_next = &table->buckets[key_idx]; /* this strange var is for base case */

    while (cur_node != NULL) {
        if (KeyCmp(ukey, ukey_size, cur_node->key, cur_node->key_size) == 0) {
            HashNode* next_node = cur_node->next;
            if (prev_node == NULL && next_node == NULL) {
                --table->buckets_size;
            }

            HashNodeDestroy(prev_node_next);
            *prev_node_next = next_node;
            
            if (next_node) {
                next_node->prev = prev_node;
            }
            --table->element_cnt;

            if ((double)table->buckets_size / (double)table->buckets_capacity <= MIN_LOAD_FACTOR) {
                size_t new_capacity = table->buckets_capacity / EXP_MUL;
                if (new_capacity >= INITIAL_CAPACITY) {
                    HashTableRealloc(table, new_capacity);
                }
            }

            return 1;
        }

        prev_node = cur_node;
        cur_node = cur_node->next;
        prev_node_next = &prev_node->next;
    }

    return 0;
}

static int KeyCmp(const void *key1, size_t key1_size, const void *key2, size_t key2_size) {
    size_t min_size = MIN(key1_size, key2_size);
    return memcmp(key1, key2, min_size);
}

static uint32_t fnv_hash_to_index(const void* ukey, size_t ukey_size, uint32_t table_capacity) {
    assert(ukey != NULL);
    assert(ukey_size > 0);
    assert(table_capacity > 0);
    
    uint64_t hash = fnv1a_hash(ukey, ukey_size);

    // multiply-shift method
    const uint64_t A = 11400714819323198485ULL; /* knuth (sqrt(5)-1)/2 * 2^64 */

    if ((table_capacity & (table_capacity - 1)) == 0) {
        uint64_t multiplied = hash * A;
        return (uint32_t)(multiplied >> (64 - __builtin_ctz(table_capacity)));
    } else {
        return (uint32_t)((hash * A) % table_capacity);
    }
}

static uint64_t fnv1a_hash(const void* ukey, size_t ukey_size) {
    if (ukey == NULL || ukey_size == 0) {
        return 0;
    }
    
    const uint8_t* data = (const uint8_t*)ukey;
    uint64_t hash = 0xCBF29CE484222325ULL;
    
    for (size_t i = 0; i < ukey_size; i++) {
        hash ^= data[i];
        hash *= 0x100000001B3ULL;
    }
    
    return hash;
}
/*
static uint32_t murmurhash2_to_index(const void* ukey, size_t ukey_size, uint32_t table_capacity) {
    assert(ukey != NULL);
    assert(ukey_size > 0);
    assert(table_capacity > 0);
    
    uint64_t hash = murmurhash2(ukey, ukey_size);

    // multiply-shift method
    const uint64_t A = 11400714819323198485ULL; // knuth (sqrt(5)-1)/2 * 2^64

    if ((table_capacity & (table_capacity - 1)) == 0) {
        uint64_t multiplied = hash * A;
        return (uint32_t)(multiplied >> (64 - __builtin_ctz(table_capacity)));
    } else {
        return (uint32_t)((hash * A) % table_capacity);
    }
}

static uint32_t murmurhash2(const void* ukey, size_t ukey_size) {
    const uint32_t m = 0x5bd1e995;
    const uint32_t seed = 0;
    const int r = 24;

    uint32_t h = seed ^ (uint32_t)ukey_size;

    const unsigned char* data = (const unsigned char *)ukey;
    
    while (ukey_size >= 4) {
        uint32_t k = 0;
        memcpy(&k, data, sizeof(k));

        k *= m;
        k ^= k >> r;
        k *= m;
        
        h *= m;
        h ^= k;
        
        data += 4;
        ukey_size -= 4;
    }
    
    switch (ukey_size) {
        case 3: h ^= data[2] << 16;
        case 2: h ^= data[1] << 8;
        case 1: h ^= data[0];
                h *= m;
    };
    
    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;
    
    return h;
}
*/