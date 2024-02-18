#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include <vector>
#include <cmath>
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cfloat>
#include <thread>

long factorial(int n) {
    if ((n==0)||(n==1))
        return 1;
    else
        return n*factorial(n-1);
}

std::vector<double> distances;

void calculate_shortest_path();

void print_distances(std::vector<double> &dist, unsigned int n) {
    for (unsigned int i = 0; i < dist.size(); i++) {
        std::cout << dist[i] << "\t";
        if ((i + 1) % n == 0)
            std::cout << std::endl;
    }
}

double compute_distance(double x1, double y1, double x2, double y2) {
    return std::sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}

bool read_tsp_file(const std::string &file_name) {
    std::ifstream file(file_name);

    if (file.is_open()) {
        std::vector<double> xs, ys;

        std::string line;

        std::getline(file, line);
        std::getline(file, line);
        std::getline(file, line);
        std::getline(file, line);
        std::getline(file, line);
        std::getline(file, line);
        std::getline(file, line);

        while (std::getline(file, line)) {
            if (line[0] == 'E')
                break;

            std::stringstream sin(line);
            int id;
            double x, y;
            sin >> id >> x >> y;

            xs.push_back(x);
            ys.push_back(y);
        }
        file.close();

        unsigned int n = xs.size();

        distances.resize(n * n);

        for (unsigned int i = 0; i < n; i++) {
            for (unsigned int j = i; j < n; j++) {
                double dist = compute_distance(xs[i], ys[i], xs[j], ys[j]);
                distances[i * n + j] = dist;
                distances[j * n + i] = dist;
            }
        }

        //print_distances(distances, 9);
    } else {
        std::cout << file_name << " file not open" << std::endl;
    }
}

void print_permutation(std::vector<int> &perm) {
    for (int i : perm) {
        std::cout << i << " ";
    }
    std::cout << std::endl;
}

///< create initial permutation
void create_permutation(std::vector<int> &perm, unsigned int n) {
    perm.clear();
    perm.resize(n);
    for (int i = 0; i < n; i++) {
        perm[i] = i;
    }
}

double get_distance(int i, int j, int n=5) {
    return distances[i * n + j];
}

double get_path_distance(std::vector<int> &perm) {
    double dist = 0;
    int n = (int) perm.size();
    for (int i = 0; i < n - 1; i++) {
        dist += get_distance(perm[i], perm[i + 1], n);
    }
    dist += get_distance(perm[n - 1], perm[0], n);
    return dist;
}

void one_thread(std::vector<int> permutation, int chunk_size, double* result) {
    auto local_max_distance = DBL_MAX;
    int counter = 0;
    while(counter < chunk_size){
        counter++;
        double tmp_dist = get_path_distance(permutation);
        if(tmp_dist < local_max_distance){
            local_max_distance = tmp_dist;
        }
        next_permutation(permutation.begin() + 1, permutation.end());
    }
    *result = local_max_distance;
}

void calculate_shortest_path(std::vector<int> permutation) {
    double min_distance = get_path_distance(permutation);
    int n = 12;

    std::vector<double> fact(n);
    int k = 0;
    fact[k] = 1;
    while (++k < n)
        fact[k] = fact[k - 1] * k;

    long f = factorial(permutation.size() - 1) - 1;
    int thread_count = 8;
    std::vector<double> results = std::vector<double>(thread_count);
    long chunk = ceil(f / thread_count);

    std::vector<std::thread> threads;

    printf("f: %l\n", f);

    int perm_index = 0;
    for(long i = 0; i < thread_count; i++) {
        for(long j = 0; j < chunk; j++) {
            std::next_permutation(permutation.begin() + 1, permutation.end());
            perm_index++;
        }
        threads.emplace_back(one_thread, permutation, chunk, &results[i]);
    }

    for(auto &thread : threads) {
        thread.join();
    }

    auto min_distance_2 = std::min_element(results.begin(), results.end());
    min_distance = std::min(*min_distance_2, min_distance);
    std::cout << "Min distance: " << min_distance << " " << std::endl;
}

int main() {
    // measure time
    read_tsp_file("pa2_input.txt");
    std::vector<int> permutation;
    create_permutation(permutation, 22);
    auto start = std::chrono::high_resolution_clock::now();
    calculate_shortest_path(permutation);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "Elapsed time: " << elapsed.count() << " s\n";
    return 0;
}
