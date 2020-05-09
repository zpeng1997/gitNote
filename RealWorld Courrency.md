> summary of [Real-World Courrency](https://dl.acm.org/doi/pdf/10.1145/1454456.1454462)
- 单核时代 , 性能限制 -->
-->(Burroughs B5000 in 1961) disjoint CPUs concurrently executing different instruction stream but share a common memory.(In this regard) the B5000 was at least a decade ahead of its time.
-->(1980s) the need for multiprocessing became clear to a wider body of researchers, who over the course of the decade explored cache coherence protocols(eg. the Xerox Dragon and DEC Firefly), portotyped parallel operationg systems(eg. multiprocessor Unix running on the AT&T 3B20A), and developed parallel databases(Gamma at the University of Wisconsin).
-->(1990s) the seeds planted by researchers in the 1980s bore the fruit of practical systems, with many computer companies placing big bets on symmetric multiprocessing.
-->(1993 - 1997)when the TOP500 list of supercomputers was first drawn up in 1993, the highest-performing uniprocessor in the world was just#34, and more than 80 percent of the top 500 were multiprocessors of one flavvor or another. By 1997, uniprocessors were off the list entirely.
-->(trends) 1.the rise of concurrency 2.the futility(徒劳) of increasing clock rate. (the speed of main memory was not keeping up, although having deeper pipelines, caches, and prediction units to handle the problem.)  再提升时钟频率已经得不到很多的gain, 相反power,heat and area的消耗很大, 于是有人提出多核通过在一个芯片上集成多个简单的处理器核充分利用这些晶体管资源，发挥其最大的能效.（multiple (simpler) cores per die）
--> concurrent execution can improve performance in three fundamentals ways:
    1. it can reduce latency
    2. it can hide latency
    3. it can increase throughput
