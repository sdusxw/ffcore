/*********************************************************************************
  *Copyright(C):        sdboon.com
  *FileName:            unit_multimap.cpp
  *Author:              diaoguangqiang
  *Version:             2.0
  *Date:                2017.09.15
  *Description:         单元测试：订阅与发布测试
  *History:             (修改历史记录列表，每条修改记录应包含修改日期、修改者及修改内容简介)
     1. Date:           2017.09.15
        Author:         diaoguagnqiang
        Modification:   首次生成文件
     2. Data:
        Author:
        Modification:
**********************************************************************************/

#include <stdio.h>
#include <map>
#include <iostream>

using namespace std;

multimap<int,int,greater<int> > mump;  //键值从大到小 less<Type> 键值从小到大
typedef multimap<int,int>::iterator Iterator;

int main(int argc, char** argv)
{
    mump.insert(make_pair(1,2));
    mump.insert(make_pair(1,4));
    mump.insert(make_pair(1,5));
    mump.insert(make_pair(1,6));
    mump.insert(make_pair(3,4));
    mump.insert(make_pair(3,5));
    mump.insert(make_pair(3,4));

    //遍历整个multimap

    cout << "---------------visit all multimap------------ " << endl;
    Iterator it;
    it = mump.begin();

    for(it = mump.begin();it != mump.end();++it) {

        if(it->second == 5){
            cout << it->first << ",  " << it->second << endl;

            mump.erase(it);
        }
    }

    cout << "---------------end------------ " << endl;

    it = mump.begin();

    for(it = mump.begin();it != mump.end();++it) {

        cout << it->first << ",  " << it->second << endl;

    }

    return 0;
}