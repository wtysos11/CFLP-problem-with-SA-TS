#include <iostream>
#include "Solution.h"
#include <ctime>
#include <string>
using namespace std;

double solveUsingSA(string filename)
{
    Solution s(filename);
    vector<Solve> initSet;
    int factoryNum = s.getFactorySize();
    double times;
    times = 500;

    for(int i = 0;i<times;i++)
    {
        initSet.push_back(s.initializeSolution());
    }

    Solve ss = s.SA(initSet);
    s.repair(ss);

    while(!ss.fitness)
    {
        //随机打开一个工厂
        bool first = false;
        for(int i = 0;i<times;i++)
        {
            Solve& solve = initSet[i];
            vector<int> factory;
            for(int j = 0;j<solve.openList.size();j++)
            {
                if(!solve.openList[j])
                {
                    factory.push_back(j);
                }
            }
            if(!first)
            {
                cout<<"factory rest size:"<<factory.size()<<endl;
                first = true;
            }

            int index = rand()%factory.size();
            index = factory[index];
            solve.openList[index] = true;
            solve.restCapacity[index] = s.facCapacity[index];
        }
        ss = s.SA(initSet);
        ss.fitness = true;
        for(int i = 0;i<ss.restCapacity.size();i++)
        {
            if(ss.restCapacity[i]<0)
            {
                ss.fitness = false;
                break;
            }
        }
    }
    s.outputTofile("sa_"+filename,ss);
    return ss.value;
}

void outputSASolve()
{
    ofstream fout("ans_sa_total.txt");
    vector<string> filenameSet;

    for(int i = 1;i<=71;i++)
    {
        filenameSet.push_back("p"+to_string(i));
    }

    for(int i = 0;i<filenameSet.size();i++)
    {
        time_t begining,ending;
        begining = time(NULL);
        double ans = solveUsingSA(filenameSet[i]);
        ending = time(NULL);
        fout<<filenameSet[i]<<"\t"<<ans<<"\t"<<difftime(ending,begining)<<endl;
        cout<<filenameSet[i]<<"\t"<<ans<<"\t"<<difftime(ending,begining)<<endl;

    }
    fout.close();
}

int main(void)
{
    outputSASolve();
    return 0;
}
