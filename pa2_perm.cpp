#include <algorithm>
#include <iostream>
#include <vector>

using namespace std;

///< boilerplate vector printing ...
void print_permutation(vector<int> &perm) {
    for (int i : perm) {
        cout << i << " ";
    }
    cout << endl;
}

///< create initial permutation
void create_permutation(vector<int> &perm, unsigned int n) {
    perm.clear();
    perm.resize(n);
    for (unsigned int i = 0; i < n; i++) {
        perm[i] = i;
    }
}

// solution w. prefix
void test_permutations_prefix(unsigned int n) {
    vector<int> permutation;
    create_permutation(permutation, n);
    print_permutation(permutation);
    while (std::next_permutation(permutation.begin() + 1, permutation.end())) {
        print_permutation(permutation);
    }
}

int main() {
    unsigned int n = 5;
    test_permutations_prefix(n);
    return 0;
}
