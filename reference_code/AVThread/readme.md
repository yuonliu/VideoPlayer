// 程序出现异常
// 原因是多线程下对象生命周期没有管理好
// 解决方式是用enable_shared_from_this使得线程也持有一个对象的引用，并吧引用设置为线程存储期
// 为了实现原本的效果，结合CRTP方式实现

// 持有线程t的对象(std::thread)在t中调用join函数, 也就是自己join自己。或者是两个线程互相join
// 原因是虽然通过std::shared_ptr管理多线程下对象生存周期的问题，但是当最后一个shared_ptr被自己这个线程持有时，线程结束析构就会自己join自己。
terminate called after throwing an instance of 'std::system_error'
  what():  Resource deadlock avoided
// 解决方式，启动线程后直接detach。因为之前已经使线程持有了一个对象的shared_ptr引用，所以不会出现析构问题。因为detach了，所以不会出现自己join自己的问题