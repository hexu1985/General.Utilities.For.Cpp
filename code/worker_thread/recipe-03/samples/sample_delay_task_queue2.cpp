/** \example task_queue/sample_task_queue2.cpp
 * This is an example of how to use the TaskQueue class.
 */
#include "task_queue.hpp"
#include <thread>
#include <string>
#include <iostream>
#include <stdexcept>

using namespace mini_util;

void processor(TaskQueue &task_queue)
{
    while (true) {
    TaskPtrList task_list;
        task_queue.popTask(task_list);
        while (!task_list.empty()) {
            auto task = task_list.front();
            task_list.pop_front();
            try {
                task->run();
            } catch (const std::runtime_error &e) {
                std::cout << e.what() << std::endl;
                return;
            }
        }
    }
}

void print_int(int i)
{
    std::cout << __func__ << "(" << i << ")" << std::endl;
}

void print_string(std::string str)
{
    std::cout << __func__ << "(" << str << ")" << std::endl;
}

class Foo {
    int n_;
public:
    Foo(int n): n_(n) {}

    void print()
    {
        std::cout << "Foo::" << __func__ << "(" << n_++ << ")" << std::endl;
    }
};

void process_exit()
{
    std::cout << "task to stop process thread" << std::endl;
    throw std::runtime_error("stop process thread");
}

int main()
{
    Foo foo(1);
    std::shared_ptr<Foo> pfoo(new Foo(101));

    TaskQueue task_queue;
    std::thread proc_thread(processor, std::ref(task_queue));
    std::string str = "hello";
    for (int i = 0; i < 10; i++) {
        int delay = 1000*(i+1);
        task_queue.pushTask(make_task(print_int, i), delay+500);
        task_queue.pushTask(make_task(print_string, str), delay+100);
        task_queue.pushTask(make_task(&Foo::print, &foo), delay+200);
        task_queue.pushTask(make_task(&Foo::print, pfoo), delay+300);
    }
    task_queue.pushTask(make_task(process_exit), 15000);
//    while (true) {
//        bool has_task = false;
//        std::chrono::system_clock::time_point first_time_up{};
//        std::tie(has_task, first_time_up) = task_queue.firstTimeUp();
//        if (!has_task) {
//            break;
//        } 
//        std::this_thread::sleep_until(first_time_up);
//        task_queue.wakeUp();
//    }
    proc_thread.join();
    return 0;
}

