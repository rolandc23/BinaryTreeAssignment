// Implementation of the Tree ADT
// z5311913 25 October 2022

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "List.h"
#include "Record.h"
#include "Tree.h"

typedef struct node *Node;
struct node {
    Record rec;
    Node   left;
    Node   right;

    // IMPORTANT: Do not modify the fields above
    // You may add additional fields below if necessary

    int height;
    int balance;

};

struct tree {
    Node root;
    int (*compare)(Record, Record);

    // IMPORTANT: Do not modify the fields above
    // You may add additional fields below if necessary

};

static void doTreeFree(Node n, bool freeRecords);
static Node doTreeInsertAVL(Tree t, Node n, Record rec, bool *inserted);
static Node newNode(Record rec);
static int getHeight(Node n);
static Node checkRotation(Tree t, Node n, Record rec);
static Node rotateLeft(Node n);
static Node rotateRight(Node n);
static int max(int a, int b);
static Record doTreeSearch(Tree t, Node n, Record rec);
static void doTreeSearchBetween(Tree t, Node n, Record lower, Record upper, List l);
static void doTreeNext(Tree t, Node n, Record rec, Record *currBest);

////////////////////////////////////////////////////////////////////////
// Provided functions
// !!! DO NOT MODIFY THESE FUNCTIONS !!!

Tree TreeNew(int (*compare)(Record, Record)) {
    Tree t = malloc(sizeof(*t));
    if (t == NULL) {
        fprintf(stderr, "error: out of memory\n");
        exit(EXIT_FAILURE);
    }

    t->root = NULL;
    t->compare = compare;
    return t;
}

void TreeFree(Tree t, bool freeRecords) {
    doTreeFree(t->root, freeRecords);
    free(t);
}

static void doTreeFree(Node n, bool freeRecords) {
    if (n != NULL) {
        doTreeFree(n->left, freeRecords);
        doTreeFree(n->right, freeRecords);
        if (freeRecords) {
            RecordFree(n->rec);
        }
        free(n);
    }
}

////////////////////////////////////////////////////////////////////////
// Functions you need to implement

// Tree Insert //

// Boolean on whether record was inserted or not
bool TreeInsert(Tree t, Record rec) {

    // Default assignment
    bool inserted = false;

    // Insert into tree
    t->root = doTreeInsertAVL(t, t->root, rec, &inserted);

    return inserted;

}

// Inserts record into an AVL tree
static Node doTreeInsertAVL(Tree t, Node n, Record rec, bool *inserted) {

    // Null case
    if (n == NULL) {
        *inserted = true;
        return newNode(rec);
    }

    // Assign compare integer
    int cmp = t->compare(rec, n->rec);

    // Check cases for compare integer
    if (cmp == 0) {
        // Matching record found, no insertion
        return n;

    } else if (cmp < 0) {
        // Insertion occurs on the left
        n->left = doTreeInsertAVL(t, n->left, rec, inserted);

    } else {
        // Insertion occurs on the right
        n->right = doTreeInsertAVL(t, n->right, rec, inserted);
    }
    
    // Assign integers for heights of left and right sub-tree
    int LHeight = getHeight(n->left),
        RHeight = getHeight(n->right);

    // Update height and balance of node accordingly
    n->height = 1 + max(LHeight, RHeight);
    n->balance = LHeight - RHeight;

    // Check if rotation is required
    n = checkRotation(t, n, rec);
    
    return n;

}

// Function determining whether height of n
// should be 0 or not.
static int getHeight(Node n) {
    if (n == NULL) {
        return 0;
    }
    return n->height;
}

// Creates new node
static Node newNode(Record rec) {

    Node n = malloc(sizeof(*n));

    // Error if node is still null
    if (n == NULL) {
        fprintf(stderr, "error: out of memory\n");
        exit(EXIT_FAILURE);
    }

    // Define left and right pointers and record
    n->left = NULL;
    n->right = NULL;
    n->rec = rec;
    n->height = 1;
    n->balance = 0;

    return n;

}

// Checks if rotation is required to rebalance
// AVL tree
static Node checkRotation(Tree t, Node n, Record rec) {

    // Checks for cases where there are imbalances
    if (n->balance > 1) {
        // Left-side imbalance
        if (t->compare(rec, n->left->rec) > 0) {
            // Check if left node requires rotation
            n->left = rotateLeft(n->left);
        }
        n = rotateRight(n);

    } else if (n->balance < -1) {
        // Right-side imbalance
        if (t->compare(rec, n->right->rec) < 0) {
            // Check if right node requires rotation
            n->right = rotateRight(n->right);
        }
        n = rotateLeft(n);
    }

    return n;

}

// Rotates current node of tree to left
static Node rotateLeft(Node n) {

    // Null case or right branch is null
    if (n == NULL || n->right == NULL) {
        return n;
    }

    // Left rotation
    Node m = n->right;
    n->right = m->left;
    m->left = n;

    // Update heights accordingly
    n->height = 1 + max(getHeight(n->left), getHeight(n->right));
    m->height = 1 + max(getHeight(m->left), getHeight(m->right));

    return m;

}

// Rotates the current node of tree to the right
static Node rotateRight(Node n) {

    // Null case or left branch is null
    if (n == NULL || n->left == NULL) {
        return n;
    }

    // Right rotation
    Node m = n->left;
    n->left = m->right;
    m->right = n;

    // Update heights accordingly
    n->height = 1 + max(getHeight(n->left), getHeight(n->right));
    m->height = 1 + max(getHeight(m->left), getHeight(m->right));

    return m;
}

// Gets larger of two given integers
static int max(int a, int b) {
    if (a > b) {
        return a;
    } else {
        return b;   
    }
}

// Tree Search //

// Performs search of tree for a given record
// Returns record if exists, otherwise return NULL
Record TreeSearch(Tree t, Record rec) {
    return doTreeSearch(t, t->root, rec);
}

// Binary search on a balanced tree for an item
// matching given record
static Record doTreeSearch(Tree t, Node n, Record rec) {

    // Null case
    if (n == NULL) {
        return NULL;
    }

    // Assign comparison integer
    int cmp = t->compare(rec, n->rec);

    // Check if left or right search is required
    if (cmp < 0) {
        // Left search
        return doTreeSearch(t, n->left, rec);

    } else if (cmp > 0) {
        // Right search
        return doTreeSearch(t, n->right, rec);
        
    } else {
        // Return current record if found
        return n->rec;
    }

}

// Performs a search between two records which form
// lower and upper bound
List TreeSearchBetween(Tree t, Record lower, Record upper) {
    List l = ListNew();
    doTreeSearchBetween(t, t->root, lower, upper, l);
    return l;
}

// Finds all records between lower and upper bound and returns
// an ordered list containing them
static void doTreeSearchBetween(Tree t, Node n, Record lower,
                                Record upper, List l) {

    // Null Case
    if (n == NULL) {
        return;
    }
    // Assign lower and upper comparisons to integer value
    int cmpLower = t->compare(lower, n->rec), 
        cmpUpper = t->compare(upper, n->rec);

    if (cmpLower > 0) {
        // When n lies left of lower bound
        doTreeSearchBetween(t, n->right, lower, upper, l);

    } else if (cmpUpper < 0) {
        // When n lies right of upper bound
        doTreeSearchBetween(t, n->left, lower, upper, l);

    } else {
        // When n lies between lower and upper bound
        doTreeSearchBetween(t, n->left, lower, upper, l);

        // Append left -> right order
        ListAppend(l, n->rec);

        doTreeSearchBetween(t, n->right, lower, upper, l);
    }
    
}

// Function searching for next record from given record
Record TreeNext(Tree t, Record rec) {
    Record currBest = NULL;
    doTreeNext(t, t->root, rec, &currBest);
    return currBest;
}

// Finds the node which holds the "next" record from the given one,
// unless if the record is the same, to which it will return the same record
static void doTreeNext(Tree t, Node n, Record rec, Record *currBest) {

    // Null case
    if (n == NULL) {
        return;
    }

    // Assign a compare integer
    int cmp = t->compare(rec, n->rec);

    // Cases for compare integer
    if (cmp == 0) {
        // If record is same as the current node record
        *currBest = n->rec;
        return;

    } else if (cmp < 0) {
        // If record is left of current node:
        // Change current best record
        *currBest = n->rec;
        return doTreeNext(t, n->left, rec, currBest);

    } else if (cmp > 0) {
        // If record is right of current node
        return doTreeNext(t, n->right, rec, currBest);

    }
    
}


///////////////////////////////////////////////////////////////
