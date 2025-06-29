#ifndef BDD_H
#define BDD_H
#define HASH_TABLE_SIZE 1000

typedef struct BDDNode {
    char variable;
    struct BDDNode *leafTrue;
    struct BDDNode *leafFalse;
    struct BDDNode *next;      // for hash table
    unsigned int id;    // id of node
}BDDNode;

typedef struct NodeHashTable{
    BDDNode *table[HASH_TABLE_SIZE];
    unsigned int count;
} NodeHashTable;

typedef struct BDD{
    unsigned int nodeCount;

    char *order;
    char *variables; // Initial order of variables

    BDDNode *root;
    BDDNode *resultTrue; // result nodes
    BDDNode *resultFalse;
    NodeHashTable nodeTable;
} BDD;

void createResultNodes(BDD *bdd);

unsigned int hashNode(char var, BDDNode *leafTrue, BDDNode *leafFalse);

void hashTableCreate(NodeHashTable *nodeTable);

BDDNode* nodeCreate(char var, BDDNode *leafTrue, BDDNode *leafFalse, BDD *bdd);

int leafDecide(const char *bfunction, int *var_values);

BDDNode* BDDbuild(BDD *bdd, char *bfunction, const char *order, unsigned int level, int *var_values);

BDD* BDD_create(const char *bfunction, const char *order);

void orderCreate(char *array, size_t n);

BDD* BDD_create_with_best_order(const char *bfunction, const char *variables, int attempts);

char BDD_use(BDD *bdd, const char *inputs);

void BDDfree(BDD *bdd);

#endif