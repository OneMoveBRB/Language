#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <stddef.h>

typedef struct HashNode {
    void* key;
    void* data;
    size_t key_size;
    size_t data_size;
    struct HashNode* next;
    struct HashNode* prev;
} HashNode;

typedef struct {
    HashNode** buckets;
    size_t buckets_capacity;
    size_t buckets_size;
    size_t element_cnt;
} HashTable_t;

typedef enum HashTableErr_t {
    HASH_TABLE_OK,
    HASH_TABLE_CANT_CALLOC_NODE,
    HASH_TABLE_BUCKETS_OVERFLOW,
    HASH_TABLE_NODE_MEMORY_LEAK
} HashTableErr_t;

HashTable_t* HashTableInit();
HashTableErr_t HashTableDestroy(HashTable_t** table_ptr);

size_t HashTableSize(HashTable_t* table);
int HashTableEmpty(HashTable_t* table);

void* HashTableFind(HashTable_t* table, const void* ukey, size_t ukey_size);
HashTableErr_t HashTableInsert(HashTable_t* table, const void* ukey, size_t ukey_size, 
                                                   const void* udata, size_t udata_size);
int HashTableErase(HashTable_t* table, const void* ukey, size_t ukey_size);

#endif /* HASH_TABLE_H */