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

void NW_Parallel_AUX(int i, int j, string s0, string s1, int ma, int mi, int g, vector<vector<pair<int, pair<int, int>>>>& H)
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

void NW_Parallel_AUX2(int i_begin, int j_begin, int i_end, int j_end, string s0, string s1, int ma, int mi, int g, vector<vector<pair<int, pair<int, int>>>>& H, int n, int m)
{
    int i = i_begin;
    int j = j_begin;

    while (i!= i_end && j!=j_end && i!=0 && j!=m) 
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

        i-=1;
        j+=1;

    }
    
}

vector<vector<pair<int, pair<int, int>>>> NW_Parallel(string s0, string s1, int ma, int mi, int g)
{

    unsigned int num_threads = std::thread::hardware_concurrency();

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

    std::vector<std::thread> threads(num_threads-1);

    int i = 1;
    int j = 1;

    while (i != n && j!= m)
    {
        int num_elem = min(i,min(n-1,m-1));

        int batch_size = num_elem/num_threads;

        int start_block_i = i;
        int start_block_j = j;

        for (int k =0; k< num_threads-1; k++) 
        {
            int end_block_i = start_block_i - batch_size;
            int end_block_j = start_block_j + batch_size;
            threads[k] = thread(NW_Parallel_AUX2,start_block_i,start_block_j,end_block_i,end_block_j,s0,s1,ma,mi,g,std::ref(H),n,m);
            int start_block_i = end_block_i;
            int start_block_j = end_block_j;
        }

        NW_Parallel_AUX2(start_block_i,start_block_j,start_block_i-num_elem,start_block_j+num_elem,s0,s1,ma,mi,g,std::ref(H),n,m);

        for (int h=0; h<num_threads-1; h++) 
        {
            threads[h].join();
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

    return H;

}

int main() {

    unsigned int N = 1 << 12;

    string s0 = "*";
    for (int i = 0; i < N; ++i) {
        s0 += "T";
    }

    string s1 = "*";
    for (int i = 0; i < N; ++i) {
        s1 += "A";
    }

    // Start timer
    auto start = high_resolution_clock::now();

    vector<vector<pair<int, pair<int, int>>>> H1 = NW(s0, s1, 1, -1, -2);

    // End timer
    auto end = high_resolution_clock::now();

    // Calculate duration in milliseconds
    auto duration = duration_cast<milliseconds>(end - start);
    cout << "Sequential execution time: " << duration.count() << " ms" << endl;

    // Start timer
    start = high_resolution_clock::now();

    vector<vector<pair<int, pair<int, int>>>> H2 = NW_Parallel(s0, s1, 1, -1, -2);

    // End timer
    end = high_resolution_clock::now();

    // Calculate duration in milliseconds
    duration = duration_cast<milliseconds>(end - start);
    cout << "Parallel execution time: " << duration.count() << " ms" << endl;

    // printMatrix(H1);
    // std::cout<< " " << std::endl;
    // printMatrix(H2);

    std::cout << "Sequential score: " << H1[H1.size() - 1][H1[0].size() - 1].first << std::endl;

    std::cout << "Parallel score: " << H2[H2.size() - 1][H2[0].size() - 1].first << std::endl;

    
    // std::cout << "Number of hardware threads available: " << threads << std::endl;

    // string s0 = "*TAGC";
    // string s1 = "*TAGTC";

    // vector<vector<pair<int, pair<int, int>>>> H = NW_Parallel(s0,s1,1,-1,-2);

    // printMatrix(H);

    return 0;
}