#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <chrono>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <map>

std::mutex matrix_mutex;

class Data{
public:
    std::map<int, std::vector<int>> matrix_minus;       // edges from value-nodes to key-node
    std::map<int, std::vector<int>> matrix_plus;        // edges from key-node to value-nodes
};

void read_from_file(const std::string& filename, std::streampos start, std::streampos end, Data& data) {
    std::ifstream file(filename, std::ios::in);

    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }

    file.seekg(start);
    std::string line;
    while (file.tellg() < end && std::getline(file, line)) {
        std::istringstream iss(line);
        int num1, num2;
        if (iss >> num1 >> num2) {
            std::lock_guard<std::mutex> matrix_lock(matrix_mutex);
            data.matrix_minus[num1].push_back(num2);
            data.matrix_plus[num2].push_back(num1);
        }
    }

    file.close();
}

void print_matrix(const std::map<int, std::vector<int>>& matrix_minus) {
    std::lock_guard<std::mutex> matrix_lock(matrix_mutex);
    for (const auto& [key, value] : matrix_minus) {
        std::cout << key << ": ";
        for (const auto& num : value) {
            std::cout << num << " ";
        }
        std::cout << std::endl;
    }
}

void load_data(const std::string& filename, int threads_count, Data& data){

    std::ifstream file(filename, std::ios::ate);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }

    std::streampos file_size = file.tellg();
    file.close();

    std::vector<std::thread> threads;
    std::streampos chunk_size = file_size / threads_count;

    for (int i = 0; i < threads_count; ++i) {
        std::streampos start_pos = i * chunk_size;

        std::streampos end_pos = (i + 1) * chunk_size;
        bool is_last_thread = (i == (threads_count - 1));
        if (is_last_thread) {
            end_pos = file_size;
        }

        threads.emplace_back(read_from_file, filename, start_pos, end_pos, std::ref(data));
    }

    for (auto& thread : threads) {
        thread.join();
    }
}

void pagerank(Data& data,
              int max_iterations = 100){
    std::unordered_map<int, double> pagerank;
    std::unordered_map<int, double> pagerank_new;

    double d = 0.85;
    double one_minus_d = 1.0 - d;
    for (auto & vertex : data.matrix_plus){
        pagerank[vertex.first] = 1 - d;
        pagerank_new[vertex.first] = 0.0;
    }
    double diff = 1.0;

    int N = data.matrix_plus.size();

    int iterations = 0;
    while(true){
        std::map< int, std::vector<int>>::iterator datIt;
#pragma omp parallel for default(none) shared(data, pagerank, pagerank_new, d, one_minus_d, N) private(datIt)
        for(int i = 0; i < N; i++)
        {
            datIt = data.matrix_plus.begin();
            std::advance(datIt, i);
            auto edge = *datIt;
            int vertex_to = edge.first;
            double sum = 0.0;
            for (auto vertex_from : edge.second){
                unsigned long Tw = data.matrix_minus[vertex_from].size();
                sum += pagerank[vertex_from] / Tw;
            }
            pagerank_new[vertex_to] = d * sum + one_minus_d;
        }

        diff = 0.0;
        for (auto & PR_pair : pagerank){
            diff += std::abs(pagerank_new[PR_pair.first] - PR_pair.second);
        }

        float epsilon = 0.001;
        if (diff < epsilon){
            break;
        }

        pagerank = pagerank_new;
        iterations++;
        if (iterations > max_iterations){
            break;
        }
    }

    for (auto & vertex : pagerank){
        vertex.second = vertex.second / N;
        std::cout << "Page: " << vertex.first << " Rank: " << vertex.second << std::endl;
    }
}

int main() {
    const std::string filename = "test_3.txt";
    const int threads_count = 4;

    Data data;

    auto start = std::chrono::high_resolution_clock::now();
    load_data(filename, threads_count, data);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    print_matrix(data.matrix_minus);
    std::cout << "Elapsed time: " << elapsed.count() << " s\n";

    start = std::chrono::high_resolution_clock::now();
    pagerank(data);
    end = std::chrono::high_resolution_clock::now();
    elapsed = end - start;

    std::cout << "Elapsed time: " << elapsed.count() << " s\n";

    return 0;
}