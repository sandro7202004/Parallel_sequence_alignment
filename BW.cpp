#include <iostream>
#include <atomic>
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
    if (s0[i] == s1[j]){return ma;}
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

vector<int> NW_sequential(string s0, string s1, int ma, int mi, int g)
{
    int n = strlen(s0.c_str());
    int m = strlen(s1.c_str());

    vector<int> H(n*m);

    H[0] = 0;

    for (int i = 1; i < n; i++){H[i * m] = i * g;}
    for (int j = 1; j < m; j++){H[j] = j * g;}
    for (int i = 1; i < n; i++){for (int j=1; j<m; j++){NW_step(H,i,j,s0,s1,ma,mi,g);}}

    return H;
}

void worker_BW(int& working, int& mode, int& q, int& r, int index, string s0, string s1, int ma, int mi, int g,
     vector<int>& H, vector<atomic<int>>& boundary, int n, int m)
{
    while (true)
    {
        if (working == 1)
        {   
            int begin_i;
            int end_i;
            int begin_j;
            int end_j;

            if (mode == 0)  // INITIALIZING FIRST ROW
            {
                if (index < r)
                {
                    begin_j = q * index + index;
                    end_j = q *(index+1) + (index+1);
                }
                else
                {
                    begin_j = q * index + r;
                    end_j = q * (index+1) + r;
                }

                while (begin_j != end_j) 
                {
                    H[begin_j] = begin_j * g;
                    begin_j+=1;
                }
                working = 0;
            }
            else if (mode == 1) // INITIALIZING FIRST COLUMN
            {
                if (index < r)
                {
                    begin_i = (q)*index+index;
                    end_i = (q)*(index+1)+(index+1);
                }
                else
                {
                    begin_i = (q)*index+r;
                    end_i = (q)*(index+1)+r;
                }
                while (begin_i!= end_i) 
                {
                    H[begin_i*m] = begin_i*g;
                    begin_i+=1;
                }
                working = 0;
            }
            else // COMPUTE THE COEFFICIENTS 
            {   
                if (index < r) 
                {
                    begin_j = 1 + q * index + index;
                    end_j   = 1 + q * (index + 1) + (index + 1);
                } 
                else 
                {
                    begin_j = 1 + q * index + r;
                    end_j   = 1 + q * (index + 1) + r;
                }
            
                for (int ii = 1; ii < n; ++ii)
                {
                    for (int jj = begin_j; jj < end_j; ++jj) 
                    {   
                        if ((jj == begin_j) && (index != 0)){
                            while(boundary[index - 1].load() == 0){}
                            NW_step(H,ii,jj,s0,s1,ma,mi,g);
                            boundary[index - 1].fetch_sub(1);
                        }
                        else{
                            NW_step(H,ii,jj,s0,s1,ma,mi,g);
                        }
                    }
                    boundary[index].fetch_add(1);
                }
                working = 0;
                return;
            }
        }
    }
}

vector<int> NW_Parallel_BW(string s0, string s1, int ma, int mi, int g, int num_threads)
{

    int n = s0.length();
    int m = s1.length();

    vector<int> H(n * m);
    vector<int> working(num_threads - 1, 0);
    std::vector<std::atomic<int>> boundary(num_threads);
    for (int i = 0; i < num_threads; ++i){boundary[i].store(0, memory_order_relaxed);}
    vector<thread> threads(num_threads - 1);

    int mode;
    bool finished;
    int num_elem;
    int q;
    int r;
    int working_threads;

    for (int k = 0; k < num_threads - 1; k++) 
    {
        threads[k] = thread(worker_BW, ref(working[k]), ref(mode), ref(q), ref(r),
        k, s0, s1, ma, mi, g, ref(H), ref(boundary), n, m);
    }

    //INITIALIZING ROW

    mode = 0;
    num_elem = m;

    q = num_elem/num_threads;
    r = num_elem%num_threads;

    working_threads = num_threads - 1;

    if(num_elem < num_threads){working_threads = r;}

    for (int k = 0; k < working_threads; k++) {working[k] = 1;}

    {
        int begin_j = q * (num_threads-1) + r;
        int end_j = num_elem;

        while (begin_j!=end_j) 
        {
            H[begin_j] = begin_j * g;
            begin_j+=1;
        }
    }

    finished = false;

    while(!finished)
    {
        int res = 0;
        for (int k = 0; k < working_threads; k++){res += working[k];}
        finished = (res == 0);
    }

    //INITIALIZING COLUMN

    mode = 1;
    num_elem = n;

    q = num_elem/num_threads;
    r = num_elem%num_threads;

    working_threads = num_threads - 1;

    if(num_elem < num_threads){working_threads = r;}

    for (int k = 0; k < working_threads; k++){working[k] = 1;}

    {
        int begin_i = q * (num_threads - 1) + r;
        int end_i = num_elem;

        while (begin_i!= end_i) 
        {
            H[begin_i*m] = begin_i*g;
            begin_i+=1;
        }
    }

    finished = false;

    while(!finished)
    {
        int res = 0;
        for (int k = 0; k< working_threads; k++){res += working[k];}    
        finished = (res == 0);
    }

    //COMPUTING

    mode = 2;
    num_elem = m - 1;

    q = num_elem/num_threads;
    r = num_elem%num_threads;

    working_threads = num_threads - 1;

    if(num_elem < num_threads){working_threads = r;}

    for (int k = 0; k < working_threads; k++) {working[k] = 1;}

    {
        int begin_j = 1 + q * (num_threads - 1) + r;
        int end_j = 1 + num_elem;

        for (int ii = 1; ii < n; ++ii)
        {
            for (int jj = begin_j; jj < end_j; ++jj) 
            {   
                if (jj == begin_j){
                    while(boundary[num_threads - 2].load() == 0){}
                    NW_step(H,ii,jj,s0,s1,ma,mi,g);
                    boundary[num_threads - 2].fetch_sub(1);
                }
                else{
                    NW_step(H,ii,jj,s0,s1,ma,mi,g);
                }
            }
            boundary[num_threads - 1].fetch_add(1);           
        }
    }

    finished = false;

    while(!finished)
    {
        int res = 0;
        for (int k = 0; k < working_threads; k++){res += working[k];}    
        finished = (res == 0);
    }

    for (int k=0; k < num_threads - 1;k++){threads[k].join();}

    return H;
}

int main() {

    std::srand(std::time(0));

    unsigned int N = 1<<11;

    std::cout<< N << std::endl;

    std::string nucleotides = "ACGT";

    string s0 = "*";
    for (int i = 0; i < N; ++i) {s0 += nucleotides[std::rand() % 4];}

    string s1 = "*";
    for (int i = 0; i < N; ++i) {s1 += nucleotides[std::rand() % 4];}

    std::cout << "RUNNING SEQUENTIAL CODE" << std::endl;

    auto start = high_resolution_clock::now();
    vector<int> H1 =  NW_sequential(s0, s1, 1, -1, -2);
    auto end = high_resolution_clock::now();

    auto duration = duration_cast<microseconds>(end - start);
    cout << "Sequential execution time: " << duration.count() << " µs" << endl;
    std::cout << "Sequential score: " << H1.back() << std::endl;

    std::cout<< " " << std::endl;

    std::cout<< "RUNNING PARALLEL CODE" << std::endl;

    start = high_resolution_clock::now();
    vector<int> H2 = NW_Parallel_BW(s0, s1, 1, -1, -2, 5);
    end = high_resolution_clock::now();

    duration = duration_cast<microseconds>(end - start);
    cout << "Parallel execution time: " << duration.count() << " µs" << endl;
    std::cout << "Parallel score: " << H2.back() << std::endl;
    return 0;
}
