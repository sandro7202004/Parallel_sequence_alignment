#include <iostream>
#include <utility> 
#include <vector>
#include <cstring> 
#include <thread>
#include <chrono>
using namespace std;
using namespace std::chrono; 

void printMatrix(const std::vector<int>& data, int n, int m) {

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            std::cout << data[i * m + j] << " ";
        }
        std::cout << "\n";
    }
}

int p(int i, int j, string s0, string s1, int ma, int mi) 
{
    if (s0[i] == s1[j]) {
        return ma;
    }
    return mi;
}

void NW_step(vector<int>& H, int i, int j, string s0, string s1, int ma, int mi, int g)
{
    int n = strlen(s0.c_str());
    int m = strlen(s1.c_str());

    int up = H[i * m + (j-1)] + g;
    int left = H[(i-1) * m + (j)] + g;
    int diag = H[(i-1) * m + (j-1)] + p(i,j,s0,s1,ma,mi);

    H[i * m + j] = max(max(diag, up), left);

    return;
}

vector<int> NW(string s0, string s1, int ma, int mi, int g)
{

    int n = strlen(s0.c_str());
    int m = strlen(s1.c_str());
    int p;

    vector<int> H(n*m);

    H[0] = 0;

    for (int i = 1; i<n; i++) {
        H[i*m] = i*g;
    }

    for (int j = 1; j<m; j++) {
        H[j] = j*g;
    }

    for (int i=1; i <n; i++)
    {
        for (int j=1; j<m; j++) 
        {
            if (s0[i] == s1[j]) 
            {
                p = 1;
            }
            else 
            {
                p = -1;
            }
            int res = max(max(H[(i-1)*m + (j-1)] + p, H[(i-1)*m + (j)] + g), H[(i)*m + (j-1)] + g);

            H[i*m + j] = res;
        }
    }

    return H;

}


void worker_col(int& working, int& mode, int& col, int& num_threads, int index, string s0, string s1, int ma, int mi, int g, vector<int>& H, vector<bool>& boundary_done, int n, int m)
{
    while (true)
    {
        if (working == 1)
        {
            if (mode == 0)
            {
                // INIT first column in parallel: H[i,0] = i * g
                int rows_per = (n - 1) / num_threads;
                int rem      = (n - 1) % num_threads;
                int begin_i  = 1 + index * rows_per + std::min(index, rem);
                int end_i    = begin_i + rows_per + (index < rem ? 1 : 0);

                if (begin_i < 1) begin_i = 1;
                for (int i = begin_i; i < end_i; ++i) {
                    H[i * m + 0] = i * g;
                }
            }
            else if (mode == 1)
            {
                // COMPUTE column 'col' in parallel, respecting row-dependency
                int rows_per = (n - 1) / num_threads;
                int rem      = (n - 1) % num_threads;
                int begin_i  = 1 + index * rows_per + std::min(index, rem);
                int end_i    = begin_i + rows_per + (index < rem ? 1 : 0);

                if (begin_i < 1) begin_i = 1;

                // Wait for previous thread to finish its block for this column
                if (index > 0) {
                    while (!boundary_done[index - 1]) { /* busy-wait */ }
                }

                for (int i = begin_i; i < end_i; ++i) {
                    int j = col;
                    int up   = H[i * m + (j - 1)] + g;   // H[i, j−1]
                    int left = H[(i - 1) * m + j] + g;   // H[i−1, j]
                    int diag = H[(i - 1) * m + (j - 1)]
                                + ((s0[i] == s1[j]) ? ma : mi);
                    H[i * m + j] = max(max(up, left), diag);
                }

                // Signal that this thread's last row for this column is done
                boundary_done[index] = true;
            }
            else if (mode == 2)
            {
                // SHUTDOWN
                working = 0;
                return;
            }

            working = 0;
        }
    }
}

vector<int> NW_column_parallel(string s0, string s1, int ma, int mi, int g, int num_threads)
{
    int n = s0.length();
    int m = s1.length();

    vector<int> H(n * m, 0);
    vector<int> working(num_threads - 1, 0);
    vector<bool> boundary_done(num_threads, false);
    vector<thread> threads(num_threads - 1);

    int mode = 0;  // 0=init col0, 1=compute col, 2=shutdown
    int col  = 1;  // next column to compute

    for (int k = 0; k < num_threads - 1; k++) {
        threads[k] = thread(worker_col, ref(working[k]), ref(mode), ref(col), ref(num_threads), k, s0, s1, ma, mi, g, ref(H), ref(boundary_done), n, m);
    }

    // 1)a) INITIALIZE column 0 in parallel:
    mode = 0;
    // wake up worker threads
    for (int k = 0; k < num_threads - 1; k++) {
        working[k] = 1;
    }
    // main thread does its own slice, index = num_threads−1
    {
        int index = num_threads - 1;
        int rows_per = (n - 1) / num_threads;
        int rem      = (n - 1) % num_threads;
        int begin_i  = 1 + index * rows_per + std::min(index, rem);
        int end_i    = begin_i + rows_per + (index < rem ? 1 : 0);

        if (begin_i < 1) begin_i = 1;
        for (int i = begin_i; i < end_i; ++i) {
            H[i * m + 0] = i * g;
        }
        boundary_done[index] = true;  // signal last block done
    }
    // barrier
    while (true) {
        int active = 0;
        for (int k = 0; k < num_threads - 1; k++) active += working[k];
        if (active == 0) break;
    }

    // 1)b) INITIALIZE first row: H[0,j] = j*g for j=1..m−1
    for (int j = 1; j < m; ++j) {
        H[j] = j * g;
    }

    // 2) For each column j=1..m−1, compute it in parallel:
    for (col = 1; col < m; ++col) {
        mode = 1;
        // reset boundary flags before each column
        for (int k = 0; k < num_threads; k++) {
            boundary_done[k] = false;
        }

        // wake up worker threads
        for (int k = 0; k < num_threads - 1; k++) {
            working[k] = 1;
        }
        // main thread (index = num_threads−1) must wait for thread num_threads−2
        int main_index = num_threads - 1;
        if (main_index > 0) {
            while (!boundary_done[main_index - 1]) { /* busy-wait */ }
        }
        {
            int index = main_index;
            int rows_per = (n - 1) / num_threads;
            int rem      = (n - 1) % num_threads;
            int begin_i  = 1 + index * rows_per + std::min(index, rem);
            int end_i    = begin_i + rows_per + (index < rem ? 1 : 0);

            if (begin_i < 1) begin_i = 1;
            for (int i = begin_i; i < end_i; ++i) {
                int j = col;
                int up   = H[i * m + (j - 1)] + g;   // H[i, j−1]
                int left = H[(i - 1) * m + j] + g;   // H[i−1, j]
                int diag = H[(i - 1) * m + (j - 1)]
                            + ((s0[i] == s1[j]) ? ma : mi);
                H[i * m + j] = max(max(up, left), diag);
            }
            boundary_done[index] = true;  // signal main's block done
        }
        // barrier for workers
        while (true) {
            int active = 0;
            for (int k = 0; k < num_threads - 1; k++) active += working[k];
            if (active == 0) break;
        }
    }

    // 3) SHUTDOWN all workers
    mode = 2;
    for (int k = 0; k < num_threads - 1; k++) {
        working[k] = 1;
    }
    // wait until all have seen mode=2 and set working=0
    while (true) {
        int active = 0;
        for (int k = 0; k < num_threads - 1; k++) active += working[k];
        if (active == 0) break;
    }
    for (int k = 0; k < num_threads - 1; ++k) {
        threads[k].join();
    }

    return H;
}


int main() {
    std::srand(std::time(0));

    unsigned int N = 1 << 14;
    std::cout << "N = " << N << "\n";

    std::string nucleotides = "ACGT";

    string s0 = "*";
    for (int i = 0; i < N; ++i) {
        char c = nucleotides[std::rand() % 4];
        s0 += c;
    }
    std::cout << "s0 generated (first 10 chars): " << s0.substr(0, 10) << "...\n";

    string s1 = "*";
    for (int i = 0; i < N; ++i) {
        char c = nucleotides[std::rand() % 4];
        s1 += c;
    }
    std::cout << "s1 generated (first 10 chars): " << s1.substr(0, 10) << "...\n";

    std::cout << "RUNNING SEQUENTIAL CODE\n";
    auto start = high_resolution_clock::now();
    vector<int> H1 = NW(s0, s1, 1, -1, -2);
    auto end = high_resolution_clock::now();

    auto duration = duration_cast<microseconds>(end - start);
    std::cout << "Sequential execution time: " << duration.count() << " µs\n";
    std::cout << "Sequential score: " << H1.back() << "\n\n";

    std::cout << "RUNNING PARALLEL CODE\n";
    start = high_resolution_clock::now();
    vector<int> H2 = NW_column_parallel(s0, s1, 1, -1, -2, 5);
    end = high_resolution_clock::now();

    duration = duration_cast<microseconds>(end - start);
    std::cout << "Parallel execution time: " << duration.count() << " µs\n";
    std::cout << "Parallel score: " << H2.back() << "\n";

    return 0;
}

