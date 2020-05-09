# Folly::function
- A Non-copyable Alternative to std::function

- folly::Function is a replacement for std::function which
    - is not copyable, i.e. move-only
    - is noexcept movable
    - enforces const correctness
    - as fast or faster than std::function in terms of execution time and compiler time on our relevant platform.

- how to use it in cpp
    - g++ --std=c++11 -std=c++1y -lfolly [your cpp e.g. follypromise.cpp]

## outline
- from callable types to function wrappers
- problems we found using std::function
- design decisions for folly::Function
- what we learned from migrating to folly::Function and during 18 months of production use.

### Callable Types in C++
- function pointers/references
- lambdas
- classes or structs that implement operator()
```C++
// some examples:
int (*functionPointer)(std::string)
int (&functionReference)(std::string)

auto lambda = [](std::string ss) -> int {return ss.size();};

class ComplexObject{
    static int operator()(std::string);
};

// stateful Callables
auto lambda = [x](std::string s){return s.size() + x;};
class ComplexObject{
    int operator()(std::string) const;
};

//State-Mutating Callables
auto lambda = [x](std::string s) mutable {return x += s.size();};
class ComplexObject{
    int operator()(std::string);
}; 
```

### Passing Callables
```C++
// function pointers can only be used to pass stateless callables. 
std::string work(int x);
void workAsynchronously(int x, void (*processResult)(std::string));

//Function Wrappers
void workAsynchronously(int x, 
                            std::function<void(std::string)> processResult);
template<typename R, typename... Args>
class function<R(Args...)>{
    void *state;
    void (*func)(void*, Args...);
    void (*destory)(void*);

    ~function();

    R operator() (Args...);

    template<typename F>
    function(F&& f);

    template<typename F>
    function& operator=(F&& f);
};
```

### std::function
- 48 bytes (libstdc++ on x86_64)
    - function pointer for invoking
    - pointer to management function(copy,delete,etc.)
    - 32 bytes to store wrapped object, or alternatively to store pointerro object on heap
- copyable(make copy of wrapped object)
- not noexcept movable

### Typical Use Cases
- passing a task to libraries for execution at a later time or in a different thread
- storing those tasks in the library implementations
- in either case, those tasks are never executed more than once and there is never a need to copy them.(通常只是使用一次, 不需拷贝, 当然这是在Facebook的经验, 这里我有点疑问, 不一定只是executed 一次吧？)

### More Popular Use Cases(from the Facebook code base)
```C++
folly::Executor* executor;
executor->add(callable);
```
- folly::Executor is an interface (abstract base class) for passing tasks that need to be executed.
- implementations include a thread pool which executes tasks in parallel.
- std::function<void()> was used to pass tasks to the executor.
```C++
folly::Future<std::string> result = someRpcCall(1, 2, 3);
result.then([&foo](std::string) {return foo.extractNumber(r);})
      .then([obj = std::move(obj)] (int x) {obj.setNumber(x);});
```
- Future::then takes a function that is executed when the Future has a value
- the implementation used to use std::function to store the callback.
> 其实C++11已经有future和promise, 但是Facebook实现了自己的一套, 而且腾讯也是如此, 下面是我找到的博客, 有很全面的资料: 以后有时间需要精读一遍.
- [How to design your future/promise](https://blog.csdn.net/libaineu2004/article/details/79693246)

## The Problem with std::function ....(1)
- often want to capture move-only types(e.g. unique_ptr, folly::Promise)
- std:function can only wrap copyable objects
- this did not compile:
```C++
MoveOnlyType x; // 假设现在的对象是 non-copyable
executor.add([x = std::move(x)] () mutable {x.doStuff();}); // error! 
// if you store non-copyable object in lambda, then lambda becomes non-copyable.
```

### Workarounds
```C++
auto x = std::make_shared<MoveOnlyType>(); // share_ptr is copyable
executor.add([x](){x->doStuff();});
```
- impacts performance:
    - extra memory allocation(from stack object to heap object)
    - incrementing/decrementing reference count

### Another Workarounds
```C++
folly::MoveWrapper<MoveOnlyType> x;
executor.add([x]() mutable {x->doStuff();});
```
- wrapper type that implements the copy constructor by moving the contained object
- folly::MoveWrapper<std::unique_ptr<T>> is basically std::auto_ptr<T>
- breaks copy semantics: it is bad!

### The Need for a Different Function Wrapper
- requiring all callable objects to be copyable is painful
- for most of our use cases, we never need to make copies of functions
- you want this to work:
```C++
MoveOnlyType x; 
executor.add([x = std::move(x)] () mutable {x.doStuff();});
```

## Another Problem: Const Correctness (I do not understand!!!) ....(2)
- std::function's operator() is declared [const](https://en.cppreference.com/w/cpp/utility/functional/function/operator())
- but it invokes the wrapped object as a non-const reference
- it does not enforce const correctness
```C++
// const reference: it's right
// non-const reference: the compiler can't help you, if you code it wrong.
std::function<int(int)> func2 = [](int i) mutable { return ++ i; };
std::cout << "func2: " << func2(6) << '\n';
// 这里说的应该是, 虽然std::function的operator是const的, 调用一个对象as non-const reference
// 为什么这么说, 因为它调用上面mutable lambda as const reference则会编译失败
// 下面有例子证明.
```

### some notes about lambda and std::function 
- 如果你传递值，你将复制闭包对象（假设你没有定义lambda内联，在这种情况下它将被移动）。如果复制状态昂贵，则可能不合需要，如果状态不可复制，则无法编译。
```C++
template <class Function>
void higherOrderFunction(Function f);

std::unique_ptr<int> p;
auto l = [p = std::move(p)] {}; // C++14 lambda with init capture
higherOrderFunction(l);         // doesn't compile because l is non-copyable 
                                // due to unique_ptr member
higherOrderFunction([p = std::move(p)] {}); // this still works, the closure object is moved
```
- 如果通过const引用传递，则无法传递mutable将其数据成员修改为参数higherOrderFunction()的mutable lambda ，因为lambda具有非const operator()，并且您无法在const对象上调用它。
```C++
template <class Function>
void higherOrderFunction(Function const& f);

int i = 0;
higherOrderFunction([=]() mutable { i = 0; }); // will not compile
// mutable meaning in lambda : mutable允许 函数体 修改各个复制捕获的对象，以及调用其非 const 成员函数 
```
- 最好的选择是使用转发参考。然后higherOrderFunction可以接受调用者传递的左值或右值。
```C++
template <class Function>
void higherOrderFunction(Function&& f) {
     std::forward<Function>(f)();
}
```

# How does Folly::Function look like ? How to implement it?

### Folly::Function
- must be able to store non-copyable callables
- on run-time performance regression(倒退)
- maintain value semantics
- therefore must be non-copyable itself(becase we hardly want to copy it.)

### noexcept-Movable
- non-copyable types really need to be noexcept movable.
- std::move_if_noexcept is used e.g. by STL Containers.
- classes containing folly::Function members would be rendered not-except-movable if folly::Function was not noexcept-movable.

### Const Correct
- folly::Function comes in two flavours:
    - folly::Function<void()>  // non-const 负责 mutable
    - folly::Function<void() const> // const 负责const

### Implementation Details
- 64 bytes(on x86_64)
    - function pointer for invoking
    - pointer to management function(move,delete,etc)
    - 48bytes to store wrapped objects inline
- types that are not noexcept-movable are never stored inline

- Why 48 bytes store rapped objects?
    - this is a trade-off
    - the larger you make the wrapper the more objects you can store inline without extra memory allocation.
    - the larger you make the more memory you just waste if you end up storing small objects.
    - it aligns nicely with cache lines on our standard architecuture.

### Trivia
- std::function object can be converted to folly::Function, but not vice versa

### Migrating to folly::Function
- folly::Function works as a drop-in replacement for std::function
- unless copyability is needed(surprisingly rare)
- or lax const behaviour is needed(considered a bug)
- const variant is rarely needed, most function objects are non-const
- std::function often passed as a const&, when replaced with non-const folly::Function must be passed as & or && ...(1)
    - (1) the author say that std::function often passed as a const& may led to some sort of copies or accident. (作者后面也说了, 这是传mutable variant, ....也不是特别清楚)

### Adoption at Facebook
- folly::Function replaced std::function in folly::Future
- instant win: can use move-only callbacks
- folly::Function repalced std::function in folly::Executor
- required audit/changes to all derived classes
- where migration was non-trivial, it usually pointed to code that needed fixing anyway(expecially when you copy an object.)

### std::function VS folly::Funcion
- both can be useful
- std::function in APIs where copies of function objects will be needed
- folly::Function everywhere else, to be less restrictive
- use of copy-as-move wrappers(like folly::MoveWrapper) problematic as they make non-copyable things appear copyable

### Benchmark

