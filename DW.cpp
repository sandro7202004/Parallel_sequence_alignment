#include <iostream>
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

void worker(int& i,int& j, int& working, int& mode, int& q, int& r, int index, string s0, string s1, int ma, int mi, int g, vector<int>& H, int n, int m)
{
    int p;
    while (true)
    {
        if (i==n && j==m)
        {
            return;
        }
        if (working == 1)
        {
            if (mode == 1)
            {
                if (index < r)
                {
                        int begin_j = j+(q)*index+index;
                        int end_j = j+(q)*(index+1)+(index+1);

                        while (begin_j!=end_j) 
                        {
                            H[begin_j] = begin_j*g;

                            begin_j+=1;
                        }
                }
                else
                {
                        int begin_j = j+(q)*index+r;
                        int end_j = j+(q)*(index+1)+r;
                        
                        while (begin_j!=end_j) 
                        {
                            H[begin_j] = begin_j*g;
                            begin_j+=1;
                        }
                }
                working = 0;
            }
            else if (mode == 2)
            {
                if (index < r)
                {
                        int begin_i = i+(q)*index+index;
                        int end_i = i+(q)*(index+1)+(index+1);

                        while (begin_i!= end_i) 
                        {
                            H[begin_i*m] = begin_i*g;
                            begin_i+=1;
                        }
                }
                else
                {
                    int begin_i = i+(q)*index+r;
                    int end_i = i+(q)*(index+1)+r;

                    while (begin_i!= end_i) 
                    {
                        H[begin_i*m] = begin_i*g;
                        begin_i+=1;
                    }
                }
                working = 0;
            }
            else if (mode == 3)
            {
                if (index < r)
                {
                        int begin_i = i-(q)*index-index;
                        int end_i = i-(q)*(index+1)-(index+1);
                        int begin_j = j+(q)*index+index;
                        int end_j = j+(q)*(index+1)+(index+1);

                        while (begin_i!= end_i && begin_j!=end_j) 
                        {
                            if (s0[begin_i] == s1[begin_j]) 
                            {
                                p = ma;
                            }
                            else 
                            {
                                p = mi;
                            }   

                            int res = max(max(H[(begin_i-1)*m + (begin_j-1)] + p, H[(begin_i-1)*m + (begin_j)] + g), H[(begin_i)*m + (begin_j-1)] + g);

                            H[begin_i*m + begin_j] = res;

                            begin_i-=1;
                            begin_j+=1;
                        }
                }
                else
                {
                        int begin_i = i-(q)*index-r;
                        int end_i = i-(q)*(index+1)-r;
                        int begin_j = j+(q)*index+r;
                        int end_j = j+(q)*(index+1)+r;

                        while (begin_i!= end_i && begin_j!=end_j) 
                        {
                            if (s0[begin_i] == s1[begin_j]) 
                            {
                                p = ma;
                            }
                            else {
                                p = mi;
                            }

                            int res = max(max(H[(begin_i-1)*m + (begin_j-1)] + p, H[(begin_i-1)*m + (begin_j)] + g), H[(begin_i)*m + (begin_j-1)] + g);

                            H[begin_i*m + begin_j] = res;

                            begin_i-=1;
                            begin_j+=1;

                        }
                }
                working = 0;
            }
        }
    }
}

vector<int> NW_Parallel(string s0, string s1, int ma, int mi, int g, int num_threads)
{

    int mode = 0;

    vector<int> working(num_threads);

    for (int k = 0; k < num_threads; k++)
    {
        working[k] = 0;
    }

    int n = s0.length();
    int m = s1.length();

    int i = 0;
    int j = 0;

    int q = 0;
    int r = 0;

    std::vector<std::thread> threads(num_threads-1);

    vector<int> H(n*m);

    for (int k =0; k < num_threads-1; k++) 
    {
        threads[k] = thread(worker,std::ref(i),std::ref(j),std::ref(working[k]), std::ref(mode), std::ref(q), std::ref(r),k, s0, s1, ma, mi, g, std::ref(H), n,m);
    }

    //initializing

    //initializing row

    std::cout<< "INITIALIZING ROW" << std::endl;

    mode = 1;

    int num_elem = m;
            
    q = num_elem/num_threads;
    r = num_elem%num_threads;

    for (int k = 0; k < working.size(); k++)
    {
        working[k] = 1;
    }

    int begin_j = (q)*(num_threads-1)+r;
    int end_j = (q)*(num_threads)+r;

    while (begin_j!=end_j) 
    {
        H[begin_j] = begin_j*g;

        begin_j+=1;
    }
    working[num_threads-1] = 0;

    bool finished = false;

    while(!finished)
    {
        int res = 0;

        for (int k = 0; k< working.size(); k++)
        {
            res += working[k];
        }
                
        finished = (res == 0);
    }

    mode = 2;

    //initializing col

    std::cout<< "INITIALIZING COL" << std::endl;

    num_elem = n;
            
    q = num_elem/num_threads;
    r = num_elem%num_threads;

    for (int k = 0; k < working.size(); k++)
    {
        working[k] = 1;
    }

    int begin_i = (q)*(num_threads-1)+r;
    int end_i = (q)*(num_threads)+r;

    while (begin_i!= end_i) 
    {

        H[begin_i*m] = begin_i*g;

        begin_i+=1;
    }
    working[num_threads-1] = 0;

    finished = false;

    while(!finished)
    {
        int res = 0;

        for (int k = 0; k< working.size(); k++)
        {
            res += working[k];
        }
                
        finished = (res == 0);
    }

    mode = 3;

    //computing

    std::cout<< "COMPUTING" << std::endl;

    i = 1;
    j = 1;

    int p;

    while (i != n && j!= m)
    {

        num_elem = min(i,m-j);
            
        q = num_elem/num_threads;
        r = num_elem%num_threads;

        for (int k = 0; k < working.size(); k++)
        {
            working[k] = 1;
        }

        int begin_i = i-(q)*(num_threads-1)-r;
        int end_i = i-(q)*(num_threads)-r;
        int begin_j = j+(q)*(num_threads-1)+r;
        int end_j = j+(q)*(num_threads)+r;

        while (begin_i!= end_i && begin_j!=end_j) 
        {
            if (s0[begin_i] == s1[begin_j]) 
            {
                p = ma;
            }
            else 
            {
                p = mi;
            }

            int res = max(max(H[(begin_i-1)*m + (begin_j-1)] + p, H[(begin_i-1)*m + (begin_j)] + g), H[(begin_i)*m + (begin_j-1)] + g);

            H[begin_i*m + begin_j] = res;

            begin_i-=1;
            begin_j+=1;
        }
        working[num_threads-1] = 0;

        finished = false;

        while(!finished)
        {
            int res = 0;

            for (int k = 0; k< working.size(); k++)
            {
                res += working[k];
            }
                
            finished = (res == 0);
        }

        if (i!= n-1) 
        {
            i+=1;
        }
        else 
        {
            if (j!= m-1)
            {
                j+=1;
            }
            else 
            {
                i+=1;
                j+=1;
            }
        }
    }
    for (int k=0; k<num_threads-1;k++)
    {   
        threads[k].join();
    }

    mode = 0;

    return H;
}

void printMatrix(const std::vector<int>& data, int n, int m) {

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            std::cout << data[i * m + j] << " ";
        }
        std::cout << "\n";
    }
}

// int main() {

//     std::srand(std::time(0));

//     unsigned int N = 1 << 7;

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
//     vector<int> H2 = NW_Parallel(s0, s1, 1, -1, -2, 5);
//     end = high_resolution_clock::now();

//     duration = duration_cast<microseconds>(end - start);
//     cout << "Parallel execution time: " << duration.count() << " µs" << endl;
//     cout << "Parallel score: " << H2.back() << endl;
    
//     // printMatrix(H1,strlen(s0.c_str()),strlen(s1.c_str()));
//     // printMatrix(H2,strlen(s0.c_str()),strlen(s1.c_str()));

//     int n = s0.length();
//     int m = s1.length();

//     if (check_matrix(H1,H2,n,m))
//     {
//         std::cout << "\nSame Matrices" << std::endl;
//     }
//     else 
//     {
//         std::cout << "\nDifferent Matrices" << std::endl;
//     }

//     return 0;
// }

int main() {
    using namespace std;
    using namespace std::chrono;

    std::srand(std::time(0));
    std::string nucleotides = "ACGT";

    const int min_power = 7;
    const int max_power = 15;
    const int num_cases = max_power - min_power + 1;

    vector<long long> sequential_times(num_cases);
    vector<long long> parallel_times(num_cases);

    for (int p = min_power; p <= max_power; ++p) {
        unsigned int N = 1 << p;
        cout << "\nN = " << N << endl;

        // Generate random sequences s0 and s1
        string s0 = "*";
        string s1 = "*";
        for (unsigned int i = 0; i < N; ++i) {
            s0 += nucleotides[std::rand() % 4];
            s1 += nucleotides[std::rand() % 4];
        }

        cout << "RUNNING SEQUENTIAL CODE" << endl;
        auto start = high_resolution_clock::now();
        vector<int> H1 = NW(s0, s1, 1, -1, -2);
        auto end = high_resolution_clock::now();
        sequential_times[p - min_power] = duration_cast<microseconds>(end - start).count();
        cout << "Sequential time: " << sequential_times[p - min_power] << " µs" << endl;

        cout << "RUNNING PARALLEL CODE" << endl;
        start = high_resolution_clock::now();
        vector<int> H2 = NW_Parallel(s0, s1, 1, -1, -2, 5); // 5 threads
        end = high_resolution_clock::now();
        parallel_times[p - min_power] = duration_cast<microseconds>(end - start).count();
        cout << "Parallel time: " << parallel_times[p - min_power] << " µs" << endl;

        int n = s0.length();
        int m = s1.length();
        if (check_matrix(H1, H2, n, m)) {
            cout << "Same Matrices" << endl;
        } else {
            cout << "Different Matrices" << endl;
        }
    }

    cout << "\n=== Summary of Execution Times ===\n";
    cout << "N\tSequential (µs)\tParallel (µs)\n";
    for (int i = 0; i < num_cases; ++i) {
        unsigned int N = 1 << (i + min_power);
        cout << N << "\t" << sequential_times[i] << "\t\t" << parallel_times[i] << endl;
    }

    return 0;
}