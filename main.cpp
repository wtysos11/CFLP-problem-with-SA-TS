#include <iostream>
#include "Solution.h"
using namespace std;

int main(void)
{
    Solution s("p1");
    vector<Solve> initSet;
    for(int i = 0;i<1;i++)
    {
        initSet.push_back(s.initializeSolution());
    }

    Solve ss = s.SA(initSet);
    ss.print();
    cout<<ss.fitness<<endl;
    return 0;
}
