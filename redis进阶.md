## what is redis?
- remote dicitionary service.

## redis conf
- includes: 可以按模块分类
- network:
    - tcp-backlog: backblog是一个链接队列, =未完成三次握手队列 + 已经完成三次握手队列. 避免慢客户链接问题.
    - tcp-keepalive: 心跳检测. 
    - timeout: 客户端闲置多少时间后关闭连接.
- gengeral:
    - loglevel: 默认notice, debug, verbose, notice, warning.
    - database: 默认database的数量.
    - logfile: 
    - maxclients: 0为无限制, 默认无限制
    - syslog: 打系统log
    - flushall: 会生成空的dump.rdb文件.
- security:
    - masterauth: master设置密码保护, slave连接master需要密码
    - requirepass: 设置redis连接密码
- `` config get requirepass, config set requirepass ``
- RDB snapshot: 主机和备机一定要是不同的机器.

    - save(只管保存, 其它所有操作阻塞), bgsave(异步保存, 可以响应客户端请求), lastave查看最后一次成功执行快照的时间.
    - stop-writes-on-bgsave-error: 磁盘错误是否停止写.
    - rdbcompression: yes, 压缩存储
    - rdbchecksum: CRC64算法校验
    - dbfilename: 
    - dir:
    - 如何恢复: CONFIG GET dir获取目录, 将备份文件移动到redis安装目录并启动服务即可.
    - 优势: 适合大规模的数据恢复, 对数据完整性和一致性要求不高.
    - 劣势: 一定间隔时间做一次备份, 所以redis以外down掉, 会丢失最后一次快照后的所有修改. Fork进程, 内存数据被克隆一份, 2倍内存需要考虑.
- AOF:
    - 以日志的形式记录写操作. appendonly.aof
    - aof 和 rdb同时存在, 可以同时加载.
    - redis-check-aof --fix appendonly.aof
    - appendfsync： everysec默认, always每次(导致性能差), no
    - 优势:
        - no-appendfsync-on-rewrite: 重写是否可运用appendfsync, 大于min-size大小时, 开始扩充一倍然后rewrite.
        - auto-aof-rewrite-min-size: 重写基准
        - auto-aof-rewrite-percentage:
    - 劣势: 相同数据集的数据而言aof文件要远大于rdb文件, 恢复速度快与rdb
        - aof慢与rdb文件, 每秒同步策略效率较好, 不同步效率和rdb相同.   
    - 性能建议:
        - 建议只在slave上持久化rdb文件, 只保留save 900 1,
        - enable AOF, 大小5G以上, 最恶劣的情况跟丢失不超过两秒数据. 但是rewrite到新文件的阻塞是不可避免的.
        - 不enable AOF, 靠master-slave replication实现高可用性, 代价是master-slave同时倒掉, 会丢失十几分钟数据. 但是减少了rewrite的IO时间.  
- 配置命令:
    - CONFIG SET 可以动态调整而无需重启redis, 但是只是运行时更改, 没有改文件
    - ./redis-server ../redis.conf 服务器需要这样启动.
    - CONFIG REWRITE 
- 调试命令:
    - slowlog: 在内存中, FIFO队列, 记录查询(执行时间超过某一时间的)命令
    - mintor: 实时打印出服务器接收到的命令.
- 服务器与客户端:
    - client list: 写出所有的client
    - SHUTDOWN: 是关闭所有客户端和redis服务器, quit是直接退出客户端.

## redis事务:
- multi exec事务的开始与结束, discard 事务打断.
- watch unwatch 监视, 一个或者多个key
    - 放在mutil之前, 属于乐观锁; 分布式锁是悲观锁.
    - 乐观锁: 不加锁, 提交版本必须要大于当前版本。CAS(Check And Set)算法.
        - 成功修改, 正常返回; 修改失败, 返回(nil).
    - 悲观锁: 数据库本身锁机制实现.
- 放到缓存队列当中, 因为是单线程, 所以不用担心会被干扰.
- redis的事务原子性:
    - 语法问题, 都不会执行
    - 操作问题, 错误操作前面的操作可以正常执行.

## redis发布订阅
- 进程间的通信模式, 发送者(pub)发送消息, 订阅者(sub)接收消息

## redis主从复制
- 配从(库)不配主(库)
- 从库配置: slaveof主库ip主库端口
    - 每次与master断开后, 都需要重新连接, 除非你配置进redis.conf文件
    - Info replication:
    - slaveof host ip:
    - slaveof no one:
    - 从机不可以写. 多搞破坏, 看看其他时刻的执行情况(不是光吸收书本的知识, 尽信书不如无书.)
    - 主机死后, 从机不夺位; 主机恢复后还是主机.
- 修改配置文件细节操作:
    - 1. 拷贝多个redis.conf文件
    - 2. 开启daemonize yes
    - 3. pid文件名字
    - 4. 指定端口
    - 5. Log文件名字
    - 6. Dump.rdb名字
- 常用3招:
    - 1. 一主二仆: 
    - 2. 薪火相传: 
    - 3. 反客为主: 
- 哨兵模式:
    - 主机崩溃, 剩下从机投票选择主机.
    - 复制延迟:
- redis集群:    
    - 

## redis持久化
- rdb(Redis DataBase), aof(Append Only File)
- 在指定时间间隔内将内存中的数据集快照写入磁盘, Snapshot快照.
- redis 先fork一个子进程来持久化, 先会将数据写入到一个临时文件中, 待持久化过程结束, 再用这个临时文件替换上次持久化好的文件. RDB的缺点就是最后一次持久化后的数据可能丢失.

## redis的应用场景
- 记录帖子的点赞数、评论数和点击数 (hash)。 
- 记录用户的帖子 ID 列表 (排序)，便于快速显示用户的帖子列表 (zset)。
- 记录帖子的标题、摘要、作者和封面信息，用于列表页展示 (hash)。
- 记录帖子的点赞用户 ID 列表，评论 ID 列表，用于显示和去重计数 (zset)。
- 缓存近期热帖内容 (帖子内容空间占用比较大)，减少数据库压力 (hash)。
- 记录帖子的相关文章 ID，根据内容推荐相关帖子 (list)。
- 如果帖子 ID 是整数自增的，可以使用 Redis 来分配帖子 ID(计数器)。
- 收藏集和帖子之间的关系 (zset)。
- 记录热榜帖子 ID 列表，总热榜和分类热榜 (zset)。
- 缓存用户行为历史，进行恶意行为过滤 (zset,hash)。