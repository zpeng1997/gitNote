**What is a distributed system?**
*multiple cooperating computers
*storage for big web sites, MapReduce, peer-to-peer sharing, &c
*lots of critical infrastructure is distributed

**Why do people build distributed systems?**
*to increase capacity via parallelism
*to tolerate faults via replication
*to place computing physically close to external entities
*to achieve security via isolation

**But:**
*many concurrent parts, complex interactions
*must cope with partial failure
*tricky to realize performance potential

**Why take this course?**
*interesting -- hard problems, powerful solutions
*used by real systems -- driven by the rise of big Web sites
*active research area -- important unsolved problems
*hands-on -- you'll build real systems in the labs

**第一节课笔记**
1. 课程介绍：主要涉及两方面内容，分布式系统的性能和容错
2. 实现工具： RPC（远程过程调用），threads（多线程），并发控制（各种🔒）
3. 对于scalability，相比于雇程序员设计复杂的算法提高性能，通过直接购买机器就能获得成倍的性能显然是更划算的，但是盲目的增加机器会带来服务器与数据库的通信压力，从而限制了性能提升，所以需要设计分布式系统。
4. 对于容错，1) 可用 2) 可回恢复，两个工具1) NV存储 2)复制
5. 一致性，强一致性代价太大
6. mapreduce

**mapreduce**
* 灵感是从Lisp的map和reduce来的 和其它很多的函数式语言
* 