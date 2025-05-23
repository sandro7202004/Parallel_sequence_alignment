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

vector<vector<pair<int, pair<int, int>>>> NW(string s0, string s1, int ma, int mi, int g)
{

    int n = strlen(s0.c_str());
    int m = strlen(s1.c_str());

    vector<vector<pair<int, pair<int, int>>>> H(n, vector<pair<int, pair<int, int>>>(m));
    
    H[0][0] = {0, {0, 0}};

    for (int i = 1; i<n; i++) {
        H[i][0] = {i*g, {i-1, 0}};
    }

    for (int j = 1; j<m; j++) {
        H[0][j] = {j*g, {0, j-1}};
    }

    for (int i=1; i <n; i++)
    {
        for (int j=1; j<m; j++) 
        {
            pair<int, pair<int, int>> res = {H[i-1][j-1].first + p(i,j,s0,s1,ma,mi),{i-1,j-1}};

            if (H[i][j-1].first + g > res.first) 
            {
                res = {H[i][j-1].first + g, {i,j-1}};
            }

            if (H[i-1][j].first + g > res.first)
            {
                res = {H[i-1][j].first + g, {i-1,j}};
            }

            H[i][j] = res;
        }
    }

    return H;

}

void printMatrix(const vector<vector<pair<int, pair<int, int>>>>& H) {
    for (size_t i = 0; i < H.size(); ++i) {
        for (size_t j = 0; j < H[i].size(); ++j) {
            const auto& p = H[i][j];
            cout << "(" << p.first << ", (" << p.second.first << ", " << p.second.second << ")) ";
        }
        cout << endl;
    }
}

// void NW_Parallel_AUX2(int i_begin, int j_begin, int i_end, int j_end, string s0, string s1, int ma, int mi, int g, vector<vector<pair<int, pair<int, int>>>>& H, int n, int m)
// {
//     int i = i_begin;
//     int j = j_begin;

//     while (i!= i_end && j!=j_end) 
//     {
//         pair<int, pair<int, int>> res = {H[i-1][j-1].first + p(i,j,s0,s1,ma,mi),{i-1,j-1}};

//         if (H[i][j-1].first + g > res.first) 
//         {
//             res = {H[i][j-1].first + g, {i,j-1}};
//         }

//         if (H[i-1][j].first + g > res.first)
//         {
//             res = {H[i-1][j].first + g, {i-1,j}};
//         }

//         H[i][j] = res;

//         i-=1;
//         j+=1;

//     }
    
// }

// vector<vector<pair<int, pair<int, int>>>> NW_Parallel(string s0, string s1, int ma, int mi, int g)
// {

//     unsigned int num_threads = std::thread::hardware_concurrency();

//     std::vector<std::thread> threads(num_threads-1);

//     int n = s0.length();
//     int m = s1.length();

//     vector<vector<pair<int, pair<int, int>>>> H(n, vector<pair<int, pair<int, int>>>(m));

    // H[0][0] = {0, {0, 0}};

    // for (int i = 1; i<n; i++) {
    //     H[i][0] = {i*g, {i-1, 0}};
    // }

    // for (int j = 1; j<m; j++) {
    //     H[0][j] = {j*g, {0, j-1}};
    // }

//     int i = 1;
//     int j = 1;

//     while (i != n && j!= m)
//     {

//         int num_elem = min(i,min(n-1,m-1));

//         int batch_size = num_elem/num_threads;

//         int start_block_i = i;
//         int start_block_j = j;

//         for (int k =0; k< num_threads-1; ++k) 
//         {
//             int end_block_i = start_block_i - batch_size;
//             int end_block_j = start_block_j + batch_size;
//             threads[k] = thread(NW_Parallel_AUX2,start_block_i,start_block_j,end_block_i,end_block_j,s0,s1,ma,mi,g,std::ref(H),n,m);
//             start_block_i = end_block_i;
//             start_block_j = end_block_j;
//         }

//         NW_Parallel_AUX2(start_block_i,start_block_j,i-num_elem,j+num_elem,s0,s1,ma,mi,g,std::ref(H),n,m);

//         for (int h=0; h<num_threads-1; h++) 
//         {
//             threads[h].join();
//         }

//         if (i!= n-1) 
//         {
//             i+=1;
//         }
//         else 
//         {
//             if (j!= m-1)
//             {
//                 j+=1;
//             }
//             else 
//             {
//                 return H;
//             }
//         }

//     }

// }

void worker(int& i,int& j, int& working, int& q, int& r, int index, string s0, string s1, int ma, int mi, int g, vector<vector<pair<int, pair<int, int>>>>& H, int n, int m)
{
    while (true)
    {
        if (i==n && j==m)
        {
            return;
        }
        if (working == 1)
        {
            if (q==0)
            {
                if (index < r)
                {
                    if (q==0)
                    {
                        int begin_i = i-index;
                        int end_i = i-(index+1);
                        int begin_j = j+index;
                        int end_j = j+(index+1);

                        // std::cout << "Thread point i: " << begin_i << " " << end_i << std::endl;
                        // std::cout << "Thread point j: " << begin_j << " " << end_j << std::endl;

                        while (begin_i!= end_i && begin_j!=end_j) 
                        {
                            pair<int, pair<int, int>> res = {H[begin_i-1][begin_j-1].first + p(begin_i,begin_j,s0,s1,ma,mi),{begin_i-1,begin_j-1}};

                            if (H[begin_i][begin_j-1].first + g > res.first) 
                            {
                                res = {H[begin_i][begin_j-1].first + g, {begin_i,begin_j-1}};
                            }

                            if (H[begin_i-1][begin_j].first + g > res.first)
                            {
                                res = {H[begin_i-1][begin_j].first + g, {begin_i-1,begin_j}};
                            }

                            H[begin_i][begin_j] = res;

                            begin_i-=1;
                            begin_j+=1;

                        }
                    }
                }
            }
            else 
            {
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
                        pair<int, pair<int, int>> res = {H[begin_i-1][begin_j-1].first + p(begin_i,begin_j,s0,s1,ma,mi),{begin_i-1,begin_j-1}};

                        if (H[begin_i][begin_j-1].first + g > res.first) 
                        {
                            res = {H[begin_i][begin_j-1].first + g, {begin_i,begin_j-1}};
                        }

                        if (H[begin_i-1][begin_j].first + g > res.first)
                        {
                            res = {H[begin_i-1][begin_j].first + g, {begin_i-1,begin_j}};
                        }

                        H[begin_i][begin_j] = res;

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
                        pair<int, pair<int, int>> res = {H[begin_i-1][begin_j-1].first + p(begin_i,begin_j,s0,s1,ma,mi),{begin_i-1,begin_j-1}};

                        if (H[begin_i][begin_j-1].first + g > res.first) 
                        {
                            res = {H[begin_i][begin_j-1].first + g, {begin_i,begin_j-1}};
                        }

                        if (H[begin_i-1][begin_j].first + g > res.first)
                        {
                            res = {H[begin_i-1][begin_j].first + g, {begin_i-1,begin_j}};
                        }

                        H[begin_i][begin_j] = res;

                        begin_i-=1;
                        begin_j+=1;

                    }
                }
            }
            working = 0;
        }
    }
}

vector<vector<pair<int, pair<int, int>>>> NW_Parallel(string s0, string s1, int ma, int mi, int g)
{

    unsigned int num_threads = std::thread::hardware_concurrency();

    vector<int> working(num_threads-1);

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

    vector<vector<pair<int, pair<int, int>>>> H(n, vector<pair<int, pair<int, int>>>(m));

    for (int k =0; k < num_threads-1; k++) 
    {
        threads[k] = thread(worker,std::ref(i),std::ref(j),std::ref(working[k]), std::ref(q), std::ref(r),k, s0, s1, ma, mi, g, std::ref(H), n,m);
    }

    H[0][0] = {0, {0, 0}};

    for (int i = 1; i<n; i++) {
        H[i][0] = {i*g, {i-1, 0}};
    }

    for (int j = 1; j<m; j++) {
        H[0][j] = {j*g, {0, j-1}};
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
                pair<int, pair<int, int>> res = {H[begin_i-1][begin_j-1].first + p(begin_i,begin_j,s0,s1,ma,mi),{begin_i-1,begin_j-1}};

                if (H[begin_i][begin_j-1].first + g > res.first) 
                {
                   res = {H[begin_i][begin_j-1].first + g, {begin_i,begin_j-1}};
                }

                if (H[begin_i-1][begin_j].first + g > res.first)
                {
                    res = {H[begin_i-1][begin_j].first + g, {begin_i-1,begin_j}};
                }

                H[begin_i][begin_j] = res;

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

int main() {
    unsigned int num_threads = std::thread::hardware_concurrency();
    std::cout << "Number of hardware threads available: " << num_threads << std::endl;

    unsigned int N = 1 << 8;

    string s0 = "*";
    for (int i = 0; i < N; ++i) {
        s0 += "TAGC";
    }

    string s1 = "*";
    for (int i = 0; i < N; ++i) {
        s1 += "TAGTC";
    }

    auto start = high_resolution_clock::now();
    vector<vector<pair<int, pair<int, int>>>> H1 = NW(s0, s1, 1, -1, -2);
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(end - start);
    cout << "Sequential execution time: " << duration.count() << " µs" << endl;

    start = high_resolution_clock::now();
    vector<vector<pair<int, pair<int, int>>>> H2 = NW_Parallel(s0, s1, 1, -1, -2);
    end = high_resolution_clock::now();
    duration = duration_cast<microseconds>(end - start);
    cout << "Parallel execution time: " << duration.count() << " µs" << endl;

    std::cout << "Sequential score: " << H1[H1.size() - 1][H1[0].size() - 1].first << std::endl;
    std::cout << "Parallel score: " << H2[H2.size() - 1][H2[0].size() - 1].first << std::endl;

    return 0;
}