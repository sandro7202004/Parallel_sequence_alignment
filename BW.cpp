#include <iostream>
#include <atomic>
#include <utility> 
#include <vector>
#include <cstring> 
#include <thread>
#include <chrono>
using namespace std;
using namespace std::chrono; 

bool check_matrix(vector<int> H1, vector<int> H2, int n, int m)
{
    for (int i = 0; i < n; i++)
    {
        for(int j = 0; j < m; j++)
        {
            if (H1[i*m + j] != H2[i*m+j])
            {
                return false;
            }
        }
    }
    return true;
}

void printMatrix(const std::vector<int>& data, int n, int m) {

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            std::cout << data[i * m + j] << " ";
        }
        std::cout << "\n";
    }
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
            if (s0[i] == s1[j]){p = ma;}
            else {p = mi;}

            H[i * m + j] = max(max(H[(i-1) *  m + (j-1)] + p, H[(i-1) * m + j] + g), H[i * m + (j-1)] + g);;
        }
    }

    return H;

}

void worker_BW_init_row(int& block, int& rest, int index, string s0, string s1, int ma, int mi, int g,
    vector<int>& H, vector<int>& boundary_left,vector<int>& boundary_right, int n, int m){
    
    int begin_j;
    int end_j;
        
    if (index < rest){
        begin_j = block * index + index;
        end_j = block * (index + 1) + (index + 1);
    }
    else{
        begin_j = block * index + rest;
        end_j = block * (index + 1) + rest;
    }
    
    while (begin_j != end_j) 
    {
        H[begin_j] = begin_j * g;
        begin_j += 1;
    }
}

void worker_BW_init_col(int& block, int& rest, int index, string s0, string s1, int ma, int mi, int g,
    vector<int>& H, vector<int>& boundary_left,vector<int>& boundary_right, int n, int m){

    int begin_i;
    int end_i;

    if (index < rest)
    {
        begin_i = block * index + index;
        end_i = block * (index + 1) + (index + 1);
    }
    else
    {
        begin_i = block * index + rest;
        end_i = block * (index + 1) + rest;
    }
    while (begin_i!= end_i) 
    {
        H[begin_i * m] = begin_i * g;
        begin_i += 1;
    }
}

void worker_BW_compute(int& block, int& rest, int index, string s0, string s1, int ma, int mi, int g,
    vector<int>& H, vector<int>& boundary_left,vector<int>& boundary_right, int n, int m){

    int begin_j;
    int end_j;
    int p;

    if (index < rest) 
    {
        begin_j = 1 + block * index + index;
        end_j   = 1 + block * (index + 1) + (index + 1);
    } 
    else 
    {
        begin_j = 1 + block * index + rest;
        end_j   = 1 + block * (index + 1) + rest;
    }

    for (int i = 1; i < n; ++i)
    {
        for (int j = begin_j; j < end_j; ++j) 
        {   
            if ((j == begin_j) && (index != 0)){

                while(boundary_left[index - 1] == boundary_right[index - 1]){}

                if (s0[i] == s1[j]){p = ma;}
                else {p = mi;}

                H[i * m + j] = max(max(H[i * m + (j - 1)] + g, H[(i - 1) * m + (j - 1)] + p), H[(i - 1) * m + j] + g);

                boundary_right[index - 1] += 1;
            }
            else{

                if (s0[i] == s1[j]){p = ma;}
                else {p = mi;}

                H[i * m + j] = max(max(H[i * m + (j - 1)] + g, H[(i - 1) * m + (j - 1)] + p), H[(i - 1) * m + j] + g);
            }
        }
        boundary_left[index] += 1;
    }
}

vector<int> NW_Parallel_BW(string s0, string s1, int ma, int mi, int g, int num_threads)
{
    if (num_threads == 1){return NW(s0,s1,ma,mi,g);}

    int n = s0.length();
    int m = s1.length();

    vector<int> H(n * m);
    vector<int> boundary_left(num_threads - 1, 0);
    vector<int> boundary_right(num_threads - 1, 0);
    vector<thread> threads(num_threads - 1);

    bool finished;
    int num_elem;
    int block;
    int rest;
    int working_threads;

    //INITIALIZING ROW

    num_elem = m;

    block = num_elem / num_threads;
    rest = num_elem % num_threads;

    working_threads = num_threads - 1;

    if(num_elem < num_threads){working_threads = rest;}

    for (int k = 0; k < working_threads; k++) 
    {
        threads[k] = thread(worker_BW_init_row, ref(block), ref(rest),
        k, s0, s1, ma, mi, g, ref(H), ref(boundary_left), ref(boundary_right), n, m);
    }

    if (num_threads < num_elem)
    {
        int begin_j = block * (num_threads - 1) + rest;
        int end_j = num_elem;

        while (begin_j!=end_j) 
        {
            H[begin_j] = begin_j * g;
            begin_j += 1;
        }
    }

    for (int k=0; k < num_threads - 1;k++){threads[k].join();}

    //INITIALIZING COLUMN

    num_elem = n;

    block = num_elem / num_threads;
    rest = num_elem % num_threads;

    working_threads = num_threads - 1;

    if(num_elem < num_threads){working_threads = rest;}

    for (int k = 0; k < working_threads; k++) 
    {
        threads[k] = thread(worker_BW_init_col, ref(block), ref(rest),
        k, s0, s1, ma, mi, g, ref(H), ref(boundary_left), ref(boundary_right), n, m);
    }

    if (num_threads < num_elem)
    {
        int begin_i = block * (num_threads - 1) + rest;
        int end_i = num_elem;

        while (begin_i!= end_i) 
        {
            H[begin_i * m] = begin_i * g;
            begin_i += 1;
        }
    }

    for (int k=0; k < num_threads - 1;k++){threads[k].join();}

    //COMPUTING

    num_elem = m - 1;

    block = num_elem / num_threads;
    rest = num_elem % num_threads;

    working_threads = num_threads - 1;

    if(num_elem < num_threads){working_threads = rest;}

    for (int k = 0; k < working_threads; k++) 
    {
        threads[k] = thread(worker_BW_compute, ref(block), ref(rest),
        k, s0, s1, ma, mi, g, ref(H), ref(boundary_left), ref(boundary_right), n, m);
    }

    if (num_threads < num_elem)
    {
        int begin_j = 1 + block * (num_threads - 1) + rest;
        int end_j = 1 + num_elem;
        int p;

        for (int i = 1; i < n; ++i)
        {
            for (int j = begin_j; j < end_j; ++j) 
            {   
                if (j == begin_j){

                    while(boundary_left[num_threads - 2] == boundary_right[num_threads - 2]){}

                    if (s0[i] == s1[j]){p = ma;}
                    else {p = mi;}

                    H[i * m + j] = max(max(H[i * m + (j - 1)] + g, H[(i - 1) * m + (j - 1)] + p), H[(i - 1) * m + j] + g);

                    boundary_right[num_threads - 2] += 1;
                }
                else{

                    if (s0[i] == s1[j]){p = ma;}
                    else {p = mi;}

                    H[i * m + j] = max(max(H[i * m + (j - 1)] + g, H[(i - 1) * m + (j - 1)] + p), H[(i - 1) * m + j] + g);
                }         
            }
        }
    }

    for (int k=0; k < num_threads - 1 ; k++){threads[k].join();}

    return H;
}

// int main(){

//     std::srand(std::time(0));

//     unsigned int N = 1 << 14;

//     std::string nucleotides = "ACGT";

//     // string s0 = "*TAGC";
//     string s0 = "*";
//     for (int i = 0; i < N; ++i) {
//         s0 += nucleotides[std::rand() % 4];
//     }

//     // string s1 = "*TAGTC";
//     string s1 = "*";
//     for (int i = 0; i < N; ++i) {
//         s1 += nucleotides[std::rand() % 4];
//     }

//     // std::cout << s0 << " " << s1 << std::endl;

//     cout << "RUNNING SEQUENTIAL CODE" << endl;

//     auto start = high_resolution_clock::now();
//     vector<int> H1 = NW(s0, s1, 1, -1, -2);
//     auto end = high_resolution_clock::now();

//     auto duration = duration_cast<microseconds>(end - start);
//     cout << "Sequential execution time: " << duration.count() << " µs" << endl;
//     cout << "Sequential score: " << H1.back() << endl;

//     cout<< " " << endl;

//     cout<< "RUNNING PARALLEL CODE" << endl;

//     start = high_resolution_clock::now();
//     vector<int> H2 = NW_Parallel_BW(s0, s1, 1, -1, -2, 5);
//     end = high_resolution_clock::now();

//     duration = duration_cast<microseconds>(end - start);
//     cout << "Parallel execution time: " << duration.count() << " µs" << endl;
//     cout << "Parallel score: " << H2.back() << endl;
    
//     // printMatrix(H1,strlen(s0.c_str()),strlen(s1.c_str()));
//     // printMatrix(H2,strlen(s0.c_str()),strlen(s1.c_str()));

//     return 0;
// }
// int main() {
//     using namespace std;
//     using namespace std::chrono;

//     std::srand(std::time(0));
//     std::string nucleotides = "ACGT";

//     const int min_power = 7;
//     const int max_power = 15;
//     const int num_cases = max_power - min_power + 1;

//     vector<long long> sequential_times(num_cases);
//     vector<long long> parallel_times(num_cases);

//     for (int p = min_power; p <= max_power; ++p) {
//         unsigned int N = 1 << p;
//         cout << "\nN = " << N << endl;

//         // Generate random sequences s0 and s1
//         string s0 = "*";
//         string s1 = "*";
//         for (unsigned int i = 0; i < N; ++i) {
//             s0 += nucleotides[std::rand() % 4];
//             s1 += nucleotides[std::rand() % 4];
//         }

//         cout << "RUNNING SEQUENTIAL CODE" << endl;
//         auto start = high_resolution_clock::now();
//         vector<int> H1 = NW(s0, s1, 1, -1, -2);
//         auto end = high_resolution_clock::now();
//         sequential_times[p - min_power] = duration_cast<microseconds>(end - start).count();
//         cout << "Sequential time: " << sequential_times[p - min_power] << " µs" << endl;

//         cout << "RUNNING PARALLEL CODE" << endl;
//         start = high_resolution_clock::now();
//         vector<int> H2 = NW_Parallel_BW(s0, s1, 1, -1, -2, 5); // 5 threads
//         end = high_resolution_clock::now();
//         parallel_times[p - min_power] = duration_cast<microseconds>(end - start).count();
//         cout << "Parallel time: " << parallel_times[p - min_power] << " µs" << endl;

//         int n = s0.length();
//         int m = s1.length();
//         if (check_matrix(H1, H2, n, m)) {
//             cout << "Same Matrices" << endl;
//         } else {
//             cout << "Different Matrices" << endl;
//         }
//     }

//     cout << "\n=== Summary of Execution Times ===\n";
//     cout << "N\tSequential (µs)\tParallel (µs)\n";
//     for (int i = 0; i < num_cases; ++i) {
//         unsigned int N = 1 << (i + min_power);
//         cout << N << "\t" << sequential_times[i] << "\t\t" << parallel_times[i] << endl;
//     }

//     return 0;
// }

int main() {
    using namespace std;
    using namespace std::chrono;

    std::srand(std::time(0));
    std::string nucleotides = "ACGT";

    const unsigned int N = 1 << 12; // Fixed size: 4096
    const int max_threads = 8;

    vector<long long> parallel_times(max_threads);

    // Generate random sequences s0 and s1
    string s0 = "*";
    string s1 = "*";
    for (unsigned int i = 0; i < N; ++i) {
        s0 += nucleotides[std::rand() % 4];
        s1 += nucleotides[std::rand() % 4];
    }

    cout << "RUNNING SEQUENTIAL CODE\n";
    auto start = high_resolution_clock::now();
    vector<int> H_seq = NW(s0, s1, 1, -1, -2);
    auto end = high_resolution_clock::now();
    long long sequential_time = duration_cast<microseconds>(end - start).count();
    cout << "Sequential time: " << sequential_time << " µs\n";

    for (int threads = 1; threads <= max_threads; ++threads) {
        cout << "RUNNING PARALLEL CODE with " << threads << " thread(s)\n";
        start = high_resolution_clock::now();
        vector<int> H_par = NW_Parallel_BW(s0, s1, 1, -1, -2, threads);
        end = high_resolution_clock::now();
        parallel_times[threads - 1] = duration_cast<microseconds>(end - start).count();
        cout << "Parallel time (" << threads << " threads): " << parallel_times[threads - 1] << " µs\n";

        int n = s0.length();
        int m = s1.length();
        if (check_matrix(H_seq, H_par, n, m)) {
            cout << "Same Matrices\n";
        } else {
            cout << "Different Matrices\n";
        }
    }

    cout << "\n=== Execution Times for N = " << N << " ===\n";
    cout << "Threads\tParallel Time (µs)\n";
    for (int i = 0; i < max_threads; ++i) {
        cout << (i + 1) << "\t" << parallel_times[i] << "\n";
    }

    return 0;
}
