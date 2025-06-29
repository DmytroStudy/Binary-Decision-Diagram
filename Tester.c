#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "BDD.h"

#define TESTS_PER_SIZE 100

// Generating random DNF
char* generateDNF(int N) {
    // Random number of terms (from 1.5N to 2N)
    int minTerms = (int)(1.5 * N);
    int maxTerms = 2 * N;
    int numTerms = minTerms + rand() % (maxTerms - minTerms + 1);

    // Buffer for the final result
    char* result = malloc(numTerms * N * 4);
    result[0] = '\0';

    // Generating terms
    for (int t = 0; t < numTerms; ++t) {
        int first = 1;

        // In each term, randomly choosing variables to include (50% chance)
        for (int v = 0; v < N; ++v) {
            if (rand() % 2 == 1) {
                if (!first) strcat(result, "*");
                first = 0;

                // Randomly adding negation
                if (rand() % 2 == 1) {
                    strcat(result, "!");
                }

                char V = 'a' + v; // Calculating ASCII-code id
                strncat(result, &V, 1); // Adding to the term
            }
        }

        // If no variable is in the term, we add two random variable
        if (first) {
            // First variable
            if (rand() % 2) strcat(result, "!");
            char V1 = 'a' + (rand() % N);
            strncat(result, &V1, 1);

            strcat(result, "*"); // Add * sign between variables

            // Second variable
            if (rand() % 2) strcat(result, "!");
            char V2 = 'a' + (rand() % N);
            strncat(result, &V2, 1);
        }

        // Adding + between terms
        if (t < numTerms - 1) {
            strcat(result, "+");
        }
    }
    return result;
}

// Get order of n variables
char* getOrder(int n) {
    char* result = malloc(n + 1);
    // Going from 'a' to n
    for (int i = 0; i < n; ++i) {
        result[i] = 'a' + i;
    }

    result[n] = '\0';
    return result;
}

// Calculate a^b
int intPow(int a, int b) {
    int result = 1;
    for (int i = 0; i < b; i++)
        result *= a;
    return result;
}

// Checking correct work of BDD
void resultsTest() {
    //Test for 5 variables
    BDD* bdd1 = BDD_create("a*b*!c*d+!a*c*d*e+b*!d*e", "abcde");
    if (BDD_use(bdd1, "00000") != '0')
        printf("error in BDD1, in test 1.1, result should be 0.\n");
    if (BDD_use(bdd1,"11010") != '1')
        printf("error in BDD1, in test 1.2, result should be 1.\n");
    if (BDD_use(bdd1,"01101") != '1')
        printf("error in BDD1, in test 1.3, result should be 1.\n");
    if (BDD_use(bdd1,"10111") != '0')
        printf("error in BDD1, in test 1.4, result should be 0.\n");
    if (BDD_use(bdd1,"11111") != '0')
        printf("error in BDD1, in test 1.5, result should be 0.\n");

    //Test for 10 variables
    BDD* bdd2 = BDD_create("!a*b*!c*d*e+a*!b*f*g*!h+c*d*!e*h*!i+!d*e*!f*i*j+b*!f*g*!i*!j", "abcdefghij");
    if (BDD_use(bdd2, "0000000000") != '0')
        printf("error in BDD2, in test 2.1, result should be 0.\n");
    if (BDD_use(bdd2,"0101100000") != '1')
        printf("error in BDD2, in test 2.2, result should be 1.\n");
    if (BDD_use(bdd2,"1000011000") != '1')
        printf("error in BDD2, in test 2.3, result should be 1.\n");
    if (BDD_use(bdd2,"1111000100") != '1')
        printf("error in BDD2, in test 2.4, result should be 1.\n");
    if (BDD_use(bdd2,"0100101000") != '1')
        printf("error in BDD2, in test 2.5, result should be 1.\n");

    //Test for 15 variables
    BDD* bdd3 = BDD_create("!a*b*!c*d*e*!f + a*!b*c*!d*f*g*!h + !b*!d*e*h*!i*j + c*!e*!g*i*k*!l + d*!f*!h*j*l*!m + !c*!i*!k*m*n*!o + b*!e*!j*!n*o", "abcdefghijklmno");
    if (BDD_use(bdd3, "000000000000000") != '0')
        printf("error in BDD3, in test 3.1, result should be 0.\n");
    if (BDD_use(bdd3,"010110000000000") != '1')
        printf("error in BDD3, in test 3.2, result should be 1.\n");
    if (BDD_use(bdd3,"101001100000000") != '1')
        printf("error in BDD3, in test 3.3, result should be 1.\n");
    if (BDD_use(bdd3,"001000101010000") != '0')
        printf("error in BDD3, in test 3.4, result should be 0.\n");
    if (BDD_use(bdd3,"111111111111111") != '0')
        printf("error in BDD3, in test 3.5, result should be 0.\n");

    BDDfree(bdd1);
    BDDfree(bdd2);
    BDDfree(bdd3);

    printf("All results tests are done\n");
}

// Testing number of nodes, time spent, Reduction percentage for BDD_create
void bddCreateTest(int N) {
    printf("\nBDD_Create tests (variables: 2-%d):\n", N);

    LARGE_INTEGER start, end, freq;
    QueryPerformanceFrequency(&freq);

    // Test number of variables from 2 to N
    for (int i = 2; i <= N; i++) {
        double totalNodes = 0.0;
        double totalTime = 0.0;
        double totalReduction = 0.0;
        double fullTree = (intPow(2, i + 1)) - 1; // 2^(n+1)-1 nodes (worst case)

        // Testing 100 random dnf for each size
        for (int j = 0; j < TESTS_PER_SIZE; j++) {
            srand(GetTickCount() + i * TESTS_PER_SIZE + j);
            char* dnf = generateDNF(i);
            char* order = getOrder(i);

            QueryPerformanceCounter(&start);

            BDD* bdd = BDD_create(dnf, order);

            QueryPerformanceCounter(&end);

            //Statistics:
            totalNodes += bdd->nodeCount;
            double timeSpent = (double)(end.QuadPart - start.QuadPart) / freq.QuadPart;
            totalTime += timeSpent;
            double reduction = 100.0 * (fullTree - bdd->nodeCount) / fullTree;
            totalReduction += reduction;

            free(dnf);
            free(order);
            BDDfree(bdd);
        }
        double averageNodes = totalNodes / TESTS_PER_SIZE;
        double averageTime = totalTime / TESTS_PER_SIZE;
        double averageReduction = totalReduction / TESTS_PER_SIZE;

        printf("Variables: %d, nodes: %.6f, time spent: %.6f seconds, reduction percentage: %.6f\n", i, averageNodes, averageTime, averageReduction);
    }
}

// Testing number of nodes, time spent, Reduction percentage for BDD_create_with_best_order
void bddBestOrderTest(int N) {
    printf("\nBDD_create_with_best_order tests (variables: 2-%d):\n", N);

    LARGE_INTEGER start, end, freq;
    QueryPerformanceFrequency(&freq);

    // Test number of variables from 2 to N
    for (int i = 2; i <= N; i++) {
        double totalNodes = 0.0;
        double totalTime = 0.0;
        double totalReduction = 0.0;
        double fullTree = (intPow(2, i + 1)) - 1; // 2^(n+1)-1 nodes (worst case)

        // Testing 100 random dnf for each size
        for (int j = 0; j < TESTS_PER_SIZE; j++) {
            srand(GetTickCount() + i * TESTS_PER_SIZE + j);
            char* dnf = generateDNF(i);
            char* order = getOrder(i);

            QueryPerformanceCounter(&start);

            BDD* bdd = BDD_create_with_best_order(dnf, order, 10*i);

            QueryPerformanceCounter(&end);

            //Statistics:
            totalNodes += bdd->nodeCount;
            double timeSpent = (double)(end.QuadPart - start.QuadPart) / freq.QuadPart;
            totalTime += timeSpent;
            double reduction = 100.0 * (fullTree - bdd->nodeCount) / fullTree;
            totalReduction += reduction;

            free(dnf);
            free(order);
            BDDfree(bdd);
        }
        double averageNodes = totalNodes / TESTS_PER_SIZE;
        double averageTime = totalTime / TESTS_PER_SIZE;
        double averageReduction = totalReduction / TESTS_PER_SIZE;

        printf("Variables: %d, nodes: %.6f, time spent: %.6f seconds, reduction percentage: %.6f\n", i, averageNodes, averageTime, averageReduction);
    }
}

int main() {
    resultsTest();
    bddCreateTest(15);
    bddBestOrderTest(15);
    return 0;
}