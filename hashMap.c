/*
 * CS 261 Data Structures
 * Assignment 5
 * Name: Jessica Speigel
 * Date: 08/10/2018
 */

#include "hashMap.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

int hashFunction1(const char *key) {
    int r = 0;
    for (int i = 0; key[i] != '\0'; i++) {
        r += key[i];
    }
    return r;
}

int hashFunction2(const char *key) {
    int r = 0;
    for (int i = 0; key[i] != '\0'; i++) {
        r += (i + 1) * key[i];
    }
    return r;
}

/**
 * Creates a new hash table link with a copy of the key string.
 * @param key Key string to copy in the link.
 * @param value Value to set in the link.
 * @param next Pointer to set as the link's next.
 * @return Hash table link allocated on the heap.
 */
HashLink *hashLinkNew(const char *key, int value, HashLink *next) {
    HashLink *link = malloc(sizeof(HashLink));
    link->key = malloc(sizeof(char) * (strlen(key) + 1));
    strcpy(link->key, key);
    link->value = value;
    link->next = next;
    return link;
}

/**
 * Free the allocated memory for a hash table link created with hashLinkNew.
 * @param link
 */
static void hashLinkDelete(HashLink *link) {
    free(link->key);
    free(link);
}

/**
 * Initializes a hash table map, allocating memory for a link pointer table with
 * the given number of buckets.
 * @param map
 * @param capacity The number of table buckets.
 */
void hashMapInit(HashMap *map, int capacity) {
    map->capacity = capacity;
    map->size = 0;
    map->table = malloc(sizeof(HashLink *) * capacity);
    for (int i = 0; i < capacity; i++) {
        map->table[i] = NULL;
    }
}

/**
 * Removes all links in the map and frees all allocated memory. You can use
 * hashLinkDelete to free the links.
 * @param map
 */
void hashMapCleanUp(HashMap *map) {
    assert(map != NULL);
    for (int i = 0; i < hashMapCapacity(map); i++) {
        // Loop through the buckets
        if (map->table[i] != NULL) {
            // Delete the links in the bucket
            struct HashLink *currentLink = map->table[i];
            struct HashLink *nextLink;
            while (currentLink != NULL) {
                // Loop through the links in the bucket and remove them
                nextLink = currentLink->next;
                hashLinkDelete(currentLink);
                currentLink = nextLink;
            }
        }
    }
    free(map->table);
}

/**
 * Creates a hash table map, allocating memory for a link pointer table with
 * the given number of buckets.
 * @param capacity The number of buckets.
 * @return The allocated map.
 */
HashMap *hashMapNew(int capacity) {
    HashMap *map = malloc(sizeof(HashMap));
    hashMapInit(map, capacity);
    return map;
}

/**
 * Removes all links in the map and frees all allocated memory, including the
 * map itself.
 * @param map
 */
void hashMapDelete(HashMap *map) {
    assert(map != NULL);
    hashMapCleanUp(map);
    free(map);
}

/**
 * Returns a pointer to the value of the link with the given key and skip traversing as well. Returns NULL
 * if no link with that key is in the table.
 * 
 * Use HASH_FUNCTION(key) and the map's capacity to find the index of the
 * correct linked list bucket. Also make sure to search the entire list.
 * 
 * @param map
 * @param key
 * @return Link value or NULL if no matching link.
 */
int *hashMapGet(HashMap *map, const char *key) {
    assert(map != NULL);
    assert(key != NULL);

    int *returnValue = NULL;

    int capacity = hashMapCapacity(map);
    // Compute the hash value to find the correct bucket
    int hashIndex = HASH_FUNCTION(key) % capacity;
    if (hashIndex < 0) {
        hashIndex += capacity;
    }

    // Check to see if the key exists in the table in the bucket it hashes to
    struct HashLink *currentLink = map->table[hashIndex];
    while (currentLink != NULL) {
        if (strcmp(currentLink->key, key) == 0) {
            // Update the returnValue
            returnValue = &currentLink->value;
            break;
        }
        currentLink = currentLink->next;
    }

    return returnValue;
}

/**
 * Resizes the hash table to have a number of buckets equal to the given 
 * capacity (double of the old capacity). After allocating the new table, 
 * all of the links need to rehashed into it because the capacity has changed.
 * 
 * Remember to free the old table and any old links if you use hashMapPut to
 * rehash them.
 * 
 * @param map
 * @param capacity The new number of buckets.
 */
void resizeTable(HashMap *map, int capacity) {
    assert(map != NULL);
    assert(capacity > hashMapCapacity(map));

    // Create a new table with the new number of buckets
    struct HashMap *newMap = hashMapNew(capacity);

    // Loop through all the buckets and links to rehash the values
    struct HashLink *currentLink;
    for (int i = 0; i < hashMapCapacity(map); i++) {
        // Loop through the buckets
        currentLink = map->table[i];
        while (currentLink != NULL) {
            // Add the items to the new table
            hashMapPut(newMap, currentLink->key, currentLink->value);
            currentLink = currentLink->next;
        }
    }
    // Delete the old table
    hashMapCleanUp(map);
    // Set the old map table to the new map table and update the capacity
    map->table = newMap->table;
    map->capacity = capacity;
    // Delete the temporary map
    free(newMap);
}

/**
 * Updates the given key-value pair in the hash table. If a link with the given
 * key already exists, this will just update the value and skip traversing. Otherwise, it will
 * create a new link with the given key and value and add it to the table
 * bucket's linked list. You can use hashLinkNew to create the link.
 * 
 * Use HASH_FUNCTION(key) and the map's capacity to find the index of the
 * correct linked list bucket.
 * 
 * @param map
 * @param key
 * @param value
 */
void hashMapPut(HashMap *map, const char *key, int value) {
    assert(map != NULL);
    assert(key != NULL);

    int capacity = hashMapCapacity(map);
    // Compute the hash value to find the correct bucket
    int hashIndex = HASH_FUNCTION(key) % capacity;
    if (hashIndex < 0) {
        hashIndex += capacity;
    }

    // Check to see if the key exists in the table in the bucket it hashes to
    struct HashLink *currentLink = map->table[hashIndex];
    while (currentLink != NULL) {
        //printf("Looking for key: %s // Current key: %s\n", key, currentLink->key);
        if (strcmp(currentLink->key, key) == 0) {
            //printf("Found! Updating value found at map->table[%i], key: %s, to new value: %i (old value: %i)\n", hashIndex, currentLink->key, value, currentLink->value);
            // Update the value and exit the function
            currentLink->value = value;
            return;
        }
        currentLink = currentLink->next;
    }

    // This code only executes if a matching link wasn't found
    // Create the new link and add it to the bucket
    HashLink *newLink = hashLinkNew(key, value, map->table[hashIndex]);
    assert(newLink);
    map->table[hashIndex] = newLink;
    map->size++;
    //printf("map->table[%i] // key: %c, value: %i, next: %p\n", hashIndex, *map->table[hashIndex]->key, map->table[hashIndex]->value, map->table[hashIndex]->next);

    // Check to see if a resize is necessary
    if (hashMapTableLoad(map) > MAX_TABLE_LOAD) {
        resizeTable(map, hashMapCapacity(map) * 2);
    }

}

/**
 * Removes and frees the link with the given key from the table. If no such link
 * exists, this does nothing. Remember to search the entire linked list at the
 * bucket. You can use hashLinkDelete to free the link.
 * @param map
 * @param key
 */
void hashMapRemove(HashMap *map, const char *key) {
    assert(map != NULL);
    assert(key != NULL);

    int capacity = hashMapCapacity(map);
    // Compute the hash value to find the correct bucket
    int hashIndex = HASH_FUNCTION(key) % capacity;
    if (hashIndex < 0) {
        hashIndex += capacity;
    }

    // Check to see if the key exists in the table in the bucket it hashes to
    struct HashLink *currentLink = map->table[hashIndex];
    struct HashLink *lastLink = NULL;
    while (currentLink != NULL) {
        if (strcmp(currentLink->key, key) == 0) {
            if (lastLink == NULL) {
                // If the key is found at first entry, set beginning to the next entry
                map->table[hashIndex] = currentLink->next;
            } else {
                // The value is in the middle, link lastLink and next
                lastLink->next = currentLink->next;
            }

            // Remove the link
            hashLinkDelete(currentLink);
            map->size--;
            return;
        }
        lastLink = currentLink;
        currentLink = currentLink->next;
    }
}

/**
 * Returns 1 if a link with the given key is in the table and 0 otherwise.
 * 
 * Use HASH_FUNCTION(key) and the map's capacity to find the index of the
 * correct linked list bucket. Also make sure to search the entire list.
 * 
 * @param map
 * @param key
 * @return 1 if the key is found, 0 otherwise.
 */
int hashMapContainsKey(HashMap *map, const char *key) {
    assert(map != NULL);
    assert(key != NULL);

    int containsKey = 0;

    int capacity = hashMapCapacity(map);
    // Compute the hash value to find the correct bucket
    int hashIndex = HASH_FUNCTION(key) % capacity;
    if (hashIndex < 0) {
        hashIndex += capacity;
    }

    // Check to see if the key exists in the table in the bucket it hashes to
    struct HashLink *currentLink = map->table[hashIndex];
    while (currentLink != NULL) {
        if (strcmp(currentLink->key, key) == 0) {
            // Update the returnValue
            containsKey = 1;
            break;
        }
        currentLink = currentLink->next;
    }

    return containsKey;
}

/**
 * Returns the number of links in the table.
 * @param map
 * @return Number of links in the table.
 */
int hashMapSize(HashMap *map) {
    assert(map != NULL);
    return map->size;
}

/**
 * Returns the number of buckets in the table.
 * @param map
 * @return Number of buckets in the table.
 */
int hashMapCapacity(HashMap *map) {
    assert(map != NULL);
    return map->capacity;
}

/**
 * Returns the number of table buckets without any links.
 * @param map
 * @return Number of empty buckets.
 */
int hashMapEmptyBuckets(HashMap *map) {
    assert(map != NULL);
    int emptyBuckets = 0;
    for (int i = 0; i < hashMapCapacity(map); i++) {
        // Loop through the buckets
        if (map->table[i] == NULL) {
            emptyBuckets++;
        }
    }
    return emptyBuckets;
}

/**
 * Returns the ratio of (number of links) / (number of buckets) in the table.
 * Remember that the buckets are linked lists, so this ratio tells you nothing
 * about the number of empty buckets. Remember also that the load is a floating
 * point number, so don't do integer division.
 * @param map
 * @return Table load.
 */
float hashMapTableLoad(HashMap *map) {
    assert (map != NULL);
    assert(hashMapCapacity(map) > 0);
    return (float) hashMapSize(map) / hashMapCapacity(map);
}

/**
 * Prints all the links in each of the buckets in the table.
 * @param map
 */
void hashMapPrint(HashMap *map) {
    assert(map != NULL);
    for (int i = 0; i < hashMapCapacity(map); i++) {
        // Loop through the buckets
        struct HashLink *currentLink = map->table[i];
        if (currentLink != NULL) {
            printf("\nBucket %i -> ", i);
            while (currentLink != NULL) {
                // Loop through the links in the bucket
                printf("(%s, %i) -> ", currentLink->key, currentLink->value);
                currentLink = currentLink->next;
            }
        }
    }
}
