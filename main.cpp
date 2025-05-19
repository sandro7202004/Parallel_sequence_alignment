#include <iostream>
#include <utility> 
#include <vector>
#include <cstring> 
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
            std::cout << i << " " << j << std::endl;
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

int main() {

    string s0 = "*TAGC";
    string s1 = "*TAGTC";

    vector<vector<pair<int, pair<int, int>>>> H = NW(s0,s1,1,-1,-2);

    printMatrix(H);

    return 0;
}