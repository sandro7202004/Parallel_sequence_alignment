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
            int p;
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

__global__ 
void initROW(int* H, int q, int r, int g) 
{
    int index = blockIdx.x * blockDim.x + threadIdx.x;
    if (index < r)
    {
            int begin_j = (q)*index+index;
            int end_j = (q)*(index+1)+(index+1);

            while (begin_j!=end_j) 
            {
                H[begin_j] = begin_j*g;

                begin_j+=1;
            }
        }
        else
        {
            int begin_j = (q)*index+(r);
            int end_j = (q)*(index+1)+(r);
                            
            while (begin_j!=end_j) 
            {
                H[begin_j] = begin_j*g;
                begin_j+=1;
            }
    }
}

__global__ 
void initCOL(int* H, int q, int r, int m, int g) 
{
    int index = blockIdx.x * blockDim.x + threadIdx.x;
    if (index < r)
    {
            int begin_i = (q)*index+index;
            int end_i = (q)*(index+1)+(index+1);

            while (begin_i!=end_i) 
            {
                H[begin_i*m] = begin_i*g;

                begin_i+=1;
            }
        }
        else
        {
            int begin_i = (q)*index+(r);
            int end_i = (q)*(index+1)+(r);
                            
            while (begin_i!=end_i) 
            {
                H[begin_i*m] = begin_i*g;
                begin_i+=1;
            }
    }
}

__global__ 
void compute(int* H, int i, int j, int q, int r, int m, char* s0, char* s1, int g) 
{
    int p;
    int index = blockIdx.x * blockDim.x + threadIdx.x;
    if (index < r)
    {
        int begin_i = i-(q)*index-index;
        int end_i = i-(q)*(index+1)-(index+1);
        int begin_j = j+(q)*index+index;
        int end_j = j+(q)*(index+1)+(index+1);

        while(begin_i!= end_i && begin_j!=end_j) 
        {
            if (s0[begin_i] == s1[begin_j]) 
            {
                p = 1;
            }
            else 
            {
                p = -1;
            }
            int res = max(max(H[(begin_i-1)*m + (begin_j-1)] + p, H[(begin_i-1)*m + (begin_j)] + g), H[(begin_i)*m + (begin_j-1)] + g);

            H[begin_i*m + begin_j] = res;

            begin_i-=1;
            begin_j+=1;
        }
    }
    else
    {
        int begin_i = i-(q)*index-(r);
        int end_i = i-(q)*(index+1)-(r);
        int begin_j = j+(q)*index+(r);
        int end_j = j+(q)*(index+1)+(r);

        while(begin_i!= end_i && begin_j!=end_j) 
        {
            if (s0[begin_i] == s1[begin_j]) 
            {
                p = 1;
            }
            else 
            {
                p = -1;
            }
            int res = max(max(H[(begin_i-1)*m + (begin_j-1)] + p, H[(begin_i-1)*m + (begin_j)] + g), H[(begin_i)*m + (begin_j-1)] + g);

            H[begin_i*m + begin_j] = res;

            begin_i-=1;
            begin_j+=1;
        }
    }
}


vector<int> NW_Parallel_GPU(string s0, string s1, int ma, int mi, int g)
{

    const size_t BLOCKS_NUM = 48;
    const size_t THREADS_PER_BLOCK = 256;
    const size_t num_threads = BLOCKS_NUM * THREADS_PER_BLOCK;

    int n = s0.length();
    int m = s1.length();

    int q = m/num_threads;
    int r = m%num_threads;

    int* H;
    cudaMalloc(&H, n * m * sizeof(int));

    char* d_s0;
    char* d_s1;

    cudaMalloc(&d_s0, n * sizeof(char));
    cudaMalloc(&d_s1, m * sizeof(char));

    cudaMemcpy(d_s0, s0.c_str(), n * sizeof(char), cudaMemcpyHostToDevice);
    cudaMemcpy(d_s1, s1.c_str(), m * sizeof(char), cudaMemcpyHostToDevice);

    initROW<<<BLOCKS_NUM, THREADS_PER_BLOCK>>>(H,q,r,g);

    cudaDeviceSynchronize();

    initCOL<<<BLOCKS_NUM, THREADS_PER_BLOCK>>>(H,q,r,m,g);

    cudaDeviceSynchronize();

    int i = 1;
    int j = 1;
    int num_elem;

    while(i!= n && j!= m)
    {   

        num_elem = min(i,m-j);

        q = num_elem/num_threads;
        r = num_elem%num_threads;

        compute<<<BLOCKS_NUM, THREADS_PER_BLOCK>>>(H,i,j,q,r,m,d_s0,d_s1,g);
        cudaDeviceSynchronize();

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

    int* res = (int*)malloc(sizeof(int) * n * m);
    cudaMemcpy(res, H, n * m * sizeof(int), cudaMemcpyDeviceToHost);

    std::vector<int> result(res, res + n * m);
    
    free(res);

    cudaFree(H);

    return result;
}

void printMatrix(const std::vector<int>& data, int n, int m) {

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            std::cout << data[i * m + j] << " ";
        }
        std::cout << "\n";
    }
}

int main() {
    std::srand(std::time(0));

    unsigned int N = 1 << 13;

    const char nucleotides[] = {'A', 'T', 'G', 'C'};

    // string s0 = "*TAGC";
    string s0 = "*";
    for (int i = 0; i < N; ++i) {
        s0 += nucleotides[std::rand() % 4];
    }

    // string s1 = "*TAGTC";
    string s1 = "*";
    for (int i = 0; i < N; ++i) {
        s1 += nucleotides[std::rand() % 4];
    }

    std::cout << "RUNNING SEQUENTIAL CODE" << std::endl;
    auto start = high_resolution_clock::now();
    vector<int> H1 = NW(s0, s1, 1, -1, -2);
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(end - start);
    cout << "Sequential execution time: " << duration.count() << " µs" << endl;
    std::cout << "Sequential score: " << H1.back() << std::endl;

    std::cout << "\nRUNNING GPU PARALLEL CODE" << std::endl;

    start = high_resolution_clock::now();
    std::vector<int> H2 = NW_Parallel_GPU(s0, s1, 1, -1, -2);
    end = high_resolution_clock::now();
    duration = duration_cast<microseconds>(end - start);
    std::cout << "GPU execution time: " << duration.count() << " µs" << std::endl;
    std::cout << "GPU score: " << H2.back() << std::endl;

    // printMatrix(H1,strlen(s0.c_str()),strlen(s1.c_str()));
    // printf("\n");
    // printMatrix(H2,strlen(s0.c_str()),strlen(s1.c_str()));

    int n = s0.length();
    int m = s1.length();

    if (check_matrix(H1,H2,n,m))
    {
        std::cout << "\nSame Matrices" << std::endl;
    }
    else 
    {
        std::cout << "\nDifferent Matrices" << std::endl;
    }

    return 0;
}