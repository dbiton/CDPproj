# include <iostream>
# include <cmath>
# include <cstdlib>
# include <ctime>
# include <limits>
#include "CountMinSketch.h"

using namespace std;

vector<pair<int,int>> CountMinSketch::hash_keys;

/**
   Class definition for CountMinSketch.
   public operations:
   // overloaded updates
   void update(int item, int c);
   void update(char *item, int c);
   // overloaded estimates
   unsigned int estimate(int item);
   unsigned int estimate(char *item);
**/


// CountMinSketch constructor
// ep -> error 0.01 < ep < 1 (the smaller the better)
// gamma -> probability for error (the smaller the better) 0 < gamm < 1
CountMinSketch::CountMinSketch(float ep, float gamm) {
    if (!(0.009 <= ep && ep < 1)) {
        cout << "eps must be in this range: [0.01, 1)" << endl;
        exit(EXIT_FAILURE);
    }
    else if (!(0 < gamm && gamm < 1)) {
        cout << "gamma must be in this range: (0,1)" << endl;
        exit(EXIT_FAILURE);
    }
    eps = ep;
    gamma = gamm;
    w = ceil(exp(1) / eps);
    d = ceil(log(1 / gamma));
    total = 0;
    // initialize counter array of arrays, C
    C = new int* [d];
    unsigned int i, j;
    for (i = 0; i < d; i++) {
        C[i] = new int[w];
        for (j = 0; j < w; j++) {
            C[i][j] = 0;
        }
    }
    // initialize d pairwise independent hashes
    while (hash_keys.size() < d) {
        genHashKey();
    }
}

// CountMinSkectch destructor
CountMinSketch::~CountMinSketch() {
    // free array of counters, C
    unsigned int i;
    for (i = 0; i < d; i++) {
        delete[] C[i];
    }
    delete[] C;
}

// CountMinSketch totalcount returns the
// total count of all items in the sketch
unsigned int CountMinSketch::numEvents() {
    return total;
}

// countMinSketch update item count (int)
void CountMinSketch::add(int item, int c) {
    total = total + c;
    unsigned int hashval = 0;
    for (unsigned int j = 0; j < d; j++) {
        hashval = ((long)hash_keys[j].first * item + hash_keys[j].second) % LONG_PRIME % w;
        C[j][hashval] = C[j][hashval] + c;
    }
}

// CountMinSketch estimate item count (int)
unsigned int CountMinSketch::query(int item) {
    int minval = numeric_limits<int>::max();
    unsigned int hashval = 0;
    for (unsigned int j = 0; j < d; j++) {
        hashval = ((long)hash_keys[j].first * item + hash_keys[j].second) % LONG_PRIME % w;
        minval = MIN(minval, C[j][hashval]);
    }
    return minval;
}

// generates aj bj from field Z_p for use in hashing
void CountMinSketch::genHashKey() {
    int i0 = int(float(rand()) * float(LONG_PRIME) / float(RAND_MAX) + 1);
    int i1 = int(float(rand()) * float(LONG_PRIME) / float(RAND_MAX) + 1);
    hash_keys.push_back(std::make_pair(i0, i1));
}

void CountMinSketch::merge(const CountMinSketch& sketch_other)
{
    if (sketch_other.w != w || sketch_other.d != d) {
        cout << "sketch dimensions must fit!" << endl;
        exit(EXIT_FAILURE);
    }
    total += sketch_other.total;
    unsigned int i, j;
    for (i = 0; i < d; i++) {
        for (j = 0; j < w; j++) {
            C[i][j] += sketch_other.C[i][j];
        }
    }
}
