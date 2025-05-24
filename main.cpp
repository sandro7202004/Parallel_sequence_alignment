#include <iostream>
#include <utility> 
#include <vector>
#include <cstring> 
#include <thread>
#include <chrono>
using namespace std;
using namespace std::chrono; 

int p(int i, int j, string s0, string s1, int ma, int mi) 
{
    
    if (s0[i] == s1[j]) 
    {
        return ma;
    }

    return mi;
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
            int res = max(max(H[(i-1)*m + (j-1)] + p(i,j,s0,s1,ma,mi), H[(i-1)*m + (j)] + g), H[(i)*m + (j-1)] + g);

            H[i*m + j] = res;
        }
    }

    return H;

}

void worker(int& i,int& j, int& working, int& q, int& r, int index, string s0, string s1, int ma, int mi, int g, vector<int>& H, int n, int m)
{
    while (true)
    {
        if (i==n && j==m)
        {
            return;
        }
        if (working == 1)
        {
            // if (q==0)
            // {
            //     if (index < r)
            //     {
            //         if (q==0)
            //         {
            //             int begin_i = i-index;
            //             int end_i = i-(index+1);
            //             int begin_j = j+index;
            //             int end_j = j+(index+1);

            //             // std::cout << "Thread point i: " << begin_i << " " << end_i << std::endl;
            //             // std::cout << "Thread point j: " << begin_j << " " << end_j << std::endl;

            //             while (begin_i!= end_i && begin_j!=end_j) 
            //             {
            //                 int res = max(max(H[(begin_i-1)*m + (begin_j-1)] + p(begin_i,begin_j,s0,s1,ma,mi), H[(begin_i-1)*m + (begin_j)] + g), H[(begin_i)*m + (begin_j-1)] + g);

            //                 H[begin_i*m + begin_j] = res;

            //                 begin_i-=1;
            //                 begin_j+=1;

            //             }
            //         }
            //     }
            // }
            if (index < r)
            {
                    int begin_i = i-(q)*index-index;
                    int end_i = i-(q)*(index+1)-(index+1);
                    int begin_j = j+(q)*index+index;
                    int end_j = j+(q)*(index+1)+(index+1);

                    // std::cout << "Thread point i: " << begin_i << " " << begin_j << std::endl;
                    // std::cout << "Thread point j: " << end_i << " " << end_j << std::endl;

                    while (begin_i!= end_i && begin_j!=end_j) 
                    {
                        int res = max(max(H[(begin_i-1)*m + (begin_j-1)] + p(begin_i,begin_j,s0,s1,ma,mi), H[(begin_i-1)*m + (begin_j)] + g), H[(begin_i)*m + (begin_j-1)] + g);

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
                        int res = max(max(H[(begin_i-1)*m + (begin_j-1)] + p(begin_i,begin_j,s0,s1,ma,mi), H[(begin_i-1)*m + (begin_j)] + g), H[(begin_i)*m + (begin_j-1)] + g);

                        H[begin_i*m + begin_j] = res;

                        begin_i-=1;
                        begin_j+=1;

                    }
            }
            working = 0;
        }
    }
}

vector<int> NW_Parallel(string s0, string s1, int ma, int mi, int g, int num_threads)
{

    vector<int> working(num_threads);

    for (int k = 0; k < num_threads-1; k++)
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
        threads[k] = thread(worker,std::ref(i),std::ref(j),std::ref(working[k]), std::ref(q), std::ref(r),k, s0, s1, ma, mi, g, std::ref(H), n,m);
    }

    H[0] = 0;

    for (int i2 = 1; i2<n; i2++) {
        H[i2*m] = i2*g;
    }

    for (int j2 = 1; j2<m; j2++) {
        H[j2] = j2*g;
    }

    i = 1;
    j = 1;

    while (i != n && j!= m)
    {

        // std::cout << "Point" << std::endl;
        // std::cout << i << " " << j << std::endl;

        int num_elem = min(i,min(n-1,m-1));

        // std::cout << "num_elem: " << num_elem << std::endl;

        if(num_elem < num_threads)
        {
            q = 0;
            r = num_elem;
            for (int k = 0; k < r; k++)
            {
                working[k] = 1;
            }

        }

        else
        {
            
            q = num_elem/num_threads;
            r = num_elem%num_threads;

            for (int k = 0; k < working.size(); k++)
            {
                working[k] = 1;
            }
        }

        if(working[num_threads-1]==1)
        {
            int begin_i = i-(q)*(num_threads-1)-r;
            int end_i = i-num_elem;
            int begin_j = j+(q)*(num_threads-1)+r;
            int end_j = j+num_elem;

            while (begin_i!= end_i && begin_j!=end_j) 
            {
                int res = max(max(H[(begin_i-1)*m + (begin_j-1)] + p(begin_i,begin_j,s0,s1,ma,mi), H[(begin_i-1)*m + (begin_j)] + g), H[(begin_i)*m + (begin_j-1)] + g);

                H[begin_i*m + begin_j] = res;

                begin_i-=1;
                begin_j+=1;
            }
            working[num_threads-1] = 0;
        }

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

int main() {

    unsigned int N = 1 << 13;

    string s0 = "*TAGC";
    // string s0 = "*";
    // for (int i = 0; i < N; ++i) {
    //     s0 += "T";
    // }

    string s1 = "*TAGTC";
    // string s1 = "*";
    // for (int i = 0; i < N; ++i) {
    //     s1 += "T";
    // }

    auto start = high_resolution_clock::now();
    vector<int> H1 = NW(s0, s1, 1, -1, -2);
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(end - start);
    cout << "Sequential execution time: " << duration.count() << " µs" << endl;

    start = high_resolution_clock::now();
    vector<int> H2 = NW_Parallel(s0, s1, 1, -1, -2, 5);
    end = high_resolution_clock::now();
    duration = duration_cast<microseconds>(end - start);
    cout << "Parallel execution time: " << duration.count() << " µs" << endl;

    std::cout << "Sequential score: " << H1.back() << std::endl;
    printMatrix(H1,strlen(s0.c_str()),strlen(s1.c_str()));
    std::cout << "Parallel score: " << H2.back() << std::endl;
    printMatrix(H2,strlen(s0.c_str()),strlen(s1.c_str()));

    return 0;
}