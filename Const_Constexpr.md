### 对象的常量性
- 物理常量性: 每个bit都不可改变(C++ using it, but there is a bug!)
- 逻辑常量性: 对象的表现保持不变

```C++
// 我们可以理解, 只是const a * const = &b, 
// 不同于上面式子, 下面只是声明变量为常量, 但是指针地址不是常量
struct A{
    int *ptr;
}
int k = 5, r = 6;
const A a = {&k};
a.ptr = &r; // !error
*a.ptr = 7; // no error, 但是违背了逻辑常量性

// another example: 改变对象中的某些值不会改变其对象的逻辑常量性
class CTextBook{
public:
    std::size_t length() const;
private:
    char* pText;
    std::size_t textLength; // 在length中改变textLength是不影响逻辑常量性
    bool lengthIsValid;     // 但是违背了物理常量性, 所以需要 对应的解决方案:增加mutable关键字
}
```
- const 变量定义在.h文件中, 编译器会将这个变量放到每个.cpp文件的 匿名namespace中所以是不同变量, 
- 不会冲突。如果此const变量依赖于函数的返回值,而函数返回值不定,那么就会造成.cpp文件不同值

- 可以把引用看做为const指针, const引用看做const Type* const指针
- const_cast 去掉const含义, static_cast 可以为指针或者引用加上const含义
```C++
int i;
const int *cp = &i;
int *p = const_cast<int *>(cp);
const int* cp2 = static_cast<const int*>(p);
// C++ 类中this指针就是, 自身为const的指针, 类的const方法中 是 自身和指向都是const
```

- 类中const变量 与 static const变量
```C++
// example 1:
class B{
public:
    B():name("aaa"){
        // const变量必须在初始化列表初始化, 也就是在进入构造函数的函数体之前
        // name = "ccc"; !error
    }
private:
    const std::string name;
}

// example 2:
// c.h
class C{
    // ...
    static const std::string name;
}
//c.cpp
// 如果变量是static const类型, 那必须在类外初始化, 从属于static的属性
const std::string C::name("aaa");

// example 3:
// d.h
class D{
    // 内置类型, 如果只是使用SIZE的值, 那么直接 类内初始化 ...(1)
    static const int SIZE = 50; 
}
// d.cpp
// 如果不仅要使用值, 还需要 取地址 之类的操作, 那么还需要在类外声明 ...(2)
const int D::SIZE = 50; 
```

### const 修饰非static成员函数
- 修改变量, 调用非const函数 都禁止, mutable变量修改 允许.
- const 参与函数的重载 from effective C++, 以前看的时候没太理解
```C++
class A{
public:
    int &operator[](int i){
        ++ cachedReadCount;
        return data[i];
    }

    const int &operator[](int i) const{
        //++ size; error!
        ++ cachedReadCount;// ok
        return data[i];
    }
private:
    int size;
    mutable cachedReadCount;
    std::vector<int> data;
};

// 代码重用
int &A::operator[](int i){
    return const_cast<int &>(static_cast<const A&>(*this).operator[](i));
    // 避免调用自身导致死循环, 先转化为const后去掉, 一般不建议const_cast, 这里明确知道是调用非const对象, 所以安全
}

A &a = ...;
const A& ca = ...;
int j = a[0];
int cj = ca[0]; // call const operator[], const对象优先调用const方法
a[0] = 2;
ca[0] = 2; //error
``` 


### constexpr : 常量表达式
- 必须编译期可求
- 字面值, 全局变量/函数地址, sizeof返回的值, 
- constexpr 可用于enum, switch, 数组长度等场合
- 修饰类的构造函数, 得到constexpr对象: 初始化必须在初始化列表
```C++
// example 1:
constexpr int Inc(int i){
    return i + 1;
}

constexpr int a = Inc(1);
constexpr int b = Inc(cin.get()); // error!
constexpr int c = a *2 + 1;

// example 2:
struct A{
    // 对于构造函数与析构函数 (C++20 起)，该类必须无虚基类 
    // 构造函数必须要有空的函数体
    constexpr A(int xx, int yy):xx(x),yy(y){}
    int x, y;
}

constexpr A a(1, 2);
enum {SIZE_X = a.x, SIZE_Y = a.y};
```
- constexpr的优点: 
    - 是一种很强的约束，更好地保证程序的正确语义不被破坏。
    - 编译器可以在编译期对constexpr的代码进行非常大的优化，比如将用到的constexpr表达式都直接替换成最终结果等。
    - 相比宏来说，没有额外的开销，但更安全可靠。
    - 为什么要有constexpr? 自己去Google或者stackoverflow:)


