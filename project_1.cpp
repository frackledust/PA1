#include <algorithm>
#include <iostream>
#include <vector>
#include <cfloat>
#include <fstream>
#include <sstream>
#include <cmath>
#include <thread>

struct Data {
    unsigned int n;
    std::vector<int> distances;
    std::vector<int> flow;
};

int factorial(int n) {
    if ((n==0)||(n==1))
        return 1;
    else
        return n*factorial(n-1);
}

void print_matrix(std::vector<int> &mat, unsigned int n) {
    for (unsigned int i = 0; i < n; i++) {
        for (unsigned int j = 0; j < n; j++) {
            std::cout << mat[i * n + j] << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void read_matrix_file(const std::string &file_name, Data& data) {
    std::ifstream file(file_name);

    if (file.is_open()) {
        // from the first like of the file read the number of cities
        file >> data.n;

        // read the distance matrix
        data.distances.resize(data.n * data.n);
        for(int i = 0; i < data.n; i++) {
            for(int j = 0; j < data.n; j++) {
                file >> data.distances[i * data.n + j];
            }
        }

        // read the flow matrix
        data.flow.resize(data.n * data.n);
        for(int i = 0; i < data.n; i++) {
            for(int j = 0; j < data.n; j++) {
                file >> data.flow[i * data.n + j];
            }
        }

        file.close();
    } else {
        std::cout << file_name << " file not open" << std::endl;
    }
}

void print_permutation(std::vector<int> &perm) {
    for (int i: perm) {
        std::cout << i << " ";
    }
    std::cout << std::endl;
}

void create_permutation(std::vector<int> &perm, unsigned int n) {
    perm.clear();
    perm.resize(n);

    for (unsigned int i = 0; i < n; i++) {
        perm[i] = i;
    }
}

struct QAP {
    double cost = DBL_MAX;
    unsigned int index = 0;

    QAP() = default;

    QAP(double c, unsigned int i) : cost(c), index(i) {}
};

struct BestResult{
    std::vector<int> init_permutation;
    double min_cost = DBL_MAX;
    std::vector<int> best_permutation;
    BestResult() = default;
};

double get_value(const std::vector<int> &distances, const int i, const int j, const unsigned int n) {
    return distances[i * n + j];
}

QAP evaluate_qap(const Data &data, const std::vector<int> &perm, const unsigned int n, const double best_cost) {
    double sum = 0;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            sum += get_value(data.distances, i, j, n) * get_value(data.flow, perm[i], perm[j], n);

            if (sum >= best_cost) {
                return {sum,  (i + 1) % n};
            }
        }
    }
    return {sum, 0};
}


void one_thread_bnb(const Data& data, unsigned int n, const int chunk_size, BestResult& result) {

    // make copy of permutation
    std::vector<int> permutation = result.init_permutation;
    QAP qap_res;
    result.min_cost = DBL_MAX;
    int counter = 0;
    int skipped = 0;
    while(counter < chunk_size){
        qap_res = evaluate_qap(data, permutation, n, result.min_cost);
        if (qap_res.index > 0) {
            unsigned int val = permutation[qap_res.index];
            while (std::next_permutation(permutation.begin(), permutation.end())) {
                counter++;
                skipped++;
                if (permutation[qap_res.index] != val) {
                    break;
                }
            }
            continue;
        }
        if (qap_res.cost < result.min_cost) {
            result.best_permutation = permutation;
            result.min_cost = qap_res.cost;
        }

        counter++;
        next_permutation(permutation.begin(), permutation.end());
    }

//    print_permutation(result.best_permutation);
//    std::cout << "BEST: " << result.min_cost << ", \t skipped perms.: " << counter - skipped << std::endl;
}

void test_permutations_bnb(const Data& data, unsigned int n) {
    auto min_cost = DBL_MAX;
    std::vector<int> best_permutation;
    std::vector<int> permutation;
    create_permutation(permutation, n);

    int f = factorial(n) - 1;
    int thread_count = 10;
    std::vector<BestResult> results = std::vector<BestResult>(thread_count);
    int chunk = ceil(f / thread_count);

    std::vector<std::thread> threads;
    std::cout << f << std::endl;

    int perm_index = 0;
    for(long i = 0; i < thread_count; i++) {
        results[i].init_permutation.resize(n);
        std::copy(permutation.begin(), permutation.end(), results[i].init_permutation.begin());
        threads.emplace_back(one_thread_bnb, data, n, chunk, std::ref(results[i]));
        for(long j = 0; j < chunk; j++) {
            std::next_permutation(permutation.begin(), permutation.end());
            perm_index++;
        }
    }

    for(auto &thread : threads) {
        thread.join();
    }

    // find min cost in best results
    for (auto &result : results) {
        if (result.min_cost < min_cost) {
            min_cost = result.min_cost;
            best_permutation = result.best_permutation;
        }
    }

    std::cout << "Min cost: " << min_cost << std::endl;
    print_permutation(best_permutation);
}

int main() {
    Data data;
    read_matrix_file("data.txt", data);

    test_permutations_bnb(data, data.n);
    return 0;
}
