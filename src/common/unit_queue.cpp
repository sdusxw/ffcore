/*********************************************************************************
  *Copyright(C):        sdboon.com
  *FileName:            unit_queue.cpp
  *Author:              diaoguangqiang
  *Version:             2.0
  *Date:                2017.09.15
  *Description:         单元测试：线程安全队列
  *History:             (修改历史记录列表，每条修改记录应包含修改日期、修改者及修改内容简介)
     1. Date:           2017.09.15
        Author:         diaoguagnqiang
        Modification:   首次生成文件
     2. Data:
        Author:
        Modification:
**********************************************************************************/

#include "safeguard_queue.h"
#include <stdio.h>

using namespace BASE;

void test_queue();

int main(int argc, char** argv){
    test_queue();

    return 0;
}

void test_queue(){

    safeguard_queue<std::string > jpg_queue_;
    safeguard_queue<std::pair<std::string, std::string> > jpg2_queue_;

    //jpg_queue_.push("adb");

    //cv::Mat m;

    //jpg2_queue_.push(std::make_pair("ssad", "ad"));

    //jpg2_queue_.get_size();
    //jpg_queue_.get_size();
    jpg_queue_.clear();
    jpg_queue_.push("adf");
    jpg_queue_.empty();
    jpg_queue_.get_size();

    printf("size:%d\n", jpg_queue_.get_size());

    return;
}