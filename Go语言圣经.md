
## Go的起源
> Go是一个开源的编程语言，它很容易用于构建简单、可靠和高效的软件。
* 在C语言发明之后约5年的时间之后（1978年），Brian W. Kernighan和Dennis M. Ritchie合作编写出版了C语言方面的经典教材《The C Programming Language》，该书被誉为C语言程序员的圣经，作者也被大家亲切地称为K&R。同样在Go语言正式发布（2009年）约5年之后（2014年开始写作，2015年出版），由Go语言核心团队成员Alan A. A. Donovan和K&R中的Brian W. Kernighan合作编写了Go语言方面的经典教材《The Go Programming Language》
* 从早期提交日志中也可以看出，Go语言是从Ken Thompson发明的B语言、Dennis M. Ritchie发明的C语言逐步演化过来的，是C语言家族的成员，因此很多人将Go语言称为21世纪的C语言。纵观这几年来的发展趋势，Go语言已经成为云计算、云存储时代最重要的基础编程语言。
* [How Go Was Made](https://talks.golang.org/2015/how-go-was-made.slide)
![Go语言的起源](https://github.com/zpeng1997/gitNote/blob/master/Picture/Go语言的起源.png)

## go安装教程
```
//1、wget方式下载并解压
wget https://dl.google.com/go/go1.9.3.linux-amd64.tar.gz
sudo tar -xzf go1.9.3.linux-amd64.tar.gz -C /usr/local/lib
// 2、添加配置
echo 'export GOROOT=/usr/local/lib/go
export GOBIN=$GOROOT/bin
export PATH=$PATH:$GOBIN
export GOPATH=$HOME/.go' | sudo tee -a /etc/profile.d/go.sh
# 让配置生效
sudo source /etc/profile.d/go.sh
//3、下载web框架gin
go get github.com/gin-gonic/gin
```
[Linux环境下工作空间搭建](https://www.cnblogs.com/qtiger/p/14062129.html)

## go的特殊点(主要是与C++的区别)
* Go语言不需要在语句或者声明的末尾添加分号，除非一行上有多条语句。实际上，编译器会主动把特定符号后的换行符转换为分号, 因此换行符添加的位置会影响Go代码的正确解析（译注：比如行末是标识符、整数、浮点数、虚数、字符或字符串文字、关键字break、continue、fallthrough或return中的一个、运算符和分隔符++、--、)、]或}中的一个）。举个例子, 函数的左括号{必须和func函数声明在同一行上, 且位于末尾，不能独占一行，而在表达式x + y中，可在+后换行，不能在+前换行（译注：以+结尾的话不会被插入分号分隔符，但是以x结尾的话则会被分号分隔符，从而导致编译错误）。
* 在Go语⾔ 中，函数参数都是以复制的⽅式(不⽀持以引⽤的⽅式)传递（⽐较特殊的是，Go语⾔闭包函数对外部 变量是以引⽤的⽅式使⽤）
* 自增语句i++给i加1；这和i += 1以及i = i + 1都是等价的。对应的还有i--给i减1。它们是语句，而不像C系的其它语言那样是表达式。所以j = i++非法，而且++和--都只能放在变量名后面，因此--i也非法
* for循环三个部分不需括号包围。大括号强制要求, 左大括号必须和post语句在同一行。for循环的这三个部分每个都可以省略，如果省略initialization和post，分号也可以省略。如果连condition也省略了, 这就变成一个无限循环，尽管如此，还可以用其他方式终止循环, 如一条break或return语句。
* for的另一种形式: 在某种数据类型的区间（range）上遍历，如字符串或切片.
* 1.命名: 包的名字小写, 包中可以导出的首字母大写; 一般而言, Go语言习惯于短变量命名, 一般声明周期越长, 作用域越大, 其名字也就越长. 驼峰式, 一般不用下划线.
* 2.声明: 全局(属于对应的包)、局部(只属于对应的函数). 
```C++
import (
    "fmt"
    "os"
)

func main() {
    // 一般使用前两种形式
    // 第一种形式，是一条短变量声明，最简洁，但只能用在函数内部,而不能用于包变量
    s, sep := "", ""
    // 第二种形式依赖于字符串的默认初始化零值机制，被初始化为""
    // var s string
    // 第三种形式用得很少，除非同时声明多个变量
    // var s = ""
    // 第四种形式显式地标明变量的类型，当变量类型与初值类型相同时，
    // 类型冗余，但如果两者类型不同，变量类型就必须了
    // var s string = ""

    // Go语言中这种情况的解决方法是用空标识符（blank identifier）
    // 即_（也就是下划线）。空标识符可用于任何语法需要变量名但程序逻辑不需要的时候
    for _, arg := range os.Args[1:] {
        s += sep + arg
        // s每次赋予新的值时, s原来的内容已经不再使用，将在适当时机对它进行垃圾回收
        // 当s过大时, 代价比较大.
        sep = " "
    }
    fmt.Println(s)
}
```
* 另一种实现方式
```go
func main() {
    fmt.Println(strings.Join(os.Args[1:], " "))
}

// exercise 1.2
func main() {
	for i, arg := range os.Args {
		fmt.Printf("%d: %s\n", i, arg)
	}
}
```
* 3.变量
```go
// var 变量名字 类型 = 表达式
var i, j, k int                 // int, int, int
var b, f, s = true, 2.3, "four" // bool, float64, string
// 在包级别声明的变量会在main入口函数执行前完成初始化
// 局部变量将在声明语句被执行到的时候完成初始化
var f, err = os.Open(name)
// 一组变量也可以通过调用一个函数，由函数返回的多个返回值初始化：

// 简短声明变量(适用于函数内部), 名字:=表达式
anim := git.GIF{LoopCount, nframes}
freq := rand.Float64() * 3.0
t := 0.0

// var形式的声明语句往往是用于需要显式指定变量类型的地方
// 或者因为变量稍后会被重新赋值而初始值无关紧要的地方。
i := 100                  // an int
var boiling float64 = 100 // a float64
var names []string
var err error
var p Point
// 初始化一组变量, 只适用于提高代码可读性的地方, 例如for循环内部
i, j := 1, 3
// 请记住“:=”是一个变量声明语句，而“=”是一个变量赋值操作
// 简洁赋值语句 := 可在类型明确的地方代替 var 声明
// 也不要混淆多个变量的声明和元组的多重赋值

// 简短声明, 有时候并不是声明操作
// 第一个语句声明了in和err两个变量。
// 在第二个语句只声明了out一个变量，然后对已经声明的err进行了赋值操作。
in, err := os.Open(infile)
out, err := os.Create(outfile)
// 简短变量声明语句中必须至少要声明一个新的变量，下面的代码将不能编译通过：
f, err := os.Open(infile)
f, err := os.Create(outfile) // compile error: no new variables
// 解决的方法是第二个简短变量声明语句改用普通的多重赋值语句
f, err = os.Create(outfile) // right!
```
* 4.指针
```go
// 
x := 1
p := &x         // p, of type *int, points to x
fmt.Println(*p) // "1"
*p = 2          // equivalent to x = 2
fmt.Println(x)  // "2"
// 在Go语言中，返回函数中局部变量的地址也是安全的
var p = f()
func f() *int{
    v := 1;
    return &v;
}
// 每次调用f函数都将返回不同的结果
fmt.Println(f() == f()) 
// 由于new只是一个预定义的函数，它并不是一个关键字，因此我们可以将new名字重新定义为别的类型。例如下面的例子：
```
* 5. 声明周期
```c++
// 而相比之下，局部变量的生命周期则是动态的：
// 每次从创建一个新变量的声明语句开始，直到该变量不再被引用为止
```
* 6. 返回值
```go
func split(sum int) (x, y int) {
	x = sum * 4 / 9
	y = sum - x
	return
}

func main() {
	fmt.Println(split(17))
}
```
* 7. if,else 局部变量的作用范围
* 在 if 的简短语句中声明的变量同样可以在任何对应的 else 块中使用
* defer 语句
```go
package main
import "fmt"

func main() {
	defer fmt.Println("world")

	fmt.Println("hello")
}

// 而且是压入栈中, 函数返回后依次调用
func main() {
	fmt.Println("counting")

	for i := 0; i < 10; i++ {
		defer fmt.Println(i)
	}

	fmt.Println("done")
}
```
* 数组
```go
func main(){
    var a [2]string
    a[0] = "Hello"
    a[1] = "World"

    primes := [6]int{2, 3, 5, 7, 11, 13}
	fmt.Println(primes)
    // 切片就像数组的引用
    var s []int = primes[1:4]
    fmt.Println(s)
    s[0] = 9 // 会改变primes[1]的值

    // 类似c++的 vector<pair<int, bool>>
    s := []struct {
		i int
		b bool
	}{
		{2, true},
		{3, false},
		{5, true},
		{7, true},
		{11, false},
		{13, true},
	}

    a := make([]int, 5)
    // a len=5 cap=5 [0 0 0 0 0]
    b := make([]int, 0, 5)
    // b len=0 cap=5 []
    c := b[:2]
    // c len=2 cap=5 [0 0]
    d := c[2:5]
    // d len=3 cap=3 [0 0 0]

    // append 函数 可向切片追加元素
    // 这个切片会按需增长
	s = append(s, 1)
    // range 遍历, _ 忽略变量
}
```
* map映射
```go
type Vertex struct {
	Lat, Long float64
}

var m = map[string]Vertex{
	"Bell Labs": Vertex{
		40.68433, -74.39967,
	},
	"Google": Vertex{
		37.42202, -122.08408,
	},
}
```
* 函数可以作为参数和返回值
```go
func compute(fn func(float64, float64) float64) float64 {
	return fn(3, 4)
}

func main() {
	hypot := func(x, y float64) float64 {
		return math.Sqrt(x*x + y*y)
	}
	fmt.Println(hypot(5, 12))

	fmt.Println(compute(hypot))
	fmt.Println(compute(math.Pow))
}
```
* 方法
```go
// 方法接收者在它自己的参数列表内，位于 func 关键字和方法名之间。
// 在此例中，Abs 方法拥有一个名为 v，类型为 Vertex 的接收者
// 听着很玄虚 --> 就是的成员函数
type Vertex struct {
	X, Y float64
}

// func Vectoex::Abs() float64{
// 这样是不是好理解多了
// }
// 不一样的是, 这里的方法接收者
// 不是类的方法, 而是对象的方法,
// 把下面 (v Vertex), 则必须有一个Vertex 的变量v来访问.
func (v Vertex) Abs() float64 {
	return math.Sqrt(v.X*v.X + v.Y*v.Y)
}

func main() {
	v := Vertex{3, 4}
	fmt.Println(v.Abs())
}

// 这意味着对于某类型 T，接收者的类型可以用 *T 的文法。（此外，T 不能是像 *int 这样的指针。）
// 指针接收者的方法可以修改接收者指向的值（就像 Scale 在这做的）。由于方法经常需要修改它的接收者，指针接收者比值接收者更常用。
// 普通接受者不可以修改
func (v Vertex) Abs() float64 {
	return math.Sqrt(v.X*v.X + v.Y*v.Y)
}

func (v *Vertex) Scale(f float64) {
	v.X = v.X * f
	v.Y = v.Y * f
}

func main() {
	v := Vertex{3, 4}
	v.Scale(10)
	fmt.Println(v.Abs())
}

// 比较前两个程序，你大概会注意到带指针参数的函数必须接受一个指针：

var v Vertex
ScaleFunc(v, 5)  // 编译错误！
ScaleFunc(&v, 5) // OK

// 而以指针为接收者的方法被调用时，接收者既能为值又能为指针：
var v Vertex
v.Scale(5)  // OK
p := &v
p.Scale(10) // OK

// 即便 v 是个值而非指针，带指针接收者的方法也能被直接调用。 
// 也就是说，由于 Scale 方法有一个指针接收者，为方便起见，
// Go 会将语句 v.Scale(5) 解释为 (&v).Scale(5)
// 相反竟然也可以, 而以值为接收者的方法被调用时，接收者既能为值又能为指针

// 使用指针接收者的原因有二：
// 首先，方法能够修改其接收者指向的值。
// 其次，这样可以避免在每次调用方法时复制该值。若值的类型为大型结构体时，这样做会更加高效
```
* 接口
```go
// 隐式接口从接口的实现中解耦了定义，这样接口的实现可以出现在任何包中，无需提前准备。
// 接口也是值。它们可以像其它值一样传递。
// 接口值可以用作函数的参数或返回值。
// 在内部，接口值可以看做包含值和具体类型的元组：
// (value, type)
// 接口值保存了一个具体底层类型的具体值。
// 接口值调用方法时会执行其底层类型的同名方法。
fmt.Printf("(%v, %T)\n", i, i)
// (&{Hello}, *main.T)
// 空接口:
func describe(i interface{}) {
	fmt.Printf("(%v, %T)\n", i, i)
}
// (<nil>, <nil>)
```
* 断言
```go
func main() {
	var i interface{} = "hello"

	s := i.(string)
	fmt.Println(s)

	s, ok := i.(string)
	fmt.Println(s, ok)

	f, ok := i.(float64)
	fmt.Println(f, ok)

	f = i.(float64) // 报错(panic)
	fmt.Println(f)
}
```
*  

## 遇到的问题
```
// 代理就不算了
// 第一个
$go get golang.org/x/tools/cmd/goimports: copying /tmp/go-build3964966949/b001/exe/a.out: open /usr/local/lib/go/bin/goimports: permission denied

//解决方案: 修改文件夹权限
sudo chmod -R 777 go/

```


## Reference
* [GO语言圣经中文版](https://docs.hacknode.org/gopl-zh/)
* [GoForum](https://forum.golangbridge.org/)
* [Go语言中文网](https://studygolang.com/)
* [The Go Programming Language](http://www.gopl.io/)