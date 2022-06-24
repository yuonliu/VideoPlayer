//
// Created by 于承业 on 2022/4/9.
//

#ifndef GLMP_PIPELINE_H
#define GLMP_PIPELINE_H

#include "pipeline/threadpool.hpp"
#include "pipeline/mpmc_queue.hpp"
#include "iostream"
#include "vector"
#include "atomic"
#include "future"
#include "mutex"
#include "condition_variable"
#include "thread"
#include "queue"
#include "type_traits"
#include "cstdio"
#include "pipeline/platform.hpp"

#define MAX_RING_BUFFER_SIZE 128

/**
 * 管道类，每个管道和一个处理函数绑定
 * @tparam IN_T 该节管道处理结果的类型
 * @tparam OUT_T 该节管道输入数据的类型
 */
template<typename IN_T, typename OUT_T, typename NEXT_OUT_T=void *>
class Pipe {
public:
    /**
     * @brief 创建一节对数据进行处理的管道
     * @details
     */
    explicit Pipe(std::function<OUT_T(IN_T)> &&f) : func(f) {};

    void process(IN_T arg) {
        result = func(std::forward<IN_T>(arg));
    }

    /**
     * [语法糖]重载管道运算符
     * @tparam NEXT_OUT_T 下一节管道的输出类型
     * @param p 下一节管道
     * @return 参数p的引用
     */
    Pipe<NEXT_OUT_T, OUT_T> &&operator|(Pipe<NEXT_OUT_T, OUT_T> &&p) {
        p_next = &p;
        return std::forward<decltype(p)>(p);
    }

    OUT_T operator()(IN_T arg) {
        process(std::forward<IN_T>(arg));
        return result;
    }

private:
    std::function<OUT_T(IN_T)> func;
    Pipe<OUT_T, NEXT_OUT_T> *p_next{nullptr};
    OUT_T result;
};

/**
 * [语法糖]重载管道运算符
 * @tparam T 待处理的数据类型
 * @tparam F 函数类型
 * @param data 待处理数据
 * @param f 普通函数、lambda函数、std::function等
 * @return 处理结果
 */
//template<typename T, typename F>
//auto operator|(T &&data, F &&f) {
//    return std::forward<F>(f)(std::forward<T>(data));
//}

template<typename OUT_T, typename IN_T>
auto make_pipe(std::function<OUT_T(IN_T)> &&f) {
    return Pipe<OUT_T, IN_T>(f);
}

template<typename IN, typename OUT>
constexpr std::function<OUT(IN)> default_func() {
    auto f = [](IN &&elem) -> OUT {
        std::cout << "default proc function: cast " << type_name<IN>() << " to " << type_name<OUT>() << std::endl;
        // if compile error here, it means you have to define the proc function yourself
        return static_cast<OUT>(elem);
    };
    return std::function<OUT(IN)>(f);
}

template<typename...> class Pipeline;

template<typename... STAGES_T>
class QueuePipeline {
public:
    using QUEUE_T = typename std::tuple<lfringqueue<STAGES_T, MAX_RING_BUFFER_SIZE>...>;
    using PROC_T = typename zip<STAGES_T...>::tuple_functions::type;
    using IN_T = typename select_type<0, STAGES_T...>::type;
    using OUT_T = typename select_last<STAGES_T...>::type;

    friend class Pipeline<STAGES_T...>;
    /**
     * @brief 创建一条管线（流水线），由多个Queue和Func组成
     * @details 每个Pipeline对象仅仅是一条管线，内部的并行是指对数据进行并行处理，而不是多条管线并行。
     * 需要多条管线并行，你需要使用更高的封装：TODO
     */
    QueuePipeline() {
        Init(std::make_integer_sequence<int, sizeof...(STAGES_T) - 1>{});
    };

    /**
     * 初始化_funcs, 设置默认处理函数
     * @tparam N number of stages
     * @return
     */
    template<int...N>
    constexpr void Init(std::integer_sequence<int, N...> const &) {
        using _unused = int[];
        (void) _unused{0, (SetFunction<N>(default_func<typename std::tuple_element<N, QUEUE_T>::type::value_type,
                typename std::tuple_element<N + 1, QUEUE_T>::type::value_type>()), 0)...};
        (void) _unused{0, (std::get<N>(_funcs)(0), 0)...};
    };

    /**
     * 为_funcs<Stage>设置处理函数
     * @tparam Stage 某一stage
     * @param function std::function
     */
    template<std::size_t Stage>
    void SetFunction(typename std::tuple_element<Stage, PROC_T>::type &&function) {
        std::get<Stage>(_funcs) = function;
    }

    void DebugInfo() {
        PRINT_TYPE(QUEUE_T);
        PRINT_TYPE(PROC_T);
        PRINT_TYPE(IN_T);
        PRINT_TYPE(OUT_T);
    }

private:
    std::size_t n_stages{sizeof...(STAGES_T)};
    QUEUE_T _queues;
    PROC_T _funcs;
};

template<typename... STAGES_T>
class Pipeline{
    using SELF_T = Pipeline<STAGES_T...>;
    using QP_T = QueuePipeline<STAGES_T...>;
    using IN_T = typename QP_T::IN_T;
    using OUT_T = typename QP_T::OUT_T;
public:
    Pipeline() = default;
    /**
     * 原地构造对象并提交；或在提交右值时被调用
     * @tparam Args
     * @param args
     */
    template<typename... Args>
    void Emplace(Args&&... args){
        IN_T *p {new IN_T(args...)};
    }
    /**
     * 复制构造对象并提交
     * @param elem 左值对象
     */
    void Submit(const IN_T& elem){
        IN_T *p {new IN_T(elem)};
    }
    /**
     * 移动右值后复制构造，并提交
     * @param elem 右值对象
     */
    void Submit(IN_T&& elem){
        Emplace(std::move(elem));
    }
    /**
     * 直接提交对象指针
     * @param elem_ptr 对象指针
     */
    void Submit(IN_T* elem_ptr){
    }
    void Get(OUT_T* ret){}
    void AsyncSubmit(IN_T&& elem){
        AsyncEmplace(std::move(elem));
        std::cout<<"IN_T&&: "<<elem<<std::endl;
    }
    void AsyncSubmit(const IN_T& elem){
        IN_T *p {new IN_T(elem)};
        std::cout<<"const IN_T&: "<<elem<<std::endl;
    }
    void AsyncSubmit(IN_T* elem){
        IN_T *p = elem;
        pp = p;
        std::cout<<"IN_T*: "<<(*elem)<<std::endl;
    }
    template<typename... Args>
    void AsyncEmplace(Args&&... args){
        IN_T *p {new IN_T(args...)};
        std::cout<<"Emplace: "<<(*p)<<std::endl;
    }

    SELF_T& operator<<(const IN_T& elem){
        AsyncSubmit(elem);
        return *this;
    }

    SELF_T& operator<<(IN_T&& elem){
        AsyncSubmit(std::forward<decltype(elem)>(elem));
        return *this;
    }
    SELF_T& operator<<(IN_T* elem_ptr){
        AsyncSubmit(elem_ptr);
        return *this;
    }
    void t(){
        std::cout<<"test:"<<(*pp)<<std::endl;
    }

    SELF_T& operator=(const SELF_T& rhs) = delete;  // 禁用赋值
    Pipeline(const SELF_T& x) = delete; // 禁用拷贝构造
    Pipeline(SELF_T&& x) = delete; // 禁用移动构造
private:
    QP_T Qp;
    IN_T* pp;
};

#endif //GLMP_PIPELINE_H
