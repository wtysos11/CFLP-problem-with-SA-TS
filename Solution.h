
#ifndef SOLUTION_H
#define SOLUTION_H
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <utility>
#include <fstream>

#define MAX 0xFFFFFFFF
using namespace std;
struct Solve
{
    vector<bool> openList;
    vector<int> assignmentList;

    //工厂相关
    vector<int> restCapacity;
    vector<int> useCapacity;
    bool fitness;
    double value;
    Solve(int n,int m)
    {
        openList = vector<bool>(n,0);
        assignmentList = vector<int>(m,-1);
        restCapacity = vector<int>(n,0);
        useCapacity = vector<int>(n,0);
        fitness = true;
    }
    Solve()
    {
        fitness = true;
    }

    //如果工厂j的容量在操作后小于0，返回false，不然返回true
    bool factoryUseCapacity(int j,int value)
    {
        restCapacity[j] -= value;
        useCapacity[j] += value;
        if(restCapacity[j]<0)
        {
            fitness = false;
            return false;
        }

        else
            return true;
    }

    //如果工厂j的容量在操作后小于0，返回false，不然返回true
    bool factoryFreeCapacity(int j,int value)
    {
        restCapacity[j] += value;
        useCapacity[j] -= value;
        if(restCapacity[j]<0)
        {
            fitness = false;
            return false;
        }
        else
            return true;
    }
    void print()
    {
        cout<<"Open List"<<endl;
        for(int i = 0;i<openList.size();i++)
        {
            cout<<openList[i]<<" ";
        }
        cout<<endl;
        cout<<"assignment list"<<endl;
        for(int i = 0;i<assignmentList.size();i++)
        {
            cout<<assignmentList[i]<<" ";
        }
        cout<<endl;
        cout<<"use"<<endl;
        for(int i = 0;i<useCapacity.size();i++)
        {
            cout<<useCapacity[i]<<" ";
        }
        cout<<endl;
        cout<<"rest"<<endl;
        for(int i = 0;i<restCapacity.size();i++)
        {
            cout<<restCapacity[i]<<" ";
        }
        cout<<endl;
    }
};

bool cmp1(const pair<int,int>& a,const pair<int,int>& b)
{
    return a.second<b.second;
}

bool cmp2(const pair<int,int>& a,const pair<int,int>& b)
{
    return a.second>b.second;
}
class Solution
{
private:
    int clientNum,factoryNum;
    vector<int> facCapacity;
    vector<int> facOpenCost;
    vector<int> cliDemand;
    vector<vector<double>> assignmentCost;//外层为工厂号，内层为用户号

    //
    double greedyFunction(int j,Solve& s)
    {
        if(s.openList[j])
        {
            return 0;
        }

        double totalCost = 0;
        int num = 0;
        vector<pair<int,int>> cost;//工厂j到各个用户i的标号与代价
        for(int i = 0;i<clientNum;i++)
        {
            if(s.assignmentList[i]==-1)
            {
                cost.push_back(make_pair(i,assignmentCost[j][i]));
            }
        }

        sort(cost.begin(),cost.end(),cmp1);

        int restCapacity = s.restCapacity[j];
        for(int i = 0;i<cost.size();i++)
        {
            int client = cost[i].first;
            int asCost = cost[i].second;
            if(restCapacity - cliDemand[client]<0)
            {
                break;
            }
            else
            {
                restCapacity -= cliDemand[client];
            }
            num++;
            totalCost += asCost;
        }

        return (totalCost + facOpenCost[j])/num;
    }

    //局部搜索估值函数
    double judgeValue(const Solve& s)
    {
        double totalCost = 0;
        for(int i = 0;i<factoryNum;i++)
        {
            if(s.openList[i])
            {
                totalCost += facOpenCost[i];
            }

            if(s.restCapacity[i]<0)
            {
                totalCost += (double)abs(s.restCapacity[i])/facCapacity[i]*facOpenCost[i];
            }
        }

        for(int i = 0;i<clientNum;i++)
        {
            totalCost += assignmentCost[s.assignmentList[i]][i];
        }
        return totalCost;
    }
    //根据给定解返回其随机邻域中的一个，并进行赋值
    /*
    邻域操作：
        如果解没有不合法的地方：
            随机选取两个工厂
                随机选取第一个工厂的客户i，如果第二个工厂有空位，则丢过去。不然，交换。

        如果解有不合法的地方：
            按照剩余概率加权与超出概率加权选择两个工厂，优先将超出工厂的流量移除，不然则交换。
    */
    Solve generateNeighbourRandom(const Solve& origin)
    {
        cout<<"In neighbour Generator"<<endl;
        Solve s = origin;
        s.print();
        int j1,j2;
        if(s.fitness)
        {
            cout<<"\t neighbour fit"<<endl;
            j1 = rand()%factoryNum,j2 = rand()%factoryNum;
            while(j1==j2 || !s.openList[j1] || !s.openList[j2])
            {
                j1 = rand()%factoryNum;
                j2 = rand()%factoryNum;
            }
        }
        else
        {
            cout<<"\t neighbour unfit"<<endl;
            int totalRest = 0;
            int totalOut = 0;
            vector<pair<int,int>> restFactory;
            vector<pair<int,int>> outFactory;
            cout<<"\t\t"<<"unfit debug. Question2: why always have the same ans."<<endl;
            for(int i = 0;i<factoryNum;i++)
            {
                if(s.restCapacity[i]>0 && s.openList[i])
                {
                    totalRest += s.restCapacity[i];
                    cout<<"\t\t"<<"number:"<<i<<" totalRest:"<<totalRest<<endl;
                    restFactory.push_back(make_pair(i,totalRest));
                }
                else if(s.restCapacity[i]<0 && s.openList[i])
                {
                    totalOut += abs(s.restCapacity[i]);
                    cout<<"\t\t"<<"number:"<<i<<" totalOut:"<<totalOut<<endl;
                    outFactory.push_back(make_pair(i,totalOut));
                }
            }

            //
            int index1 = rand()%totalOut;
            cout<<"\t\t"<<"index1:"<<index1<<endl;
            int index2 = rand()%totalRest;
            cout<<"\t\t"<<"index1:"<<index2<<endl;
            j1 = outFactory[outFactory.size()-1].first;
            j2 = restFactory[restFactory.size()-1].first;
            int j1Judge = totalOut,j2Judge = totalRest;
            for(int i = 0;i<outFactory.size();i++)
            {
                if(outFactory[i].second>index1 && outFactory[i].second<j1Judge)
                {
                    j1Judge = outFactory[i].second;
                    j1 = outFactory[i].first;
                }
            }
            cout<<"\t\t"<<"Last j1:"<<j1<<endl;
            cout<<"\t\t"<<"Last j1:"<<j2<<endl;
            for(int i = 0;i<restFactory.size();i++)
            {
                if(restFactory[i].second>index2 && restFactory[i].second<j2Judge)
                {
                    j2Judge = restFactory[i].second;
                    j2 = restFactory[i].first;
                }
            }
        }

        vector<int> clientI,clientJ;
        for(int i = 0 ;i<clientNum;i++)
        {
            if(s.assignmentList[i] == j1)
            {
                clientI.push_back(i);
            }
            else if(s.assignmentList[i] == j2)
            {
                clientJ.push_back(i);
            }
        }

        if(clientI.size()==0)
        {
            return generateNeighbourRandom(origin);
        }
        int index1 = rand()%clientI.size();

        int i1 = clientI[index1];
        if(cliDemand[i1] < s.restCapacity[j2])
        {
            cout<<"\tsituation 1"<<endl;
            cout<<"\tclient "<<i1<<" from "<<j1<<" to "<<j2<<endl;
            s.factoryFreeCapacity(j1,cliDemand[i1]);
            s.factoryUseCapacity(j2,cliDemand[i1]);
            s.assignmentList[i1] = j2;
        }
        else
        {
            int index2 = rand()%clientJ.size();
            int i2 = clientJ[index2];
            cout<<"\tsituation 2"<<endl;
            cout<<"\tinterchange"<<i1<<" and "<<i2<<" from "<<j1<<" and "<<j2<<endl;
            s.factoryFreeCapacity(j1,cliDemand[i1]);
            s.factoryUseCapacity(j2,cliDemand[i1]);
            s.factoryFreeCapacity(j2,cliDemand[i2]);
            s.factoryUseCapacity(j1,cliDemand[i2]);
            s.assignmentList[i1] = j2;
            s.assignmentList[i2] = j1;
        }
        for(int i = 0;i<factoryNum;i++)
        {
            if(s.restCapacity[i]<0)
            {
                s.fitness = false;
                return s;
            }
        }
        s.fitness = true;
        return s;
    }
public:
    //使用文件内容初始化
    Solution(string filename)
    {
        srand((unsigned)time(NULL));
        ifstream fin(filename);
        fin>>factoryNum>>clientNum;
        facCapacity = vector<int>(factoryNum,0);
        facOpenCost = vector<int>(factoryNum,0);
        cliDemand = vector<int>(clientNum,0);
        for(int i = 0;i<factoryNum;i++)
        {
            fin>>facCapacity[i]>>facOpenCost[i];
        }
        for(int i = 0;i<clientNum;i++)
        {
            double x;
            fin>>x;
            cliDemand[i] = x;
        }
        for(int i = 0;i<factoryNum;i++)
        {
            vector<double> cost(clientNum,0);
            for(int j = 0;j<clientNum;j++)
            {
                fin>>cost[j];
            }
            assignmentCost.push_back(cost);
        }
    }

    //使用贪婪随机自适应算法产生初始解并返回
    Solve initializeSolution()
    {
        double alpha = 0.4;
        Solve s(factoryNum,clientNum);
        for(int j = 0;j<factoryNum;j++)
        {
            s.restCapacity[j] = facCapacity[j];
        }
        int totalDemand = 0;//尚未满足的用户需求
        for(int i = 0;i<clientNum;i++)
        {
            totalDemand += cliDemand[i];
        }
        int satisFiedDemand = 0;
        //对于所有的工厂，检查其是否能够满足用户需求
        for(int j = 0;j<factoryNum;j++)
        {
            vector<double> greedyCost(factoryNum,0);
            double greedyMin = MAX;
            for(int i = 0;i<factoryNum;i++)
            {
                if(s.openList[i])
                {
                    greedyCost[i] = MAX;
                }
                else
                {
                    greedyCost[i] = greedyFunction(i,s);
                    if(greedyCost[i]<greedyMin)
                    {
                        greedyMin = greedyCost[i];
                    }
                }
            }
            //产生候选解集
            vector<int> RCL;
            double threshold = (1+alpha)*greedyMin;
            for(int i = 0;i<greedyCost.size();i++)
            {
                if(greedyCost[i]<threshold)
                {
                    RCL.push_back(i);
                }
            }
            int index = rand()%RCL.size();
            int openFactory = RCL[index];
            s.openList[openFactory] = true;

            //求出需要赋值的客户列表
            vector<pair<int,int>> cost;//工厂j到各个用户i的标号与代价
            for(int i = 0;i<clientNum;i++)
            {
                if(s.assignmentList[i]==-1)
                {
                    cost.push_back(make_pair(i,assignmentCost[openFactory][i]));
                }
            }

            sort(cost.begin(),cost.end(),[](const pair<int,int>& a,const pair<int,int>& b)
            {
                return a.second<b.second;
            });
            //进行赋值，修改解
            for(int i = 0;i<cost.size();i++)
            {
                int client = cost[i].first;
                int asCost = cost[i].second;
                if(s.restCapacity[openFactory] - cliDemand[client]<0)
                {
                    break;
                }

                s.factoryUseCapacity(openFactory,cliDemand[client]);
                s.assignmentList[client] = openFactory;
            }
            //检查是否满足需求
            satisFiedDemand += s.useCapacity[openFactory];
            if(satisFiedDemand>=totalDemand)
            {
                break;
            }

        }
        //对于没有赋值的客户进行赋值
        vector<pair<int,int>> restClient;
        vector<pair<int,int>> restFactory;
        for(int i = 0;i<clientNum;i++)
        {
            if(s.assignmentList[i]==-1)
            {
                restClient.push_back(make_pair(i,cliDemand[i]));
            }
        }
        for(int j = 0;j<clientNum;j++)
        {
            if(!s.openList[j])
            {
                restFactory.push_back(make_pair(j,facCapacity[j]));
            }
        }
        sort(restClient.begin(),restClient.end(),cmp1);
        sort(restFactory.begin(),restFactory.end(),cmp1);
        while(restClient.size()>0)
        {
            int client = restClient[restClient.size()-1].first;
            int factory = restFactory[restFactory.size()-1].first;
            s.assignmentList[client] = factory;
            s.factoryUseCapacity(factory,cliDemand[client]);

            restClient.pop_back();
            restFactory.pop_back();
            sort(restClient.begin(),restClient.end(),cmp1);
            sort(restFactory.begin(),restFactory.end(),cmp1);
        }
        s.value = judgeValue(s);
        return s;
    }

    //根据产生的初始解进行模拟退火并返回
    Solve SA(vector<Solve> initSet)
    {
        Solve best;
        double bestAns = MAX;//保存最小的解
        for(int i = 0;i<initSet.size();i++)
        {
            Solve s = initSet[i];
            s.value = judgeValue(s);
            double temperature = 400;
            double endTemperature = 1;
            double ratio = 0.97;
            while(temperature>endTemperature)
            {
                cout<<"\n\ntempearture\t"<<temperature<<endl;
                //随机产生邻域
                //如果是好解，则无条件接纳
                    //保存全局最优解
                //如果是差解，则有选择接纳
                Solve ss = generateNeighbourRandom(s);
                ss.value = judgeValue(ss);
                if(ss.value<s.value)
                {
                    s = ss;
                    if(s.value<bestAns && s.fitness)
                    {
                        bestAns = s.value;
                        best = s;
                    }
                    cout<<"good solve"<<endl;
                }
                else
                {
                    double p = exp((s.value-ss.value)/temperature);
                    double pointer = (double)1.0*rand()/RAND_MAX;
                    if(p>pointer)
                    {
                        s = ss;
                        cout<<"bad solve,but ac"<<endl;
                    }
                }

                temperature *= ratio;
            }
        }
        return best;
    }


    //根据产生的初始解进行禁忌搜索并返回
};

#endif // SOLUTION_H