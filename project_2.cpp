#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <chrono>
#include <algorithm>


std::vector<std::vector<long>> read_matrix_from_file(const std::string& filename, int N, int D){

    std::ifstream file(filename);
    std::string line;
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return {};
    }

    std::vector<std::vector<long>> matrix;

    //skip first line
    std::getline(file, line);

    while (std::getline(file, line)){
        if (matrix.size() >= N){
            break;
        }

        std::vector<long> row;
        std::istringstream iss(line);
        // divide line by commas
        std::string token;
        // skip first token
        std::getline(iss, token, ',');
        while (std::getline(iss, token, ',')){
            if(row.size() >= D){
                break;
            }

            row.push_back(std::stoi(token));
        }
        matrix.push_back(row);
    }

    file.close();
    return matrix;
}

void print_matrix(const std::vector<std::vector<long>>& matrix){
    for (const auto& row : matrix){
        for (auto elem : row){
            std::cout << elem << " ";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

std::vector<std::vector<long>> calculate_similarity_matrix(const std::vector<std::vector<long>>& matrix, int N, int D){
    std::vector<long> row(N, 0);
    std::vector<std::vector<long>> similarity_matrix = std::vector<std::vector<long>>(N, row);

    long min_similarity = 0;
# pragma omp parallel for collapse(2) default(none) shared(matrix, similarity_matrix, N, D) reduction(min: min_similarity)
    for (int i = 0; i < N; i++){
        for (int j = 0; j < N; j++){
            long similarity = 0;

            for (int k = 0; k < D; k++){
                long diff = matrix[i][k] - matrix[j][k];
                similarity += diff * diff;
            }
            similarity = -similarity;
            min_similarity = std::min(min_similarity, similarity);
            similarity_matrix[i][j] = similarity;
        }
    }

    // set diagonal to 0
    for (int i = 0; i < N; i++){
        similarity_matrix[i][i] = min_similarity;
    }

    std::cout << "Min similarity: " << min_similarity << std::endl;

    return similarity_matrix;
}

long calculate_sum_on_max(const std::vector<std::vector<long>>& matrix, long i, long k, long N){
    long sum_on_max = 0;
    for (int i_ = 0; i_ < N; i_++){
        if (i_ != i){
            sum_on_max += std::max(0L, matrix[i_][k]);
        }
    }
    return sum_on_max;
}

std::vector<std::vector<long>> calculate_affinity_propagation(const std::vector<std::vector<long>>& S, long N, long max_iter) {

    // initialize availability matrix and responsibility matrix
    std::vector<long> row(N, 0);
    std::vector<std::vector<long>> A = std::vector<std::vector<long>>(N, row);
    std::vector<std::vector<long>> R = std::vector<std::vector<long>>(N, row);
    std::vector<std::vector<long>> C = std::vector<std::vector<long>>(N, row);
    bool change = true;

    long iter = 0;
    while(change && iter < max_iter){
        iter++;
        //std::cout << "Iteration: " << iter << std::endl;

        // fill responsibility matrix
# pragma omp parallel for collapse(2) default(none) shared(A, S, R, N)
        for (int i = 0; i < N; i++) {
            for (int k = 0; k < N; k++) {
                R[i][k] = S[i][k];

                long max_ = std::numeric_limits<long>::min();
                for (int k_ = 0; k_ < N; k_++) {
                    if (k_ != k) {
                        max_ = std::max(max_, S[i][k_] + A[i][k_]);
                    }
                }
                R[i][k] -= max_;
            }
        }

# pragma omp parallel for collapse(2) default(none) shared(A, R, N)
        // fill availability matrix
        for (int i = 0; i < N; i++) {
            for (int k = 0; k < N; k++) {
                long sum_on_max = calculate_sum_on_max(R, i, k, N);
                if (i == k) {
                    A[i][k] = sum_on_max;
                } else {
                    A[i][k] = std::min(0L, R[k][k] + sum_on_max);
                }
            }
        }

        // calculate C matrix
        change = false;
# pragma omp parallel for collapse(2) default(none) shared(A, R, N, C, change)
        for (int i = 0; i < N; i++) {
            for (int k = 0; k < N; k++) {
                long new_value = A[i][k] + R[i][k];
                long old_value = C[i][k];
                change = change || (new_value != old_value);
                C[i][k] = new_value;
            }
        }
    }
    print_matrix(C);
    return C;
}

void create_clusters(const std::vector<std::vector<long>>& C) {
    // find maximum value in each row of C
    auto max_values = std::vector<long>(C.size());
    auto max_indexes = std::vector<long>(C.size());
    for (int i = 0; i < C.size(); i++) {
        auto a  = std::max_element(C[i].begin(), C[i].end());
        max_values[i] = *a;
        max_indexes[i] = std::distance(C[i].begin(), a);

        std::cout << "Element " << i + 1 << " belongs to cluster defined by element " << max_indexes[i] + 1 << std::endl;
    }

}

int main(){
    auto matrix = read_matrix_from_file("small.csv", 5, 5);
    //print_matrix(matrix);
    int N = matrix.size();
    int D = matrix[0].size();
    auto S = calculate_similarity_matrix(matrix, N, D);
    print_matrix(S);

    auto start = std::chrono::high_resolution_clock::now();
    long max_iter = 80000000;
    auto C = calculate_affinity_propagation(S, N, max_iter);

    std::chrono::duration<double> elapsed = std::chrono::high_resolution_clock::now() - start;
    std::cout << "Elapsed time: " << elapsed.count() << " s\n";

    create_clusters(C);
    return 0;
}