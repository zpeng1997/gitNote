## 0.MySQL 知识框架分为如下:
- 1.mysql框架介绍
- 2.索引优化分析
- 3.查询截取分析
- 4.主从复制
- 5.MySql锁机制

## 1.MySQL的基础架构: 一条SQL查询语句是如何执行的？
客户端 -->  连接器 ---> 查询缓存                       |   存储引擎
              |______> 分析器| --> 优化器 ---> 执行器  |   存储引擎 
            |------------- server层 ----------------- |---- 存储引擎层 ----|
- 大体分为Server层 和 存储引擎层
    - Server层负责: 存储过程, 触发器, 视图. 
    - 存储引擎层负责数据的存储提取, 其架构模式是插件式的, 指出InnoDB
    MYISAM, Memory 等多个存储引擎.(现在最常用的是InnoDB引擎)

## 2.日志系统: 一条SQL更新语句是如何执行的？
- redo log(重做日志, InnoDB的日志) 和 binlog (归档日志, server的日志)
    - redo log: 一条更新语句, InnoDB引擎就是先把记录写到redo log, 并更新内存.适当的时候, InnoDB会将这个操作记录更新到磁盘里面(空闲的时候做).
        - redo log 是固定大小, 比如配置一组4个文件, 每个文件大小1GB, 总共4GB. (写到尾部后从头开始写)
    - binlog: Server层自己的日志.
- 不同点:
    - (1.特有与通用)redo log是InnoDB引擎特有的, binlog是所有MySQL的Server层实现的, 所有引擎都可以使用.
    - (2.物理与逻辑)redo log是物理日志, 记录的是"在某个数据页上做了什么修改"; binlog是逻辑日志, 记录的是这个语句的原始逻辑, 比如"给ID=2的这一行的c字段加1". 一个底层, 一个Server层, 比较好理解.
    - (3.有限大小与无限大小)redo log是循环写的, 空间固定会用完; binlog 是可以追加写入的, 也就是binlog写到一定大小后会切换到下一个, 不会覆盖以前的日志.

## 3.事物隔离: 为什么你改了我还看不见?
- 四个性质: ACID
    - 隔离性 Isolation
    - 持久性 Durability
    - 原子性 Atomicity
    - 一致性 Consistency
- 四种隔离级别: Reasons: dirty read, non-repeatable read, phantom read
    - Read Uncommited 读取未提交内容 : 没提交就可以被其它事务看到.
    - Read Commited 读取提交内容 : 提交后, 它所做的变更才能被其它事务看到.
    - Repeatable Read 可重读(默认) :  
    - Serializable 可串行化 : 通过读锁 和 写锁, 读写锁冲突时, 后访问的事务必须等待前一个.
    - 总结: 隔离的越严实, 效率就会越低, 因此很多时候我们需要在二者之间找到一个平衡点.

### 4.主要流程如下:
事务A|事务B
--|:--:
启动事务A查询得到值1| 启动事务
- |查询得到值1
- |将1改为2
查询得到值V1 | -
- | 提交事务B
查询得到值V2 | - 
提交事务A | - 
查询得到值V3 | -

- Read Uncommited: V1的值是2, 事务B虽然没有提交, 但是A看到了, V2, V3也是 2
- Read Commited: V1是1, V2, V3是 2
- Repeatable Read: 则V1, V2是1, V3是2; V1 和 V2都是在事务A执行期间, 所以看到的数据要求一致.
- Serializable: 事务B在执行"将1改为2"时候, 会被锁住, 知道事务A执行完毕之后, 从事务A的角度看: V1 和 V2值是1, V3的值是2.
- 实现方式, 创建视图:
    - Repeatable Read: 事务启动时创建视图, 整个事务存在期间都是用这个视图.
    - Read Commited: 视图是在每个SQL语句开始执行的时候创建的.
    - Read Uncommited: 没有视图的概念, 直接返回记录上最新值.
    - Serializable:  直接加锁的方式避免并行访问.

## 5.事务的启动方式:
- 显示启动事务语句: begin 或 start transcaction。配套的提交语句是commit, 回滚语句是rollback。
- set autocommit=0, 会把线程的自动提交关闭。事务启动后, 并不会自动提交, 一直持续到主动执行commit 和 rollback语句, 或者断开连接.
    - 可以在information_schema库的innodb_trx这个表中查询长事务
    - select * from information_schema.innodb_trx where TIME_TO_SEC(timediff(now(),trx_started))>60

## 6.索引
- 哈希表: 只适用于等待查询的场景
- 有序数组索引: 只适用于静态存储引擎(不会再修改)
- N叉树
- InnoDB的索引模型: B+树
    - 主键索引(整行数据, 聚簇索引) 和 非主键索引(主键的值, 二级索引)
    - diff?
        - 主键索引:  select * from where ID=500, 搜索ID这个B+树
        - 非主键索引: select * from where k=5, 先搜索k索引树, 得到ID=500,再搜索ID索引树搜索一次, 这个过程称为回表.
        - 所以说基于非主键索引的查询需要多扫描一个索引树, 所以尽量使用主键查询.
    - 覆盖索引:
        - select ID from T where between 3 and 6, 这时只需要查ID的值, 而ID的值已经在k索引树上, 所以可以直接提供查询结果, 不需要回表. 也就是, 查询里面, 索引k已经覆盖了我们查询需求, 覆盖索引.
        - 覆盖索引是一个常用的性能优化手段(最左前缀原则)
        - 索引的复用能力: 因为可以支持最左前缀, (a,b)联合索引, 一般不需要单独在a上建立索引.

## 7.全局锁和表锁: 给表加个字段怎么有这么多阻碍?
- 锁分类: 全局锁, 表级锁 和 行锁.
    - 全局锁: 多整个数据库实例加锁, Flush tables with read lock(FTWRL): 只读. 一下语句会被阻塞.
        - 数据更新语句(增删改)
        - 数据定义语句(包括建表, 修改表结构)
        - 更新类事务的提交语句.
        - 全局锁的使用场景: 全库逻辑备份.
    - 表级锁: 表锁/元数据锁
        - 表锁: lock tables ... read/write
        - MDL: 不需要显示的加上, 访问一个表的时候会自动加上.
    - 行锁: InnoDB中, 需要的时候再加上, 等到事务结束后释放.
    - 死锁和死锁检测: 
        - 发现死锁后, 直接进入等待, 直到超时(超时时间可以通过innodb_lock_wait_timeout来设置). InnoDB中时间默认值是50s,所以如果采用第一个策略, 出现死锁后, 其他线程需要50s之后才能继续执行(对于在线服务这是无法接受的).
        - 发起死锁检测: 发现死锁后, 主动回滚死锁链条中的某个事务, 让其他事务得以继续执行, 将参数innodb_deadlock_detect设置为on, 表示开启这个逻辑.

## 8.事务到底隔离还是不隔离
- MySQL中的两个视图的概念:
    - view: 用来查询语句定义的虚拟表, 在调用的时候执行查询语句并生成结果.
    - InnoDB在实现MVCC时用到的一致性读视图, consistent read view, 用于支持Read Commited 和 Repeatable Read隔离级别的实现.