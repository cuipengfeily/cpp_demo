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

namespace test
{
    template <typename T>
    class SynchronizedQueue/*store the single data*/
    {
    public:
        SynchronizedQueue() :
            queue_(),
            mutex_(),
            cond_(),
            request_to_end_(false),
            enqueue_data_(true)
        {
        }

        bool enqueue(const T& data)
        {
            std::unique_lock<std::mutex> lock(mutex_);

            if (enqueue_data_)
            {
                queue_.push(data);
                cond_.notify_one();
                return true;
            }
            else
            {
                return false;
            }
        }

        bool dequeue(T& result)
        {
            std::unique_lock<std::mutex> lock(mutex_);

            while (queue_.empty() && (!request_to_end_))
            {
                cond_.wait(lock);
            }

            if (request_to_end_)
            {
                doEndActions();
                return false;
            }

            result = queue_.front();
            queue_.pop();

            return true;
        }

        void stopQueue()
        {
            std::unique_lock<std::mutex> lock(mutex_);
            request_to_end_ = true;
            cond_.notify_one();
        }

        unsigned int size()
        {
            std::unique_lock<std::mutex> lock(mutex_);
            return static_cast<unsigned int>(queue_.size());
        }

        bool isEmpty() const
        {
            std::unique_lock<std::mutex> lock(mutex_);
            return (queue_.empty());
        }

    private:
        void doEndActions()
        {
            enqueue_data_ = false;

            while (!queue_.empty())
            {
                queue_.pop();
            }
        }

        std::queue<T> queue_;            //udp packet queue
        mutable std::mutex mutex_;     //data access 
        std::condition_variable cond_; // The condition to wait for

        bool request_to_end_;
        bool enqueue_data_;
    };

    Producer::Producer():
        curr_datas_(new Datas()),
        need_stop_(false),
        datas_cb_(nullptr),
        mutex_(),
        cond_(),
        max_datas_count_(200)
    {
    }

    Producer::~Producer()
    {
        Stop();
    }

    void Producer::ProducerWorker()
    {
        int data = 0;
        while (!need_stop_)
        {
            this->data_->enqueue(data++);
            std::this_thread::sleep_for(std::chrono::milliseconds(110));
        }
    }
    
    void Producer::ConsumerWorker()
    {
        Data data = 0;
        while (this->data_->dequeue(data))
        {
            if(!this->curr_datas_)
                this->curr_datas_.reset(new Datas);
            this->curr_datas_->push_back(data);
            if (this->curr_datas_->size() > 10)
            {
                // data callback
                int st = 0;
                if (this->datas_cb_)
                    this->datas_cb_(this->curr_datas_, st);

                // handle catch buffer
                std::unique_lock<std::mutex> lock(this->mutex_);
                {
                    if (this->max_datas_count_ > 0)
                    {
                        while (this->datas_.size() >= this->max_datas_count_)
                        {
                            this->datas_.pop_front();
                        }
                    }
                    this->datas_.push_back(this->curr_datas_);
                }

                //Notify a new datas available.
                cond_.notify_one();

                //Allocate a new data for next one.
                this->curr_datas_.reset(new Datas);
            }
        }
    }

    void Producer::RegisterDataCallback(DatasCallback cb)
    {
        this->datas_cb_ = cb;
    }

    int Producer::GetData(DatasPtr& datas, int timeout_ms)
    {
        {
            std::unique_lock<std::mutex> lock(mutex_);

            if (this->datas_.empty())
            {
                //wait_for bug on vs2015&vs2017: https://developercommunity.visualstudio.com/content/problem/438027/unexpected-behaviour-with-stdcondition-variablewai.html
                if (std::cv_status::timeout == cond_.wait_for(lock, std::chrono::milliseconds(timeout_ms)))
                {
                    return -1;
                }
                else
                {
                    if (this->datas_.empty())
                    {
                        return -1;
                    }
                }
            }
        }
        std::unique_lock<std::mutex> lock(mutex_);
        {
            datas = this->datas_.front();
            this->datas_.pop_front();
        }
        return 0;
    }

    int  Producer::Start()
    {
        if (!this->data_)
        {
            this->data_.reset(new SynchronizedQueue<Data>);
        }

        if (!this->consumer_)
        {
            this->consumer_ = std::shared_ptr<std::thread>(
                new std::thread(std::bind(&Producer::ConsumerWorker, this)));
        }

        if (!this->producer_)
        {
            this->producer_ = std::shared_ptr<std::thread>(
                new std::thread(std::bind(&Producer::ProducerWorker, this)));
        }
        return 0;
    }

    void Producer::Stop()/*stop the thread*/
    {
        this->need_stop_ = true;

        if (this->data_)
        {
            this->data_->stopQueue();
        }

        if (this->consumer_)
        {
            this->consumer_->join();
            this->consumer_.reset();
        }

        if (this->producer_)
        {
            this->producer_->join();
            this->producer_.reset();
        }

    }


}
