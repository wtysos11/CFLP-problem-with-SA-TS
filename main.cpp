#include <iostream>
#include "Solution.h"
#include <ctime>
#include <string>
using namespace std;

//#define OUTPUT 1 //output to file?
//#define DEBUG
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
#ifdef DEBUG
    cout<<"best solution"<<endl;
    ss.print();
#endif
    if(!ss.fitness)//如果解不合法，则尝试修复
    {
        s.repair(ss);
#ifdef DEBUG
        cout<<"after repair"<<endl;
        ss.print();
#endif
    }

    while(!ss.fitness)//如果仍然不符合限制，则继续修复
    {
#ifdef DEBUG
        cout<<"not fit"<<endl;
#endif
        //如果不符合限制，则尝试打开工厂
        //对于每个给定的初始解，随机打开一个关闭的工厂
        for(int i = 0;i<times;i++)
        {
            Solve& solve = initSet[i];
            vector<int> factory;
            for(int j = 0;j<solve.openList.size();j++)//因为如果有多于一次开放工厂的可能，所以这部分要放在循环内
            {
                if(!solve.openList[j])
                {
                    factory.push_back(j);
                }
            }

            int index = rand()%factory.size();
            index = factory[index];
            solve.openList[index] = true;
            solve.restCapacity[index] = s.facCapacity[index];
        }
        ss = s.SA(initSet);
#ifdef DEBUG
        cout<<"\n\n"<<endl;
        ss.print();
        cout<<"\n\n"<<endl;
#endif
    }
    ss.value = s.judgeValue(ss);//因为可能修改，所以进行更新
#ifdef OUTPUT
    s.outputTofile("sa_"+filename,ss);
#endif
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
#ifdef OUTPUT
        fout<<filenameSet[i]<<"\t"<<ans<<"\t"<<difftime(ending,begining)<<endl;
#endif
        cout<<filenameSet[i]<<"\t"<<ans<<"\t"<<difftime(ending,begining)<<endl;

    }
    fout.close();
}

double solveUsingTS(string filename)
{
    Solution s(filename);
    vector<Solve> initSet;
    for(int i = 0;i<100;i++)
    {
        initSet.push_back(s.initializeSolution());
    }
    Solve ss = s.TabuSearch(initSet);
    if(!ss.fitness)
    {
        s.repair(ss);
    }
    ss.value = s.TabuSearchJudge(ss);
#ifdef DEBUG
    cout<<ss.value<<endl;
    ss.print();
#endif
#ifdef OUTPUT
    s.outputTofile("ts_"+filename,ss);
#endif
    return ss.value;
}

void outputTSSolve()
{
    ofstream fout("ans_ts_total.txt",ios::app);
    vector<string> filenameSet;

    for(int i = 1;i<=71;i++)
    {
        filenameSet.push_back("p"+to_string(i));
    }

    for(int i = 0;i<filenameSet.size();i++)
    {
        time_t begining,ending;
        begining = time(NULL);
        double ans = solveUsingTS(filenameSet[i]);
        ending = time(NULL);
#ifdef OUTPUT
        fout<<filenameSet[i]<<"\t"<<ans<<"\t"<<difftime(ending,begining)<<endl;
#endif
        cout<<filenameSet[i]<<"\t"<<ans<<"\t"<<difftime(ending,begining)<<endl;

    }
    fout.close();

}

int main(void)
{
   // outputSASolve();
    outputTSSolve();


    return 0;
}
