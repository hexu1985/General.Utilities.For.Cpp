#pragma once

#include <thread>
#include <functional>
#include <atomic>
#include "pipeline.hpp"

template <typename T>
class SimpleDataSource: public DataSource<T> {
public:
    SimpleDataSource(std::function<bool(T&)> product_func_, Pipe<T> pipe_)
        : done(false), DataSource<T>(pipe_), product_func(product_func_)
    {}

    void start()
    {
        if (worker.joinable()) {
            return;
        }
        worker = std::thread(&SimpleDataSource::worker_thread,this);
    }

    void stop()
    {
        if (!worker.joinable()) {
            return;
        }
        done = true;
        worker.join();
    }

    virtual ~SimpleDataSource()
    {
        stop();
    }

private:
    void worker_thread()
    {
        T value;
        while(!done)
        {
            if (!product_func(value)) {
                break;
            }
            this->put(value);
        }
    }

private:
    std::atomic_bool done;
    std::function<bool(T&)> product_func;
    std::thread worker;
};


template <typename IT, typename OT>
class SimpleDataFilter: public DataFilter<IT,OT> {
public:
    SimpleDataFilter(std::function<OT(IT)> filter_func_, Pipe<IT> in_pipe_, Pipe<OT> out_pipe_)
        : done(false), DataFilter<IT,OT>(in_pipe_, out_pipe_), filter_func(filter_func_)
    {}

    void start()
    {
        if (worker.joinable()) {
            return;
        }
        worker = std::thread(&SimpleDataFilter::worker_thread,this);
    }

    void stop()
    {
        if (!worker.joinable()) {
            return;
        }
        done = true;
        worker.join();
    }

    virtual ~SimpleDataFilter()
    {
        stop();
    }

private:
    void worker_thread()
    {
        IT arg;
        OT res;
        while(!done)
        {
            this->get(arg);
            res = filter_func(std::move(arg));
            this->put(res);
        }
    }

private:
    std::atomic_bool done;
    std::function<OT(IT)> filter_func;
    std::thread worker;
};


template <typename T>
class SimpleDataSink: public DataSink<T> {
public:
    SimpleDataSink(std::function<void(T&)> consume_func_, Pipe<T> pipe_)
        : done(false), DataSink<T>(pipe_), consume_func(consume_func_)
    {}

    void start()
    {
        if (worker.joinable()) {
            return;
        }
        worker = std::thread(&SimpleDataSink::worker_thread,this);
    }

    void stop()
    {
        if (!worker.joinable()) {
            return;
        }
        done = true;
        worker.join();
    }

    virtual ~SimpleDataSink()
    {
        stop();
    }

private:
    void worker_thread()
    {
        T value;
        while(!done)
        {
            this->get(value);
            consume_func(value);
        }
    }

private:
    std::atomic_bool done;
    std::function<void(T&)> consume_func;
    std::thread worker;
};

template <typename T>
class SimplePipeline: public Pipeline {
public:
    SimplePipeline(std::function<bool(T&)> product_func, 
            std::vector<std::function<T(T)>> filter_funcs)
    {
        for (int i = 0; i < filter_funcs.size()+1; i++) {
            queues.push_back(make_pipe<T>());
        }

        auto source = std::shared_ptr<ProcessNode>(
                new SimpleDataSource<T>(product_func, queues[0]));
        this->add_process_node(source);

        for (int i = 0; i < filter_funcs.size(); i++) {
            auto filter = std::shared_ptr<ProcessNode>(
                    new SimpleDataFilter<T,T>(filter_funcs[i], queues[i], queues[i+1]));
           this->add_process_node(filter); 
        }
    }

    void get(T& value)
    {
        auto pipe = queues.back();
        pipe->pop(value);
    }

private:
    std::vector<Pipe<T>> queues;
};
