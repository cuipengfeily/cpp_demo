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

#ifndef WAIT_FOR_H_
#define WAIT_FOR_H_
#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <thread>
#include <mutex>
#include <deque>
#include <condition_variable>


namespace test
{
    template <typename T>
    class SynchronizedQueue;
    using Data = int;
    using Datas = std::vector<Data>;
    using DatasPtr = std::shared_ptr<Datas>;

    class Producer
    {
    public:
        typedef std::function<void(DatasPtr&, int& status)> DatasCallback;

        Producer();

        ~Producer();

        void RegisterDataCallback(DatasCallback cb);

        /*start the udp handler(ThreadLoop) thread*/
        int Start();

        /*stop the udp handler(ThreadLoop) thread**/
        void Stop();

        int GetData(DatasPtr& datas, int timeout_ms);

    protected:

        /*thread function*/
        void ProducerWorker();

        /*thread function: get one group data**/
        void ConsumerWorker();

    private:

        /** \brief store the single data*/
        std::shared_ptr<SynchronizedQueue<Data> > data_;

        /** \brief store the group data*/
        DatasPtr curr_datas_;

        /** \brief store the datas for request*/
        std::deque<DatasPtr> datas_;

        /** \brief Thread: handle the single data and pack to one group*/
        std::shared_ptr<std::thread> consumer_;

        /** \brief Thread: product single data*/
        std::shared_ptr<std::thread> producer_;

        bool need_stop_;

        DatasCallback datas_cb_;

        mutable std::mutex mutex_;
        std::condition_variable cond_;

        /** \brief cache size for group datas.*/
        unsigned int max_datas_count_;

    };

}

#endif //end WAIT_FOR_H_
