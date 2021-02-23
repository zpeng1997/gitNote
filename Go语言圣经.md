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
```go
package main

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