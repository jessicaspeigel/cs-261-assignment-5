#include "hashMap.h"
#include <assert.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MIN3(a, b, c) ((a) < (b) ? ((a) < (c) ? (a) : (c)) : ((b) < (c) ? (b) : (c)))
const int NUM_SUGGESTIONS = 5;

/**
 * Allocates a string for the next word in the file and returns it. This string
 * is null terminated. Returns NULL after reaching the end of the file.
 * @param file
 * @return Allocated string or NULL.
 */
char* nextWord(FILE* file)
{
    int maxLength = 16;
    int length = 0;
    char* word = malloc(sizeof(char) * maxLength);
    while (1)
    {
        char c = fgetc(file);
        if ((c >= '0' && c <= '9') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z') ||
            c == '\'')
        {
            if (length + 1 >= maxLength)
            {
                maxLength *= 2;
                word = realloc(word, maxLength);
            }
            word[length] = (char) tolower(c);
            length++;
        }
        else if (length > 0 || c == EOF)
        {
            break;
        }
    }
    if (length == 0)
    {
        free(word);
        return NULL;
    }
    word[length] = '\0';
    return word;
}

/**
 * Loads the contents of the file into the hash map.
 * @param file
 * @param map
 */
void loadDictionary(FILE* file, HashMap* map)
{
    assert(file != NULL);
    assert(map != NULL);

    // Load the words into the dictionary
    char *word = nextWord(file);
    while (word != NULL) {
        hashMapPut(map, word, -1);
        free(word);
        word = nextWord(file);
    }
}

/**
 * Takes user-entered input and returns a lowercase version. Returns NULL if invalid input.
 * @param input
 * @return lowercase string, NULL if an invalid character is found
 */
char* validateInput(char *input) {
    int maxLength = 256;
    int length = 0;
    char* word = malloc(sizeof(char) * maxLength);
    while (length < maxLength)
    {
        char c = input[length];
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))
        {
            word[length] = (char) tolower(c);
            length++;
            if (input[length] == '\0') {
                break;
            }
        } else {
            free(word);
            return NULL;
        }
    }
    word[length] = '\0';
    return word;
}

/**
 * Calculates the Levenshtein distance and returns it.
 * Adapted from: https://en.wikibooks.org/wiki/Algorithm_Implementation/Strings/Levenshtein_distance#C
 * @param s1 and s2, two strings to compare
 * @return an int containing the Levenshtein distance.
 */
int computeLevenshtein(char *s1, char *s2) {
    unsigned int s1len, s2len, x, y, lastdiag, olddiag;
    s1len = strlen(s1);
    s2len = strlen(s2);
    unsigned int column[s1len+1];
    for (y = 1; y <= s1len; y++)
        column[y] = y;
    for (x = 1; x <= s2len; x++) {
        column[0] = x;
        for (y = 1, lastdiag = x-1; y <= s1len; y++) {
            olddiag = column[y];
            column[y] = MIN3(column[y] + 1, column[y-1] + 1, lastdiag + (s1[y-1] == s2[x-1] ? 0 : 1));
            lastdiag = olddiag;
        }
    }
    return(column[s1len]);
}

/**
 * Prints the concordance of the given file and performance information. Uses
 * the file input1.txt by default or a file name specified as a command line
 * argument.
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, const char** argv)
{
    HashMap* map = hashMapNew(1000);
    
    FILE* file = fopen("dictionary.txt", "r");
    clock_t timer = clock();
    loadDictionary(file, map);
    timer = clock() - timer;
    printf("Dictionary loaded in %f seconds\n", (float)timer / (float)CLOCKS_PER_SEC);
    fclose(file);
    
    char inputBuffer[256];
    int quit = 0;
    while (!quit)
    {

        // --- Spellchecker code begins here ---

        printf("Enter a word or \"quit\" to quit:\n");
        scanf("%s", inputBuffer);
        // Validate the input and convert it to lowercase
        char *word = validateInput(inputBuffer);
        // See if the user wants to quit
        if (strcmp(inputBuffer, "quit") == 0)
        {
            quit = 1;
        }

        // Loop until we have valid input
        while (word == NULL) {
            // Prompt the user for a valid word
            printf("I'm sorry, that's an invalid entry. Please enter a single word, containing only uppercase or lowercase letters:\n");
            scanf("%s", inputBuffer);
            // See if the user wants to quit
            if (strcmp(inputBuffer, "quit") == 0)
            {
                quit = 1;
            }
            // Validate the input is valid
            word = validateInput(inputBuffer);
        }

        // If the user hasn't chosen to quit, spell check the word
        if (!quit) {
            printf("Checking for a match...\n");

            if (hashMapGet(map, word) == NULL) {
                // The word wasn't found
                printf("The inputted word \"%s\" is spelled incorrectly.\n", word);
                // Create an array to hold the suggestions
                struct HashLink** suggestions = malloc(NUM_SUGGESTIONS * sizeof(struct HashLink*));
                for(int s = 0; s < NUM_SUGGESTIONS; s++) {
                    suggestions[s] = NULL;
                }
                // Loop through the dictionary, calculating the Levenshtein distance for each entry
                for (int i = 0; i < hashMapCapacity(map); i++) {
                    struct HashLink *currentLink = map->table[i];
                    if (currentLink != NULL) {
                        // Loop through the links in the bucket
                        while (currentLink != NULL) {
                            // Calculate the distance for the current link
                            int distance = computeLevenshtein(word, currentLink->key);
                            // Update the link
                            hashMapPut(map, currentLink->key, distance);
                            // See if we should add it to the suggestions
                            int smallestDistance = 100000;
                            int smallestDistanceIndex = -1;
                            for (int s = 0; s < NUM_SUGGESTIONS; s++) {
                                // If any link is NULL add it and break out of the loop
                                if (suggestions[s] == NULL) {
                                    suggestions[s] = malloc(sizeof(struct Hashlink*));
                                    suggestions[s]->key = currentLink->key;
                                    suggestions[s]->value = currentLink->value;
                                    suggestions[s]->next = NULL;
                                    break;
                                } else if (currentLink->value < suggestions[s]->value &&
                                           suggestions[s]->value < smallestDistance) {
                                    smallestDistance = suggestions[s]->value;
                                    smallestDistanceIndex = s;
                                }
                            }
                            // Add the item only if its distance is smaller than the smallest suggestion's distance
                            if (smallestDistanceIndex > -1) {
                                suggestions[smallestDistanceIndex]->key = currentLink->key;
                                suggestions[smallestDistanceIndex]->value = currentLink->value;
                            }
                            // Move to the next link
                            currentLink = currentLink->next;
                        }
                    }
                }
                // Print the list of suggestions
                printf("Did you mean...?\n");
                for (int s = 0; s < NUM_SUGGESTIONS; s++) {
                    //printf("%s (distance: %i)\n", suggestions[s]->key, suggestions[s]->value);
                    printf("%s\n", suggestions[s]->key);
                    free(suggestions[s]);
                }
                free(suggestions);
            } else {
                // The word was found
                printf("The inputted word \"%s\" is spelled correctly.\n", word);
            }

            /*
             *  Step 1: Compare input buffer to words in the dictionary, computing their Levenshtein distance.
             *  Store that distance as the value for each key in the table.
             *
             *  Step 2: Traverse down the hash table, checking each bucket. Jump out if you find an exact
             *  matching dictionary word. and print a message that "word spelled correctly".
             *
             *  Step 3: f the input Buffer did not match any of the dictionary words exactly, generate an array of 5
             *  words that are closest matches to input Buffer based on the lowest Levenshtein distance. Print the
             *  array including the messages "word spelled incorrectly", " Did you mean .... ? ".
             */
        }
        free(word);
        // --- Spellchecker code ends here ---
    }
    hashMapDelete(map);
    return 0;
}