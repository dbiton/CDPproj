/**
    Daniel Alabi
    Count-Min Sketch Implementation based on paper by
    Muthukrishnan and Cormode, 2004

    Note:
    Code modified by Dvir David Biton
**/

#include <vector>
#include <utility>

// define some constants
# define LONG_PRIME 4294967311l
# define MIN(a,b)  (a < b ? a : b)

/** CountMinSketch class definition here **/
class CountMinSketch {
    // width, depth 
    unsigned int w, d;

    // eps (for error), 0.01 < eps < 1
    // the smaller the better
    float eps;

    // gamma (probability for accuracy), 0 < gamma < 1
    // the bigger the better
    float gamma;

    // total count so far
    unsigned int total;

    // array of arrays of counters
    int** C;

    // array of hash values for a particular item 
    // contains two element arrays {aj,bj}
    static std::vector<std::pair<int, int>> hash_keys;

    // generate new hash key
    void genHashKey();
public:
    // constructor
    CountMinSketch(float eps, float gamma);
    // destructor
    ~CountMinSketch();

    // update item (int) by count c
    void add(int item, int c = 0);

    // estimate count of item i and return count
    unsigned int query(int item);

    // return total count
    unsigned int numEvents();

    // merge this sketch with some other sketch
    void merge(const CountMinSketch& sketch_other);
};