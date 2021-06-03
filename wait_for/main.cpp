/*
* Copyright (c) 2021 PengfeiCui.
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, version 3.
*
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <iostream>
#include <functional>
#include <fstream>
#include <thread>
#include <queue>
#include <mutex>
#include "wait_for.h"

void call_back(test::DatasPtr& datas, int& status)
{
    static int counter = 0;
    std::cout << counter++ << ": Data is comming status " << status << " ..." << std::endl;
    for (auto it : *datas)
        std::cout << it << " ";
    std::cout << std::endl;
}

int main()
{
    test::Producer pro;
    pro.RegisterDataCallback(call_back);
    pro.Start();
    
    const int poll_time_interval_ms = 500;// 800ms
    while (1)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(poll_time_interval_ms));
        test::DatasPtr datas;
        if (0 == pro.GetData(datas, poll_time_interval_ms))
        {
            std::cout << "Poll ok, data size is " << datas->size() << " ." << std::endl;
        }
        else
        {
            std::cout << "Poll timeout, no datas available." << std::endl;
        }
    }
    return 0;
}
