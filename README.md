# 用元启发式算法求解有限容量选址问题

中山大学软件工程计算机应用方向2018年算法设计课程的期末大作业

问题综述：现有n个工厂，m个用户，已知工厂容量s_j,开放代价f_j，用户需求d_i，以及分配工厂j到用户i的所有容量花费c_ji（注：每个用户能且只能分配给一家工厂，这是PI-CPLP问题）
参考文献：
1. Hugues Delmaire, Juan A.Diaz, Elena Fernandez, Maruja Ortega, **Comparing New Heuristics for the Pure Integer Capacitated Plant Location Problem** 1999

## 模拟退火算法

第一步：求出开放工厂列表
    在没有满足客户总需要的时候，使用贪婪随机自适应算法找寻最适合的工厂，然后在阈值范围内随机选取
    对于所有的用户，选择需求最大的用户，将其匹配到容量最大的工厂中。
这样子会得到一个比较可行的初始解

第二步：
    使用模拟退火优化用户分配
    邻域产生算法 注：代价函数为原代价函数加上惩罚系数
    惩罚系数可以使用工厂的开放代价*(超出容量/原容量)

    选取j1、j2，j1的客户i1，j2的客户i2
    注：如果当前状态不符合限制，则j1依据超出容量选择，j2依据剩余容量选择；不然，随机选择
    如果i1能够到j2，就到j2
    不然，i1与i2交换

    惩罚系数的计算：
        如果有工厂剩余容量小于0，则该工厂的开放代价/容量*超出容量的绝对值*工厂j对于i的贡献值（i的需求除以分配到j的用户的总需求）

    工厂j的贪婪估价函数为工厂的开放代价除以工厂服务于剩余客户的总分配代价除以客户数目
        其中分配给该工厂的用户按照到该工厂的代价依照顺序排序

## 禁忌搜索算法：
    初始化禁忌列表（一个有限循环队列，存放二元组），列表长度为客户数量的1/4
    估计函数为现有代价加上惩罚系数（如果）

    当没有达到停止要求的时候：
        创建移动的候选集合
        选择最佳的候选者
        更新禁忌列表