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
    if(!ss.fitness)//如果解不合法，则尝试修复
        s.repair(ss);
    while(!ss.fitness)//如果仍然不符合限制，则继续修复
    {
        cout<<"not fit"<<endl;
        //如果不符合限制，则尝试打开工厂
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
    ss.value = s.judgeValue(ss);//因为可能修改，所以进行更新
    s.outputTofile("sa_"+filename,ss);
    return ss.value;
}

void outputSASolve()
{
    ofstream fout("ans_sa_total.txt",ios::app);
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

void outputTSSolve()
{
    string s1 = "p1";
    Solution s(s1);
    vector<Solve> initSet;
    for(int i = 0;i<100;i++)
    {
        initSet.push_back(s.initializeSolution());
    }
    Solve ss = s.TabuSearch(initSet);
    cout<<ss.value<<endl;
    cout<<s.TabuSearchJudge(ss,true)<<endl;
    ss.print();
}

int main(void)
{
    outputSASolve();

    return 0;
}
