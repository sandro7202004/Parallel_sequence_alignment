#include <iostream>
#include <utility> 
#include <vector>
#include <cstring> 
#include <thread>
using namespace std; 

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

vector<vector<pair<int, pair<int, int>>>> NW_Parallel(string s0, string s1, int ma, int mi, int g)
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

    std::vector<std::thread> threads(min(n-1,m-1));

    int i = 1;
    int j = 1;

    while (i != n && j!= m)
    {
        std::cout << "Loop" << std::endl;
        std::cout << i << " " << j << std::endl;
        int t = 0;
        int c_i = i;
        int c_j = j;

        std::cout << "Inner Loop" << std::endl;

        while (c_i != 0 && c_j != m)
        {
            
            std::cout << c_i << " " << c_j << std::endl;
            threads[t] = std::thread(NW_Parallel_AUX,c_i,c_j,s0,s1,ma,mi,g,std::ref(H)); 
            c_i -=1;
            c_j +=1;
            t+=1;
        }

        for (int h=0; h<t; h++) 
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

    string s0 = "*TAGC";
    string s1 = "*TAGTC";

    vector<vector<pair<int, pair<int, int>>>> H = NW_Parallel(s0,s1,1,-1,-2);

    printMatrix(H);

    return 0;
}