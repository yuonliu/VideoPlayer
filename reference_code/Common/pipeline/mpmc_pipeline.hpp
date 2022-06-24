//
// Created by 于承业 on 2022/4/15.
//

#ifndef GLMP_MPMC_PIPELINE_H
#define GLMP_MPMC_PIPELINE_H
#include "pipeline/mpmc_queue.hpp"
#include "pipeline/threadpool.hpp"
#include "3rdParty/cocoyaxi/include/co/co.h"
#include "pipeline/platform.hpp"

namespace pipeline {
#define DEFAULT_RING_BUFFER_SIZE 1024
#define PPL_DEAFULT_DISPATCH_SCHED 3
    inline thread_local auto sched_v = co::all_schedulers();

    using AnyPtr = void*;

    class Pipeline;
    class Segment;
    class Fork;
    class From;
    class Sole;
    class Output;

    class _instance {
        _instance() = default;
    };

    class _segment{
    public:
        _segment() = default;
        virtual void Submit(){}
        virtual void Get(){}
        virtual void RunOnce(){}
        virtual void Run(){}
    protected:

    };

    /**
     * 一节管道，带有一个并发安全的环形缓冲区
     */
    class _pipe_base {
        friend class Pipeline;
        friend class Segment;
        friend class Fork;
        friend class Sole;
        friend class From;
        friend class Output;
    public:
        _pipe_base() = default;
        explicit _pipe_base(std::size_t bufSize): _queue(bufSize) {}

        /**
         *
         * @tparam F 函数类型
         * @param f 处理函数
         */
        template<typename F>
        _pipe_base(F f, std::size_t bufSize = DEFAULT_RING_BUFFER_SIZE): _queue(bufSize){
            SetFunc<F>(f);
        }

        /**
         * 提交数据到缓冲区
         * @tparam T 数据对象类型
         * @param pdata 对象指针
         */
        template<typename T>
        void Submit(T* pdata){
             // TODO: maybe failed
            auto* p = new AnyPtr(pdata);
            _queue.enqueue(p);
        }

        /**
         * 提交数据到缓冲区
         * @tparam T 数据对象类型
         * @param elem 对象引用
         */
        template<typename T>
        void Submit(const T& elem){
            T* pdata = new T(elem);
            _queue.enqueue(new AnyPtr(pdata));
        }

        /**
         * 提交数据到缓冲区
         * @tparam T 数据对象类型
         * @param elem 数据对象
         */
        template<typename T>
        void Submit(T&& elem){
            T* pdata = new T(elem);
            _queue.enqueue(new AnyPtr(pdata));
        }

        /**
         * 提交数据到缓冲区，原地构造
         * @tparam T 数据对象类型
         * @tparam Args 参数类型
         * @param args 构造对象参数
         */
        template<typename T, typename... Args>
        void Emplace(Args&&... args){
            T* pdata = new T(std::forward<Args>(args)...);
            _queue.enqueue(new AnyPtr(pdata));
        }

        /**
         * 从缓冲区取数据
         * @tparam T
         * @param pdata 对象指针
         */
        template<typename T>
        void Get(T*& pdata){
            AnyPtr* p = nullptr;
            _queue.dequeue(&p);
            pdata = (p==nullptr) ? nullptr : static_cast<T*>(*p);
        }

        /**
         * 设置处理函数
         * @tparam IN 输入类型（应和Submit时的类型一致）
         * @tparam OUT 输出类型（应和下一节管道的类型一致）
         * @param f 处理函数
         */
        template<typename IN, typename OUT>
        void SetFunc(std::function<OUT*(IN*)>&& f){
            auto fp = std::make_shared<std::function<OUT*(IN*)>>(std::forward<std::function<OUT*(IN*)>>(f));
            auto wrapped_func = [this, fp](){
                IN* pdata{nullptr};
                Get(pdata);
                if(pdata != nullptr) {
                    OUT* _pdata = (*fp)(pdata);
                    _next->Submit<OUT>(_pdata);
                }
            };
            _proc = wrapped_func;
        }

        /**
         * 设置处理函数
         * @tparam F lambda表达式类型
         * @param f lambda表达式
         */
        template<typename F>
        void SetFunc(F f){
            using IN = typename lambda_traits<F>::arg0;
            using OUT = typename lambda_traits<F>::ret;
            auto fp = std::make_shared<F>(f);
            auto wrapped_func = [this, fp](){
                IN pdata{nullptr};
                Get(pdata);
                if(pdata != nullptr) {
                    OUT _pdata = (*fp)(pdata);
                    _next->Submit<std::remove_pointer_t<OUT>>(_pdata);
                }
            };
            _proc = wrapped_func;
        }

        /**
         * 进行一次数据处理（阻塞）
         */
        void RunOnce(){
            _proc();
        }

        /**
         * 设置下一节管道
         * @param next 下一节管道的指针
         */
        void SetNext(_pipe_base* next){
            _next = next;
            next->_prev = this;
            _end_proc = false;
        }
    protected:
        lfringqueue<AnyPtr, DEFAULT_RING_BUFFER_SIZE> _queue;   // TODO:考虑换成co::Pool对象池
        std::function<void()> _proc;
        bool _end_proc{true};
        _pipe_base* _next{nullptr};
        _pipe_base* _prev{nullptr};
    };

    class Output: public _pipe_base{
        friend class Pipeline;
        friend class _pipe_base;
        friend class Segment;
    public:
        void SetNext(_pipe_base* next) = delete;
        template<class T> void Get(T*& pdata) = delete;
    public:
        Output() = default;
        Output(std::size_t bufSize): _pipe_base(bufSize) {}
    };

    class From: public _pipe_base{
        friend class Pipeline;
        friend class _pipe_base;
        friend class Segment;
    public:
        // void SetNext(_pipe_base* next) = delete;
        template<class T> void Get(T*& pdata) = delete;
    public:
        From() = default;
        template<typename F>
        From(F f, std::size_t bufSize = DEFAULT_RING_BUFFER_SIZE): _pipe_base(bufSize){
            SetFunc<F>(f);
        }

        template<typename F>
        void SetFunc(F f){
            using OUT = decltype(f());
            auto fp = std::make_shared<F>(f);
            auto wrapped_func = [this, fp](){
                OUT _pdata = (*fp)();
                _next->Submit<std::remove_pointer_t<OUT>>(_pdata);
            };
            _proc = wrapped_func;
        }
    };

    class Sole: public _pipe_base{
        friend class Pipeline;
        friend class _pipe_base;
        friend class Segment;
    public:
        void SetNext(_pipe_base* next) = delete;
        template<class T> void Get(T*& pdata) = delete;
    public:
        Sole() = default;
        template<typename F>
        Sole(F f, std::size_t bufSize = DEFAULT_RING_BUFFER_SIZE): _pipe_base(bufSize){
            SetFunc<F>(f);
        }

        template<typename F>
        void SetFunc(F f){
            auto fp = std::make_shared<F>(f);
            auto wrapped_func = [this, fp](){
                (*fp)();
            };
            _proc = wrapped_func;
        }
    };


    class Fork: public _pipe_base{
        friend class Pipeline;
        friend class _pipe_base;
        friend class Segment;
    public:
        // void SetNext(_pipe_base* next) = delete;
        // template<class T> void Get(T*& pdata) = delete;
    public:
        Fork() = default;
        template<typename F>
        Fork(F f, std::size_t bufSize = DEFAULT_RING_BUFFER_SIZE): _pipe_base(bufSize){
            SetFunc<F>(f);
        }

        template<typename F>
        void SetFunc(F filter){
            using IN = typename lambda_traits<F>::arg0;
            using OUT = typename lambda_traits<F>::ret;
            static_assert(std::is_same<OUT, int>::value);
            auto fp = std::make_shared<F>(filter);
            auto wrapped_func = [this, fp](){
                IN pdata{nullptr};
                Get(pdata);
                if(pdata != nullptr) {
                    int i = (*fp)(pdata);
                    if(i>=0 && i<forks.size()){
                        forks[i]->Submit<std::remove_pointer_t<IN>>(pdata);
                    }   // 否则会被丢弃
                }
            };
            _proc = wrapped_func;
        }

        void SetNext(_pipe_base* next, int i){
            if(i<forks.size()) forks[i] = next;
            else if (i==forks.size()) forks.push_back(next);
            else return;
            next->_prev = this;
            _end_proc = false;
        }
    protected:
        std::vector<_pipe_base*> forks;
    };


    /**
     * 若干_pipe_base串成一个Segment
     */
    class Segment{
        friend class Segment;
        friend class Pipeline;
    public:
        Segment() = default;
        /**
         * 构造函数
         * @param head 管道头
         * @param n_parallel 并行数
         * @param n_concurrent 并发数
         */
        explicit Segment(_pipe_base* head, int n_parallel=1, int n_concurrent=8):_head(head){
                while(head->_next) head = head->_next;
                _tail = head;
                this->n_parallel = n_parallel;
                this->n_concurrent = n_concurrent;
        }
        /**
         * 将数据提交到头部管道
         * @tparam T
         * @param pdata 对象指针
         */
        template<typename T>
        void Submit(T* pdata){
            _head->Submit(pdata);
        }
        /**
         * 从尾部管道取出处理好的数据
         * @tparam T
         * @param pdata
         */
        template<typename T>
        void Get(T*& pdata){
            _tail->Get(pdata);
        }

        /**
         * 运行一次
         */
        void RunOnce(){
            _pipe_base* p = _head;
            if(!p) return;
            while(p->_next){
                p->RunOnce();
                p = p->_next;
            }
        }

        /**
         * 开启计算任务
         * @param blocking 此方法是否阻塞。默认为false，若为true，需要保证该方法在一个coroutine里被调用
         */
        void Run(bool blocking=false){
            _running = true;
            auto loop = [this, blocking](){
//                auto sched_v = co::all_schedulers();
                int n_scheds = co::scheduler_num();
                int sid = n_scheds-1;
                int min_sid = n_scheds-n_parallel;
                co::WaitGroup wg;
                auto f = [this, blocking, wg](){
                    RunOnce();
                    wg.done();
                };
                while(_running){
                    wg.add(n_concurrent);
                    for(int i=0;i<n_concurrent && _running;++i){
                        sched_v[sid--]->go(f);
                        if(sid<min_sid) sid = n_scheds-1;
                    }
                    co::add_timer(1);
                    co::yield();    // give other pipeline::Segment a chance to run
                    if(_running) wg.wait();
                }
            };
            if(blocking) loop();
            else sched_v[co::scheduler_num()-n_parallel]->go(loop);
//            else std::thread(loop).detach();
        }
        void Stop(){ _running = false; }

    private:
        _pipe_base* _head{nullptr};
        _pipe_base* _tail{nullptr};
        Segment* _next{nullptr};
        std::atomic<bool> _running{false};
        std::atomic<int> n_concurrent;
        std::atomic<int> n_parallel;
    };

    class Pipeline{
        friend class Pipeline;
    public:
        Pipeline() = default;
        Pipeline(Segment* head):_head(head){}
        void Run(bool blocking=true){
            Segment* pSegment = _head;
            while(pSegment){
                //TODO: 启动策略
                sched_v[PPL_DEAFULT_DISPATCH_SCHED]->go(&Segment::Run, pSegment, blocking);
                pSegment = pSegment->_next;
            }
        }
        void Stop(){
            Segment* pSegment = _head;
            while(pSegment){
                pSegment->Stop();
                pSegment = pSegment->_next;
            }
        }

    private:
        Segment* _head{nullptr};
    };


    template<typename F>
    inline _pipe_base* operator|(_pipe_base* left, F f){
        _pipe_base* right = new _pipe_base;
        right->SetFunc<F>(f);
        left->SetNext(right);
        return right;
    }

    inline Segment* parallel(_pipe_base* head, int n_parallel=2, int n_concurrent=64){
        auto* seg = new Segment(head, n_parallel, n_concurrent);
        return seg;
    }
};

#endif //GLMP_MPMC_PIPELINE_H
