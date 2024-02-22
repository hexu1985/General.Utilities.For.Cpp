#pragma once

#include <thread>
#include <functional>
#include <atomic>
#include <vector>
#include <cassert>
#include <boost/any.hpp>
#include "pipeline.hpp"

template <typename T>
class SimpleDataSource: public DataSource<T> {
public:
    SimpleDataSource(std::function<bool(T&)> product_func_, Pipe<T> pipe_)
        : DataSource<T>(pipe_), done(false), product_func(product_func_)
    {}

    void start() override
    {
        if (worker.joinable()) {
            return;
        }
        worker = std::thread(&SimpleDataSource::worker_thread,this);
    }

    void stop() override
    {
        if (!worker.joinable()) {
            return;
        }
        done = true;
        worker.join();
    }

    ~SimpleDataSource() override
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
        : DataFilter<IT,OT>(in_pipe_, out_pipe_), done(false), filter_func(filter_func_)
    {}

    void start() override
    {
        if (worker.joinable()) {
            return;
        }
        worker = std::thread(&SimpleDataFilter::worker_thread,this);
    }

    void stop() override
    {
        if (!worker.joinable()) {
            return;
        }
        done = true;
        worker.join();
    }

    ~SimpleDataFilter() override
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
        : DataSink<T>(pipe_), done(false), consume_func(consume_func_)
    {}

    void start() override
    {
        if (worker.joinable()) {
            return;
        }
        worker = std::thread(&SimpleDataSink::worker_thread,this);
    }

    void stop() override
    {
        if (!worker.joinable()) {
            return;
        }
        done = true;
        worker.join();
    }

    ~SimpleDataSink() override
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

template <typename IT, typename OT>
class SimpleCompositeDataFilter: public ProcessNode {
public:
    SimpleCompositeDataFilter(Pipe<IT> in_pipe_, Pipe<OT> out_pipe_)
        : first_pipe(in_pipe_), last_pipe(out_pipe_)
    {
    }

    ~SimpleCompositeDataFilter() override
    {
        stop();
        clear();
    }

    template <typename RetType>
    SimpleCompositeDataFilter& add_first_filter(std::function<RetType(IT)> filter_func)
    {
        assert(pipes.empty());

        auto in_pipe = first_pipe;
        auto out_pipe = make_pipe<RetType>();
        pipes.push_back(out_pipe);

        auto data_filter = std::shared_ptr<ProcessNode>(
                new SimpleDataFilter<IT, RetType>(filter_func, in_pipe, out_pipe));
        filters.push_back(data_filter);
        return *this;
    }

    template <typename ArgType, typename RetType>
    SimpleCompositeDataFilter& add_filter(std::function<RetType(ArgType)> filter_func)
    {
        auto in_pipe = boost::any_cast<Pipe<ArgType>>(pipes.back());
        auto out_pipe = make_pipe<RetType>();
        pipes.push_back(out_pipe);

        auto data_filter = std::shared_ptr<ProcessNode>(
                new SimpleDataFilter<ArgType, RetType>(filter_func, in_pipe, out_pipe));
        filters.push_back(data_filter);
        return *this;
    }

    template <typename ArgType>
    SimpleCompositeDataFilter& add_last_filter(std::function<OT(ArgType)> filter_func)
    {
        assert(!pipes.empty());

        auto in_pipe = boost::any_cast<Pipe<ArgType>>(pipes.back());
        auto out_pipe = last_pipe;

        auto data_filter = std::shared_ptr<ProcessNode>(
                new SimpleDataFilter<ArgType, OT>(filter_func, in_pipe, out_pipe));
        filters.push_back(data_filter);
        return *this;
    }

    Pipe<IT> get_in_pipe() { return first_pipe; }
    Pipe<OT> get_out_pipe() { return last_pipe; }

    void start() override
    {
        for (auto data_filter : filters) {
            data_filter->start();
        }
    }

    void stop() override
    {
        for (auto data_filter : filters) {
            data_filter->stop();
        }
    }

    void clear()
    {
        filters.clear();
        pipes.clear();
    }

private:
    Pipe<IT> first_pipe;
    Pipe<OT> last_pipe;
    std::vector<boost::any> pipes;
    std::vector<std::shared_ptr<ProcessNode>> filters;
};

template <typename SourceDataType, typename SinkDataType>
class SimplePipeline: public Pipeline {
public:
    SimplePipeline()
    {
        pipes.push_back(make_pipe<SourceDataType>());
    }

    SimplePipeline& add_data_source(std::function<bool(SourceDataType&)> product_func)
    {
        auto source_data_pipe = boost::any_cast<Pipe<SourceDataType>>(pipes.front());
        auto data_source = std::shared_ptr<ProcessNode>(
                new SimpleDataSource<SourceDataType>(product_func, source_data_pipe));
        add_process_node(data_source);
        return *this;
    }

    template <typename IT, typename OT>
    SimplePipeline& add_data_filter(std::function<OT(IT)> filter_func)
    {
        auto in_pipe = boost::any_cast<Pipe<IT>>(pipes.back());

        auto out_pipe = make_pipe<OT>();
        pipes.push_back(out_pipe);

        auto data_filter = std::shared_ptr<ProcessNode>(
                new SimpleDataFilter<IT, OT>(filter_func, in_pipe, out_pipe));
        add_process_node(data_filter);
        return *this;
    }

    SimplePipeline& add_data_sink(std::function<void(SinkDataType&)> consume_func)
    {
        auto sink_data_pipe = boost::any_cast<Pipe<SinkDataType>>(pipes.back());
        auto data_sink = std::shared_ptr<ProcessNode>(
                new SimpleDataSink<SinkDataType>(consume_func, sink_data_pipe));
        add_process_node(data_sink);
        return *this;
    }

    void get(SinkDataType& value)
    {
        auto sink_data_pipe = boost::any_cast<Pipe<SinkDataType>>(pipes.back());
        sink_data_pipe->pop(value);
    }

    void put(const SourceDataType& value)
    {
        auto source_data_pipe = boost::any_cast<Pipe<SourceDataType>>(pipes.front());
        source_data_pipe->push(value);
    }

private:
    std::vector<boost::any> pipes;
};
