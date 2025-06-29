#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "BDD.h"


// Creating result nodes
void createResultNodes(BDD *bdd) {
        //Result node true
        bdd->resultTrue = (BDDNode*)malloc(sizeof(BDDNode));
        bdd->resultTrue->variable = '1';
        bdd->resultTrue->leafTrue = NULL;
        bdd->resultTrue->leafFalse = NULL;
        bdd->resultTrue->next = NULL;

        //Result node false
        bdd->resultFalse = (BDDNode*)malloc(sizeof(BDDNode));
        bdd->resultFalse->variable = '0';
        bdd->resultFalse->leafTrue = NULL;
        bdd->resultFalse->leafFalse = NULL;
        bdd->resultFalse->next = NULL;
}

// Hashing node
unsigned int hashNode(char n, BDDNode *leafTrue, BDDNode *leafFalse) {
    unsigned long hash = 5381;
    hash = ((hash << 5) + hash) + n;
    hash = ((hash << 5) + hash) + (unsigned long)leafTrue;
    hash = ((hash << 5) + hash) + (unsigned long)leafFalse;
    return hash % HASH_TABLE_SIZE;
}

// Creating hash table
void hashTableCreate(NodeHashTable *nodeTable) {
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        nodeTable->table[i] = NULL;
    }
    nodeTable->count = 0;
}

// Decide true or false leaf
int leafDecide(const char *function, int *vars) {
    char *copy = strdup(function); // copying bfunction
    char *term = strtok(copy, "+"); //parsing bfunction into terms
    int result = 0;

    // Going through all terms
    while (term) {
        int termValue = 1;
        int negative = 0;

        // Going through all variables in term
        for (char *a = term; *a; a++) {
            // Negative value
            if (*a == '!') {
                negative = 1;
                continue;
            }
            // If its variable
            if (isalpha(*a)) {
                int value = vars[(unsigned char)*a];
                // If negative variable, invert value
                if (negative) {
                    value = !value;
                    negative = 0;
                }
                // If value turn term into false
                if (value == 0) {
                    termValue = 0;
                    break;
                }
            }
            // Next term
            else if (*a == '*') {
                continue;
            }
        }
        // If one term is true, all result is true
        if (termValue == 1) {
            result = 1;
            break;
        }
        term = strtok(NULL, "+"); // Next term
    }

    free(copy);
    return result;
}

// Generating random order
void orderCreate(char *arr, size_t n) {
    for (size_t i = n - 1; i > 0; i--) {
        size_t j = rand() % (i + 1); // Random variable

        // Changing variables places
        char temp = arr[j];
        arr[j] = arr[i];
        arr[i] = temp;
    }
}

// Creating node
BDDNode* nodeCreate(char V, BDDNode *leafTrue, BDDNode *leafFalse, BDD *bdd) {
    // If both branches lead to the same node - reduction
    if (leafTrue == leafFalse) {
        return leafTrue;
    }

    // Adding node to hashtable
    unsigned int id = hashNode(V, leafTrue, leafFalse);
    BDDNode *current = bdd->nodeTable.table[id];

    //Checking for duplicates
    while (current != NULL) {
        if (current->variable == V && current->leafTrue == leafTrue && current->leafFalse == leafFalse) {
            return current;
        }
        current = current->next;
    }

    // Creating node
    BDDNode *newNode = (BDDNode*)malloc(sizeof(BDDNode));
    newNode->variable = V;
    newNode->leafTrue = leafTrue;
    newNode->leafFalse = leafFalse;
    newNode->next = bdd->nodeTable.table[id];
    bdd->nodeTable.table[id] = newNode;
    bdd->nodeTable.count++;

    return newNode;
}

// Creating BDD
BDDNode* BDDbuild(BDD *bdd, char *function, const char *order, unsigned int level, int *vars) {
    // If went through all variables, returning result node
    if (level >= strlen(order)) {
        int result = leafDecide(function, vars);

        if (result == 0) return bdd->resultFalse;
        else return bdd->resultTrue;
    }

    char V = order[level]; // Finding variable

    // True leaf for current variable
    int trueValues[256];
    memcpy(trueValues, vars, sizeof(int) * 256); // Copying variables values
    trueValues[V] = 1; // Changing value of current variable to 1
    BDDNode *leafTrue = BDDbuild(bdd, function, order, level + 1, trueValues); // Recursive call for the next variable

    // False leaf for current variable
    int falseValues[256];
    memcpy(falseValues, vars, sizeof(int) * 256); // Copying variables values
    falseValues[V] = 0; // Changing value of current variable to 0
    BDDNode *leafFalse = BDDbuild(bdd, function, order, level + 1, falseValues); // Recursive call for the next variable

    return nodeCreate(V, leafTrue, leafFalse, bdd); // Returning created node
}

//Initialising BDD
BDD* BDD_create(const char *function, const char *order) {
    BDD *bdd = (BDD*)malloc(sizeof(BDD));
    hashTableCreate(&bdd->nodeTable);
    createResultNodes(bdd);

    int values[256] = {0};

    bdd->order = strdup(order);
    bdd->variables = strdup(order);
    bdd->root = BDDbuild(bdd, function, order, 0, values);
    bdd->nodeCount = bdd->nodeTable.count + 2;

    return bdd;
}

// Creating BDD with best order
BDD* BDD_create_with_best_order(const char *function, const char *variables, int attempts) {
    size_t variablesCount = strlen(variables);

    // Creating 2 arrays of order for comparison
    char *bestOrder = malloc(variablesCount + 1);
    strcpy(bestOrder, variables);
    char *currentOrder = malloc(variablesCount + 1);
    strcpy(currentOrder, variables);

    BDD *bestBDD = NULL;
    unsigned int minNodes = 0;

    // Comparing number of nodes for each random order
    for (int i = 0; i < attempts; i++) {
        // Generating new order
        if (i > 0) {
            strcpy(currentOrder, variables);
            orderCreate(currentOrder, variablesCount);
        }

        BDD *currentBDD = BDD_create(function, currentOrder);

        // If with new order fewer nodes
        if (bestBDD == NULL || currentBDD->nodeCount < minNodes) {
            if (bestBDD) BDDfree(bestBDD);
            bestBDD = currentBDD;

            free(bestBDD->order);
            bestBDD->order = strdup(bestOrder);

            minNodes = currentBDD->nodeCount;
            strcpy(bestOrder, currentOrder);
        } else {
            BDDfree(currentBDD);
        }
    }

    free(currentOrder);
    free(bestOrder);
    return bestBDD;
}

// Using BDD
char BDD_use(BDD *bdd, const char *inputs) {
    // If wrong input
    if (!inputs || strlen(inputs) != strlen(bdd->variables)) {
        return 'f';
    }

    // Making array of values for each variable
    int values[256] = {0};

    for (int i = 0; bdd->variables[i] != '\0'; i++) {

        char V = bdd->variables[i];
        char value = inputs[i];

        if (value == '0') values[V] = 0; // For each variable ASCII-code id
        else if (value == '1') values[V] = 1;
        else return 'f';
    }

    BDDNode *current = bdd->root;
    // Going through bdd until we receive result node
    while (current->variable != '0' && current->variable != '1') {
        char V = current->variable;

        if (values[V] == 0) current = current->leafFalse;
        else current = current->leafTrue;
    }

    return current->variable; // Returning result node
}

//Releasing memory
void BDDfree(BDD *bdd) {
    if (!bdd) return;

    free(bdd->resultTrue);
    free(bdd->resultFalse);
    free(bdd->order);
    free(bdd->variables);
    free(bdd);
}