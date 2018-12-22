
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
#include <cmath>
//for Tabu Search
/*
用来记录禁忌搜索的操作（两个工厂之间交换顾客，或者一个工厂给予另外一个工厂顾客）
并记录这样变动后的大小
*/
struct Oper
{
    int j1,j2;
    int i1,i2;
    int weight;
};

#define MAX 0xFFFFFFFF
using namespace std;
struct Solve
{
    vector<bool> openList; //开放工厂列表，一系列01串，用来表示工厂是否开着
    vector<int> assignmentList; // 顾客的分配列表，表示顾客分配给了哪个工厂

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
    Solve(const Solve& other)
    {
        openList = other.openList;
        assignmentList = other.assignmentList;
        restCapacity = other.restCapacity;
        useCapacity = other.useCapacity;
        fitness = other.fitness;
        value = other.value;
    }



    //如果工厂j的容量在操作后小于0，返回false，不然返回true
    bool factoryUseCapacity(int j,int value)
    {
        restCapacity[j] -= value;
        useCapacity[j] += value;
        if(restCapacity[j]<0)
        {
            fitness = false;
        }
        return fitness;
    }

    //如果工厂j的容量在操作后小于0，返回false，不然返回true
    /*
    如果某个工厂过去流量为负，突然见为正了，则可能从一个不合法解变为合法解
    */
    bool factoryFreeCapacity(int j,int value)
    {
        bool change = false;
        if(restCapacity[j]<0 && restCapacity[j] + value>=0)
            change = true;
        
        restCapacity[j] += value;
        useCapacity[j] -= value;

        if(change)
        {
            fitness = true;
            for(int i = 0 ;i<restCapacity.size();i++)
            {
                if(restCapacity[i]<0)
                {
                    fitness = false;
                    break;
                }
            }
        }
        return fitness;
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
public:
    int clientNum,factoryNum; // 客户数目 与 工厂数目
    vector<int> facCapacity; //各个工厂的容量
    vector<int> facOpenCost; //各个工厂的花费
    vector<int> cliDemand; //用户的需求
    vector<vector<double>> assignmentCost;//外层为工厂号，内层为用户号

    //贪婪估计函数，用来为贪婪自适应随机算法提供参考数据
    /*
        通过对没有分配且可以分配到该工厂（尽量填满）的顾客进行按照代价升序排序，以尽量小的代价获得更多的流量。
    */
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

        sort(cost.begin(),cost.end(),cmp1);//升序排列代价

        int restCapacity = s.restCapacity[j];
        for(int i = 0;i<cost.size();i++)
        {
            int client = cost[i].first;
            int asCost = cost[i].second;
            if(restCapacity - cliDemand[client]<0)//到达底线，取消
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
    double judgeValue(const Solve& s,bool switcher = true)
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
                if(switcher)
                    totalCost += (double)abs(s.restCapacity[i])/facCapacity[i]*facOpenCost[i];
                else
                    totalCost += MAX;
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
        Solve s = origin;
        int j1,j2;
        if(s.fitness)//如果该解是合法解
        {
            j1 = rand()%factoryNum,j2 = rand()%factoryNum;
            while(j1==j2 || !s.openList[j1] || !s.openList[j2])
            {
                j1 = rand()%factoryNum;
                j2 = rand()%factoryNum;
            }
        }
        else
        {
            //如果存在不合法解，则将不合法的工厂的超出流量进行加权后随机选择，合法工厂的剩余流量加权后随机选择，这样子可以更可能选出尽量剩余多的工厂来接受流量。
            int totalRest = 0;
            int totalOut = 0;
            vector<pair<int,int>> restFactory;
            vector<pair<int,int>> outFactory;
            for(int i = 0;i<factoryNum;i++)
            {
                if(s.restCapacity[i]>0 && s.openList[i])
                {
                    totalRest += s.restCapacity[i];
                    restFactory.push_back(make_pair(i,totalRest));
                }
                else if(s.restCapacity[i]<0 && s.openList[i])
                {
                    totalOut += abs(s.restCapacity[i]);
                    outFactory.push_back(make_pair(i,totalOut));
                }
            }

            if(totalOut == 0)//可能出现错漏
            {
                s.fitness = true;
                return generateNeighbourRandom(s);
            }
            //找到大于指针的最小值
            int index1 = rand()%totalOut;
            int index2 = rand()%totalRest;
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
            for(int i = 0;i<restFactory.size();i++)
            {
                if(restFactory[i].second>index2 && restFactory[i].second<j2Judge)
                {
                    j2Judge = restFactory[i].second;
                    j2 = restFactory[i].first;
                }
            }
        }
        //寻找要交换的顾客
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

        if(clientI.size()==0)//有可能出现工厂没有分配人的情况，这时候随机开始下一次
        {
            return generateNeighbourRandom(origin);
        }
        //随机选择一个顾客，进行移动或者交换
        int index1 = rand()%clientI.size();

        int i1 = clientI[index1];
        if(cliDemand[i1] < s.restCapacity[j2])//如果工厂j2能够接受i1，则移动
        {
            s.factoryFreeCapacity(j1,cliDemand[i1]);
            s.factoryUseCapacity(j2,cliDemand[i1]);
            s.assignmentList[i1] = j2;
        }
        else//不然，则进行交换
        {
            int index2 = rand()%clientJ.size();
            int i2 = clientJ[index2];
            s.factoryFreeCapacity(j1,cliDemand[i1]);
            s.factoryUseCapacity(j2,cliDemand[i1]);
            s.factoryFreeCapacity(j2,cliDemand[i2]);
            s.factoryUseCapacity(j1,cliDemand[i2]);
            s.assignmentList[i1] = j2;
            s.assignmentList[i2] = j1;
        }
        s.value = judgeValue(s);

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
        //对于每个用户建立一个排序表，按照各个工厂的费用顺序排序

        fin.close();
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
            //数组greedyCost保存所有工厂的贪婪判断值
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

            //求出需要赋值的客户列表（对所有用户到该工厂的代价升序排序，从最小取起）
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
                if(s.restCapacity[openFactory] - cliDemand[client]<0)//如果剩余空间不足，则退出
                {
                    break;
                }

                s.factoryUseCapacity(openFactory,cliDemand[client]);//加入顾客
                s.assignmentList[client] = openFactory;
            }
            //检查是否满足需求
            satisFiedDemand += s.useCapacity[openFactory];
            if(satisFiedDemand>=totalDemand)
            {
                break;
            }

        }
        //对于没有赋值的客户进行赋值。取没有赋值的客户按需求从大到小排列，有剩余容量的工厂从大到小排列，以此装入
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
        //对于剩余客户进行分配，每次分配后，重新排序
        while(restClient.size()>0)
        {
            int client = restClient[restClient.size()-1].first;
            int factory = restFactory[restFactory.size()-1].first;
            s.assignmentList[client] = factory;
            s.factoryUseCapacity(factory,cliDemand[client]);

            restClient.pop_back();
            restFactory[restFactory.size()-1].second -= cliDemand[client];

            sort(restClient.begin(),restClient.end(),cmp1);
            sort(restFactory.begin(),restFactory.end(),cmp1);
        }
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
            if(s.value<bestAns)
            {
                bestAns = s.value;
                best = s;
            }

            double temperature = 400;
            double endTemperature = 1;
            double ratio = 0.97;
            int endCounting = 0;

            while(temperature>endTemperature)
            {
                //随机产生邻域
                //如果是好解，则无条件接纳
                    //保存全局最优解
                //如果是差解，则有选择接纳
                Solve ss = generateNeighbourRandom(s);
                if(ss.value<s.value)
                {
                    s = ss;
                    if(s.value<bestAns)//最大值更新
                    {
                        bestAns = s.value;
                        best = s;
                    }
                    endCounting = 0;
                }
                else
                {
                    endCounting++;
                    double p = exp((s.value-ss.value)/temperature);
                    double pointer = (double)1.0*rand()/RAND_MAX;
                    if(p>pointer)
                    {
                        s = ss;
                    }
                }
                if(endCounting > 100)
                {
                    break;
                }

                temperature *= ratio;
            }
        }
        return best;
    }

    void repair(Solve& ss)
    {
        //假设有一个工厂因为某些原因引入了一个客户导致出现负数的情况
        int factory = 0;
        for(int i = 0;i<ss.restCapacity.size();i++)
        {
            if(ss.restCapacity[i]<0)
            {
                factory = ss.restCapacity[i];
            }
        }
        vector<int> customers;
        for(int i = 0;i<ss.assignmentList.size();i++)
        {
            if(ss.assignmentList[i]==factory)
            {
                customers.push_back(i);
            }
        }
        //对于每一个用户，检查移走它们的费用与可能性
        for(int i = 0;i<customers.size();i++)
        {
            int c = customers[i];
            int minCost = MAX;
            int orderFactory = -1;
            for(int j = 0;j<ss.openList.size();j++)
            {
                if(ss.openList[j])//如果开放，能放得下，还够小
                {
                    if(ss.restCapacity[j]>=cliDemand[c] && assignmentCost[j][c]<minCost)
                    {
                        minCost = assignmentCost[j][c];
                        orderFactory = j;
                    }
                }
            }
            //如果有工厂能够接纳，则进行移动
            if(orderFactory != -1)
            {
                ss.assignmentList[c] = orderFactory;
                ss.factoryFreeCapacity(factory,cliDemand[c]);
                ss.factoryUseCapacity(orderFactory,cliDemand[c]);
                ss.fitness = true;
                for(int j = 0 ;j < ss.restCapacity.size();j++)
                {
                    if(ss.restCapacity[j]<0)
                    {
                        ss.fitness = false;
                        break;
                    }
                }
                if(ss.fitness)
                    break;
            }

        }
    }

    int getFactorySize()
    {
        return factoryNum;
    }
    void outputTofile(string filename,Solve s)
    {
        string newFilename = "ans_"+filename+".txt";
        ofstream fout(newFilename);
        fout<<s.value<<endl;
        for(int i = 0;i<s.openList.size();i++)
        {
            fout<<s.openList[i]<<" ";
        }
        fout<<endl;
        for(int i = 0;i<s.assignmentList.size();i++)
        {
            fout<<s.assignmentList[i]<<" ";
        }
        fout<<endl;
        fout.close();
    }

    void clear()
    {
        facCapacity.clear();
        facOpenCost.clear();
        cliDemand.clear();
        assignmentCost.clear();
    }

/*
    禁忌搜索用的判断函数（主要是之前的判断函数似乎效果不好，试用一下其他的）
*/
   int TabuSearchJudge(const Solve& s,bool debug = false)
   {
        int penalty = 100;//惩罚系数，每超出一单位容量惩罚50
        int totalCost = 0;
        int penaltyc = 0;
        for(int i = 0;i<factoryNum;i++)
        {
            if(s.openList[i])
            {
                totalCost += facOpenCost[i];
            }

            if(s.restCapacity[i]<0)
            {
                totalCost += penalty*abs(s.restCapacity[i]);
            }
        }

        for(int i = 0;i<clientNum;i++)
        {
            totalCost += assignmentCost[s.assignmentList[i]][i];
        }
        return totalCost;
   }


    //根据产生的初始解进行禁忌搜索并返回
    //用一个循环列表来作为禁忌搜索的表格

    /*
    禁忌搜索具体步骤：
        在邻域中找到一个合适的且最好的（符合禁忌列表要求）
        如果能够找到一个比最优解更好的解，可以特赦

        中期与长期暂时不做
        在不考虑到工厂开放费用的情况下，一个客户从工厂A到工厂B，得益于其差值。因为A是不可选的，所以B选择越小的，越能得到更优解。
        如果放不下客户A，可以考虑进行交换。
        上述两种情况，都应该优先从小的考虑。
    */

    Solve TabuSearch(vector<Solve> initSet)
    {
        Solve best;
        int bestValue = MAX;
        srand((unsigned)time(NULL));
        vector<pair<int,int>> tabuList(clientNum/4,make_pair(-1,-1));//禁忌列表表示在这段范围内，客户i不入工厂j
        int tabuPoint = 0;
        int tabuEnd = 0;
        //initSet为外层构建好的初始解，大部分可行（不排除不可行的情况）
        /*
        当没有达到停止要求的时候
            创建移动的候选集合，运用局部搜索的思想，各个工厂间均运用贪心的思想（即用户会优先选择代价更小的工厂），进行搜索
        如果100次搜索没有结果，会尝试对最好解进行加深操作
        如果1000次搜索没有结果，会尝试扩大搜索范围

        */
        for(int i = 0 ;i<initSet.size();i++)
        {
            cout<<i<<endl;
            Solve s = initSet[i];
            s.value = TabuSearchJudge(s);
            if(bestValue<s.value)
            {
                best = s;
                bestValue = s.value;
            }
            int times = 0;
            //允许不相容情况的出现
            /*如果有500次没有更新，则退出*/

            while(times<200)
            {
                //500次，从开放列表中进行遍历
                vector<Oper> oper;
                for(int j1 = 0;j1<s.openList.size();j1++)
                {
                    if(!s.openList[j1])
                    {
                        continue;
                    }

                    for(int j2 = 0;j2<s.openList.size();j2++)
                    {
                        if(!s.openList[j2] || j1==j2)
                            continue;

                        //尝试从j1到j2进行移动或者交换操作
                        for(int i1 = 0;i1<s.assignmentList.size();i1++)
                        {
                            if(s.assignmentList[i1]!=j1)
                                continue;

                            if(s.restCapacity[j2]>cliDemand[i1])
                            {
                                oper.push_back(Oper{j1,j2,i1,-1,assignmentCost[j1][i1] - assignmentCost[j2][i1]});
                            }
                            else if(j1<j2)
                            {
                                //找寻一个可以交换的对象
                                for(int i2 = 0;i2<s.assignmentList.size();i2++)
                                {
                                    if(s.assignmentList[i2]!=j2)
                                        continue;

                                    if(s.restCapacity[j2] - cliDemand[i1] + cliDemand[i2]>0 && s.restCapacity[j1] - cliDemand[i2] + cliDemand[i1]>0)
                                    {
                                        oper.push_back(Oper{j1,j2,i1,-1,assignmentCost[j1][i1] + assignmentCost[j2][i2] - assignmentCost[j2][i1] - assignmentCost[j1][i2]});
                                    }
                                }
                            }
                        }
                    }
                }
                sort(oper.begin(),oper.end(),[](const Oper &a, const Oper& b){
                    return a.weight<b.weight;
                });
                //找到禁忌表中允许的最小值，或者打破禁忌的特赦值
                for(int i = 0;i<oper.size();i++)
                {
                    int j1 = oper[i].j1,j2 = oper[i].j2;
                    int i1 = oper[i].i1,i2 = oper[i].i2;
                    bool pass = true;
                    for(int j = tabuPoint;j<tabuEnd;j++)
                    {
                        if(j == tabuList.size())
                        {
                            j = 0;
                        }

                        if(tabuList[j].first == i1 && tabuList[j].second == j1 || tabuList[j].first==i2 && tabuList[j].second == j2)
                        {
                            pass = false;
                            break;
                        }
                    }
                    if(!pass && s.value + oper[i].weight > bestValue)//判断是否存在特赦
                    {
                        continue;
                    }
                    else//找到最小值
                    {
                        if(i2==-1)//单向移动
                        {
                            s.assignmentList[i1] = j2;
                            s.factoryFreeCapacity(j1,cliDemand[i1]);
                            s.factoryUseCapacity(j2,cliDemand[i1]);
                            tabuList[tabuEnd] = make_pair(i1,j2);
                            tabuEnd++;
                            if(tabuEnd == tabuList.size())
                                tabuEnd = 0;
                            if(tabuPoint == tabuEnd)
                            {
                                tabuPoint++;
                                if(tabuPoint == tabuList.size())
                                    tabuPoint = 0;
                            }
                        }
                        else
                        {
                            s.assignmentList[i1] = j2;
                            s.assignmentList[i2] = j1;
                            s.factoryFreeCapacity(j1,cliDemand[i1]);
                            s.factoryUseCapacity(j2,cliDemand[i1]);
                            s.factoryFreeCapacity(j2,cliDemand[i2]);
                            s.factoryUseCapacity(j1,cliDemand[i2]);

                            tabuList[tabuEnd] = make_pair(i1,j2);
                            tabuEnd++;
                            if(tabuEnd == tabuList.size())
                                tabuEnd = 0;
                            if(tabuPoint == tabuEnd)
                            {
                                tabuPoint++;
                                if(tabuPoint == tabuList.size())
                                    tabuPoint = 0;
                            }

                            tabuList[tabuEnd] = make_pair(i2,j1);
                            tabuEnd++;
                            if(tabuEnd == tabuList.size())
                                tabuEnd = 0;
                            if(tabuPoint == tabuEnd)
                            {
                                tabuPoint++;
                                if(tabuPoint == tabuList.size())
                                    tabuPoint = 0;
                            }
                        }
                        s.value = TabuSearchJudge(s);
                        if(s.value<bestValue)
                        {
                            times = 0;
                            bestValue = s.value;
                            best = s;
                        }
                        break;
                    }
                }
                times++;
            }
        }


        return best;
    }

};

#endif // SOLUTION_H
