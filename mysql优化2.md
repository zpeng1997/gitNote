```C++
// 从问题出发:
mysql> CREATE TABLE 't'(
    'id' int(11) NOT NULL,
    'k' int(11) DEFAULT NULL,
    PRIMARY KEY('id')
)ENGINE=InnoDB;
insert into t(id, k) values(1, 1), (2, 2);
!
```
## 基本流程:
事务A|事务B|事务C
--|:--:|:--
begin;select k from t where id=1;| - | -
- |begin; select k from t where id=1; | -
- | - | update t set k=k+1 where id=1;
- | update t set k=k+1 where id=1; (Q1)select k from t where id=1; commit; | - | -
(Q2)select k from t where id=1; commit; | - | -
 ``// 默认是 autocommit=1; 事务C没有显式的使用begin/commit,表示这个update语句本身就是一个事务,语句完成的时候会自动提交.``
 `` 语句Q1返回的k的值是3, 而语句Q2返回的k的值是1, 你是不是感觉头晕呢? 听下面分析: ``

## 8.事务到底隔离还是不隔离
- MySQL中的两个视图的概念:
    - view: 用来查询语句定义的虚拟表, 在调用的时候执行查询语句并生成结果. 创建视图的语法是create view, 而它的查询方法与表一样.
    - InnoDB在实现MVCC时用到的一致性读视图, consistent read view, 用于支持Read Commited 和 Repeatable Read隔离级别的实现. 没有物理结构, 用来在事务执行期间定义, "我能看到什么数据"
- “快照”在MVCC里是怎么工作的?
    - InnoDB中每个事务有一个唯一的事务ID(transaction id), 它是在事务开始的时候想InnoDB的事务系统申请的, 是按申请顺序严格递增的.
    - 每行数据也都是有多个版本的, 每次事务更新数据的时候, 都会生成一个新的数据版本, 并且把transaction id赋值给这个数据版本的事务ID, 记为row trx_id, 旧的版本保留, 并且在新的版本中, 能够有信息可以直接拿到它.
    - 也就是, 一个数据表的行记录,其实可能有多个版本(row), 每个版本有自己的row trx_id

开始|set k=10 transaction id=15|set k=10 transaction id=17 | set k=k*2 transaction id=25
--|:--:|:--:|:--
|<-(U1)--|<-(U2)--|<-(U3)--
(V1)k=1,row trx_id=10|(V2)k=10,row trx_id=15|(V3)k=11,row trx_id=17|(V4)k=22,row trx_id=25

- undo log在哪里？
    - 三个虚线箭头就是undo log; V1, V2, V3并不是物理上真实存在的, 而是每次需要的是根据当前版本和undo log计算出来的. 比如: 需要V2的时候, 通过V4执行U3,U2计算出来.
- InnoDB如何定义那个"100G"的快照?
    - InnoDB在代码实现上, 一个事务只需要在启动的时候, 找到所有已经提交的事务ID的最大值, 记为up_limit_id; 然后声明说, "如果一个数据版本的row trx_id 大于 up_limit_id 我就不认, 必须找到它的上一个版本".
    - 如果有一个事务, 它的up_limit_id是18, 那么当它访问这行数据, 就会从V4同U3,算出V3, 在它看来, 这一行的值是11.  InnoDB就是利用所有数据都有多个版本的特性, 实现"秒级创建快照"的能力.
- 为什么Q2语句返回的结果是k=1
    - 我们不妨有如下假设:
        - 1.事务A开始前, 系统里面已经提交的事务最大ID是99
        - 2.事务A、B、C的版本号分别是100、101、102,且当前系统里没有别的事务.
        - 3.三个事务前,(1,1)这一行数据的row trx_id是90; 这样, 事务A,B,C的up_limit_id的值就都是99

事务A(100)|事务B(101)|事务C(102)| 历史版本
--|:--:|:--:|:--
up_limit_id=99|up_limit_id=99|up_limit_id=99|  90;(1,1);历史版本2
get k=1 | - | - | -
- | - | set k=k+1 | ---> 102;(1,2);历史版本1
- | set k=k+1 | ---> |---> 101;(1,3);当前版本
Q2:get k| - | - | -

- 事务A读数据的流程,它的up_limit_id是99:
    - 找到(1,3),判断出row trx_id=101 大于 up_limit_id, 要不起
    - 找到上一个版本,判断出row trx_id=102 大于 up_limit_id, 要不起
    - 找到(1,1),判断出row trx_id=90 小于 up_limit_id, 要得起
    - 这样执行下来, 事务A读到的这个数据, 跟它在刚开始启动的时候读到的相同, 所以我们称之为一致性读.

- (1,1)这个历史版本, 什么时候可以被删除掉?
    - 答: 没有事务需要它的时候, 可以删掉
    - 若只考虑上图的三个事务, 事务B只需考虑到(1,3);事务C只需考虑到(1,2)
- 事务B的update语句到底读到的是哪个版本?
    - 事务B前面的查询语句, 拿到的k也是1, 但是它要去更新数据的时候, 不能再在历史版本上更新了, 否则事务C的更新就丢失了.
    -  set k=k+1是在(1,2)的基础上进行操作的.
    - 规则: 更新数据都是 先读都写 , 这个读, 只能读当前的值, 称为"当前读"(没有up_limit_id限制)
    - 在更新的时候, 当前读到的数据是(1,2),更新后生成(1,3),这个新版本的row trx_id是101
    - 所以在执行事务B的Q1语句的时候, 一看自己的版本号是101,最新数据的版本号也是101,可以用,所以Q1得到的k的值是3;(虽然up_limit_id是99,但是自己更改是需要确认的.)
    - 除了update语句外, select语句如果加锁, 也是当前读; 也就是Q2加上lock in share mode 或者 for update, 也都可以读到版本号是101的数据, 返回的k的值是3
    - `` mysql> select k from t where id=1 lock in share mode; //加了读锁(S锁,共享锁) ``
    - `` mysql> select k from t where id=1 for update;写锁(X锁,排它锁) ``
- 事务的可重复读的能力是怎么实现的?
    - 可重复读的核心是一致性读, 而事务更新数据的时候, 只能用当前读. 如果当前的记录的行锁被其它事务占用的话, 就需要进入锁等待.
    - 读提交 和 可重复读的 的逻辑, 它们的主要区别是:
        - 可重复读在事务开始的时候找到那个up_limit_id, 之后事务里其它查询都共用up_limit_id;
        - 读提交隔离级别下: 每个语句执行前都会重现算一次up_limit_id值.(注意语句 和 事务的区别.)
- 如果是 读提交隔离 级别下的事务, Q1返回的是k=3 

事务A(100)|事务B(101)|事务C(102)| 历史版本
--|:--:|:--:|:--
up_limit_id=99|up_limit_id=99|up_limit_id=99|  90;(1,1);当前版本
get k=1 | - | - | 当前版本2
- | - | set k=k+1 | ---> 102;(1,2);历史版本1
- | set k=k+1 | ---> |---> 101;(1,3);当前版本
- | Q1:get k;up_limit_id=102 | - | 当前版本
Q2:get k;up_limit_id=102| - | - | 当前版本

- 为什么表结构不支持可重复读
    - 表结构没有对应的行数据, 也没有row trx_id 因此只能遵循当前读的逻辑
    - MySQL8.0已经可以把表结构放在InnoDB字典里, 也许以后会支持表结构的可重复读.

## 9.普通索引还是唯一索引
- 从查询的角度: ``select id from T where k=5;``
    - 普通索引, 找到满足条件的第一个记录(5,500)后,需要查找下一个记录, 知道碰到第一个不满足k=5条件的记录.
    - 唯一索引, 由于索引定义了唯一性, 查找到第一个满足条件的记录后, 就会停止继续检索.
    - 性能差距: 微乎其微.
        - InnoDB中每个数据页的大小默认是16KB.
        - 如果都在内存中, 对于普通索引, 要多做的那一次"查找和判断下一条记录"的操作, 就只需要一次指针寻找和一次计算.
        - 当然如果k=5这个记录刚好是这个数据页的最后一个记录, 那么要取下一条记录, 必须读取下一个数据页, 这个操作稍微复杂一点; 但是, 对于整段字段, 一个数据页可以放近千个key, 因此出现这种情况的概率会很低, 所以, 我们计算平均性能差距时, 仍可以认为这个操作成本对于现在的cpu来说可以忽略不计.
- 从更新的角度
    - change buffer: 当需要更新一个数据页时, 如果数据页不在内存中, 在不影响数据一致性的前提下, InnoDB会将这些更新操作缓存在change buffer中, 这样就不需要从磁盘中读取这个数据页了. 下次查询需要访问这个数据页的时候, 将数据页读入内存, 然后执行change buffer中这个也有关的操作。通过这种方式就能保证这个数据逻辑的正确性.(change buffer它是可以持久化的数据, 也即change buffer在内存中有拷贝, 也会被写入到磁盘上.)
    - purge: 
        - (1) 访问原数据页会导致change buffer的操作应用到原数据页
        - (2) 系统有后台线程会定期purge.
        - (3) 数据库正常关闭(shutdown)的过程中, 也会执行purge操作.
    - advantages:    
        - (1) 减少读磁盘, 语句执行速度会得到明显的提升.
        - (2) 数据读入内存是需要占用buffer pool的, 所以这种方式还能够避免占用内存, 提高内存利用率.
    - 什么情况下可以使用change buffer?
        - 对于唯一索引, 所有的更新操作必须先判断这个操作是否违反唯一性约束, 比如, 如果需要插入数据, 就必须先判断这个数据是否在数据页中, 而这种情况就必须先把数据页放到内存当中, 如果已经在内存当中了, 就不需要change buffer了. 
        - 普通索引: change buffer用的是buffer pool(Innodb维护了一个缓存区域叫做Buffer Pool，用来缓存数据和索引在内存中)中的内存, 不能无限增大, change buffer的大小可以通过参数innodb_change_buffer_max_size来动态设置. 这个参数设置为50的时候, 表示change buffer的大小最多只能占用buffer pool的50%.
    - 场景实例: 因为把某个普通索引改为唯一索引, 导致某个业务的库内存命中率突然从99%降低到75%, 整个系统处于阻塞状态, 更新语句全部堵住, 探究其原因后, 发现这个业务有很多的插入数据操作.
    - change buffer的使用场景:  
        - 之前已经清楚, change buffer只限于用在普通索引的场景, 而不适用于唯一索引. 但是对于普通索引的所有场景, 使用change buffer都可以起到加速作用吗?
            - 写多读少: 账单类, 日志类的系统, 比较适合.
            - 读多写少: 反而增加了change buffer的维护代价, change buffer起到了副作用.
    - 索引选择和实战: 
     - 普通索引 和 唯一索引的差别主要在 更新 上, 建议选择普通索引. 特别使用 机械硬盘 时, change buffer这个机制的收效是非常显著的, 所以当你有一个类似"历史数据"的库, 并且处于成本考虑用的是机械硬盘, 那你应该特别关注这个表里的索引, 尽量使用普通索引, 然后把change buffer尽量开大.
    - change buffer 和 redo log:
        - redo log主要节省的是随机写磁盘的IO消耗(转成顺序写), 而change buffer主要节省的则是随机读磁盘的IO消耗。
        - `` mysql> insert into t(id,k) values(id1,k1),(id2,k2); ``, k1所在的数据页(page1)在内存(InnoDB buffer pool)中, k2所在的数据页(page2)不再内存中.
            - (1)page1在内存中,直接更新内存(数据表空间t.ibd);(2)Page2没有在内存中,就在内存的change buffer区域(系统表空间ibdata1),记录下"我要往Page2插入一行"这个消息;(3)将上述两个动作记录redo log中(顺序写磁盘).
        - `` select * from t where k in (k1, k2) `` 假设k1,k2都在内存中.
            - 读Page1的时候, 直接从内存返回; WAL之后读数据是不是一定要读盘, 是不是一定要从redo log里面把数据更新后才可以返回? 其实是不用的, 虽然磁盘还是之前的数据, 但是这里直接从内存返回结果, 结果是正确的的.
            - 要读Page2的时候, 需要把Page2从磁盘读入内存, 然后应用change buffer里面的操作日志, 生成一个正确的版本并返回结果.
            - 主键id也是唯一索引, 所以主键索引用不上, 对二级索引才有效.
    - 场景题: 如果机器掉电重启, 会不会导致change buffer丢失呢?
        - 不对丢失, 事务提交的时候, 我们把change buffer的操作也记录导入redo log里了, 所以崩溃恢复的时候, change buffer也能找回来.
        - merge 的执行流程:
            - 从磁盘读入数据页到内存(老版本的数据页)
            - 从change buffer里找出这个数据页的change buffer记录(可能有多个),一次应用, 得到新版数据页.
            - 写redo log, 这个redo log包含了数据的变更和change buffer的变更.
            - 到这里merge过程就结束了, 这时候, 数据页和内存中change buffer对应的磁盘位置还没有修改, 属于脏页, 之后各自刷回自己的物理数据.

## 10.普通索引还是唯一索引
```C++
CREATE TABLE t(
    'id' int(11) NOT NULL,
    'a' int(11) DEFAULT NULL,
    'b' int(11) DEFAULT NULL,
    PRIMARY KEY ('id'), // a, b做联合主键
    KEY 'a' ('a'),
    KEY 'b' ('b')
) ENGINE=InnoDB;
// 我们往表t中插入10万行记录, 取值按整数递增, 即(1,1,1),(2,2,2),(3,3,3)直到(100000,100000,100000)
delimiter ;;
create procedure idata()
begin
    declare i int;
    set i=1;
    while(i<=100000)do
        insert into t values(i,i,i);
        set i=i+1;
    end while;
end;;
delimiter;
call idata();
//接下来分析一条SQL语句
mysql>select * from t where a between 10000 and 20000; // 优化器选择 索引a
```
- 另一个案例,如下:
session A|session B
--|:--:
start transaction with consistent snapshot;| -
- |delete from t; call idata();
commit; | - 
- | explain select * from t where a between 10000 and 20000;

- 事务A的操作就是开启了一个事务, 随后session B把数据都删除后, 有调用了idata这个存储过程, 插入了10万行数据.
- 这时session B的查询语句 select * from t where a between 10000 and 200000就不会在选择索引a了. 可以通过慢查询日志(slow log)来查看一下具体的执行情况.
- 为了说明优化器选择的结果是正确的的, 我增加了一个对照, 即: 使用force index(a)来让优化器强制使用索引a
```C++
set sql_long_query=0; // 慢查询日志的阈值设置为0,表示这个线程接下来的语句都会被记录慢查询日志中.
select * from t where a between 10000 and 20000; //Q1
select * from t force index(a) where a between 10000 and 20000; // Q2
// Q1显然是全表扫描 40ms, Q2扫描10001行, 执行21ms
// 这个例子对应的是平常不断删除历史记录和新增数据的场景, 这时MySQL竟然会选错索引, 是不是有点奇怪! 
// 优化器是根据 扫描行数,临时表,排序 等综合判断, 需要选择哪个索引; 这里没有用到临时表 和 排序, 所以这里MySQL选错索引肯定是在判断行数的时候出错了.
// MySQL通过 采样统计 行数: 选择数据页 N, 统计这N数据页上的不同值, 得到一个平均值, 再乘以这个索引的页面数, 就得到了这个索引的基数. 因为数据表会持续更新, 索引当变更的数据行数超过1/M的时候, 会自动触发重新做一次索引统计.
// 设置innodb_stats_persistent
// 设置为on, 统计信息会持久化存储, 这时, 默认的N是20, M是10
// 设置为off, 统计信息只存储在内存, 这时, 默认的N是8, M是16
explain select * from t where a between 10000 and 20000; //Q1
// 行数104620
explain select * from t force index(a) where a between 10000 and 20000; // Q2
// 行数 37116 , 偏差较大, 应该是10001行
// 为什么选择 Q1的一万多行, 而不选择Q2的3千来行呢? 因为把每次通过索引a, 回表到主键的代价算进去了, 然后选择Q1
// 但是根本原因还是因为 行数 估计错误
analyze table t;
explain select * from t where a between 10000 and 20000; 
// 得到10001行, 纠正了错误
// 具体的矫正方式(不是特别具有一般性)
// 1.forct index(z)
// 2.语义相同后修改, select * from t where a between 1 and 1000 and b between 50000 ad 100000 order by b, a limit 1;
// 因为之前order by b limit 1 改为 order by b, a limit 1
// 之前优化器之所以选择b, 是因为他认为使用索引b可以避免排序(b本身是索引, 已经有序)
// 3.建立更合适的索引, 删掉误用的索引.
```


## 11.如何给字符串段加索引？