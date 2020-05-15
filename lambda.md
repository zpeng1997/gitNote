## How to use lambda correctly?
> 看了cppreference中对lambda的介绍那么长, 我一开始是拒绝的。

- Grammar
    - [ 捕获 ] <模板形参>(可选)(C++20) ( 形参 ) 说明符(可选) 异常说明 attr -> ret requires(可选)(C++20) { 函数体 } (1)	
    - [ 捕获 ] ( 形参 ) -> ret { 函数体 }  (2)
    - [ 捕获 ] ( 形参 ) { 函数体 }  (3)
    - [ 捕获 ] { 函数体 }  (4)

- 说明: (1)是完整声明, (2)是const lambda函数声明, (3)省略返回值(自动推导),(4) 不接受实参, 仅当 constexpr、mutable、异常说明、属性或尾随返回类型全都不使用时才能使用此形式。 

- lambda捕获
    - &（以引用隐式捕获被使用的自动变量）和 = （以复制隐式捕获被使用的自动变量）
    - 默认是引用捕获, this就是引用捕获, *this是复制捕获
    - 当默认捕获符是 & 时，后继的简单捕获符必须不以 & 开始。 ...(1)
    - 当默认捕获符是 = 时，后继的简单捕获符必须以 & 开始，或者为 *this (C++17 起) 或 this (C++20 起)。...(2)
    - 任何捕获符只可以出现一次： ...(3)
```C++
// example for (1)
struct S2 { void f(int i); };
void S2::f(int i)
{
    [&]{};          // OK：默认以引用捕获
    [&, i]{};       // OK：以引用捕获，但 i 以值捕获
    [&, &i] {};     // 错误：以引用捕获为默认时的以引用捕获
    [&, this] {};   // OK：等价于 [&]
    [&, this, i]{}; // OK：等价于 [&, i]
}
// example for (2)
struct S2 { void f(int i); };
void S2::f(int i)
{
    [=]{};          // OK：默认以复制捕获
    [=, &i]{};      // OK：以复制捕获，但 i 以引用捕获
    [=, *this]{};   // C++17 前：错误：无效语法
                    // C++17 起：OK：以复制捕获外围的 S2
    [=, this] {};   // C++20 前：错误：= 为默认时的 this
                    // C++20 起：OK：同 [=]
}
// example for (3)
struct S2 { void f(int i); };
void S2::f(int i)
{
    [i, i] {};        // 错误：i 重复
    [this, *this] {}; // 错误："this" 重复 (C++17)
}

// example for: from c++14
int x = 4;
auto y = [&r = x, x = x + 1]()->int
    {
        // 这里的x 不是外面的x, 跟局部变量一样
        // r的生命周期, 在闭包对象生命周期结束时结束
        r += 2;
        return x * x;
    }(); // 更新 ::x 为 6 并初始化 y 为 25。
// 这亦使得以 const 引用进行捕获称为可能，比如以 &cr = std::as_const(x) 或类似的方式
```
- lambda 的隐式捕获
    - 若捕获符列表具有默认捕获符，且未显式（以 this 或 *this）捕获其外围对象，或捕获任何自动变量，则以下情况下，它隐式捕获之 ...(3)
    - 当出现任一默认捕获符(& =)时，都能隐式捕获当前对象（*this）。当它被隐式捕获时，始终被以引用捕获，即使默认捕获符是 = 也是如此。当默认捕获符为 = 时，*this 的隐式捕获被弃用。
- 若变量满足下列条件，则 lambda 表达式可以不捕获就使用它 ...(1)
    - 该变量为非局部变量，或具有静态或线程局部存储期（该情况下无法捕获该变量），或者
    - 该变量为以常量表达式初始化的引用。
- 若变量满足下列条件，则 lambda 表达式可以不捕获就读取其值 ...(2)
    - 该变量具有 const 而非 volatile 的整型或枚举类型，并已用常量表达式初始化，或者
    - 该变量为 constexpr 且无 mutable 成员
- 简单的说 (1)是全局或静态变量, (2)是const整形或枚举(已初始化), 或constexpr类型
- ODR式使用
    - 若读取（除非它是编译时常量）或写入对象的值，取对象的地址，或将引用绑定到它，则对象被 ODR 式使用
    - 若引用有被使用，且其所引用者在编译时未知，则引用被 ODR 式使用
    - 而若调用函数或取其地址，则函数被 ODR 式使用
    - 若 ODR 式使用了对象、引用或函数，则其定义必须存在于程序中的某处

## Suggestions from <<Effective Modern C++>>
- avoid default capture modes, like [=] or [&]

# 战术放弃, 以后继续, ODR这东西有点复杂

## lambda真的没有名字吗?
```C++
[]{return flag;}
//C++11语法规范, 编译后是什么样子, __lambda_xx_xx命名
class __lambda_16_27
{
    public: 
    inline /*constexpr */ bool operator()() const
    {
        return flag;
    }
    
    using retType_16_27 = auto (*)() -> bool;
    inline /*constexpr */ operator retType_16_27 () const noexcept
    {
        return __invoke;
    };
    
    private: 
    static inline bool __invoke() // 被调用的函数, 再类中以static形式存在
    {
        return flag;
    }
    public:
    // /*constexpr */ __lambda_16_27() = default;
} __lambda_16_27{};
```
- 如果加上mutable呢?
```C++
[] ()mutable {return !flag;}
// 看看C++11语法下编译后是什么样子:
class __lambda_28_27
{
    public: 
    inline /*constexpr */ bool operator()() // 变化一: 没有const
    {
        return !flag;
    }
    
    // 这里是干什么用的?
    // 无捕获的非泛型 lambda(from cppreference.com)
    
    // C++ 17前
    // using F = ret(*)(形参);
    // operator F() const;
    
    // C++17后
    // using F = ret(*)(形参);
    // constexpr operator F() const;
    using retType_28_27 = bool (*)();
    inline /*constexpr */ operator retType_28_27 () const noexcept
    {
        return __invoke;
    };
    
    private: 
    static inline bool __invoke()
    {
        return !flag;
    }

    public:
    // /*constexpr */ __lambda_28_27() = default;
} __lambda_28_27{};
```