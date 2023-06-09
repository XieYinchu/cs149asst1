## program1:Parallel Fractal Generation Using Threads
    实验要求:使用多线程并行实现图像转化,串行方式已经给出,完整并行方式即可.

首先观察串行代码
```c++
void mandelbrotSerial(
    float x0, float y0, float x1, float y1,
    int width, int height,
    int startRow, int totalRows,
    int maxIterations,
    int output[])
{
    float dx = (x1 - x0) / width;
    float dy = (y1 - y0) / height;

    int endRow = startRow + totalRows;

    for (int j = startRow; j < endRow; j++) {
        for (int i = 0; i < width; ++i) {
            float x = x0 + i * dx;
            float y = y0 + j * dy;

            int index = (j * width + i);
            output[index] = mandel(x, y, maxIterations);
        }
    }
}
```
可以看到串行时就是一行一行地进行处理,根据文档中的提示可以很容易想到并行时就是每个线程分别计算图像的一部分即可.

对于一个图像,通过给定高度height和线程数量将每个线程负责的行数给分配出来,再同时进行串行操作即可,相关代码如下:
```c++
int step = height / numThreads;
for (int i=0; i<numThreads; i++) {
    
    // TODO FOR CS149 STUDENTS: You may or may not wish to modify
    // the per-thread arguments here.  The code below copies the
    // same arguments for each thread
    args[i].x0 = x0;
    args[i].y0 = y0;
    args[i].x1 = x1;
    args[i].y1 = y1;
    args[i].width = width;
    args[i].height = height;
    args[i].maxIterations = maxIterations;
    args[i].numThreads = numThreads;
    args[i].output = output;


    args[i].startRow = i * step;
    if(i == numThreads-1) args[i].numRows = height - i*step;
    else args[i].numRows = step;
    args[i].threadId = i;
}
```
值得注意的是,由于高度和线程数不一定整除,所以对于最后一个线程来说,其numRows不一定就是一个step,而是图像全部剩余行.

现在可以给出线程数与加速比之间的关系:

![](/graph/Figure_1.png)

可以看到view1在奇数线程数时加速比反而变低了,我们可以单独检测每一个线程的执行时间来搞明白为什么,代码如下:
```c++
void workerThreadStart(WorkerArgs * const args) {

    // TODO FOR CS149 STUDENTS: Implement the body of the worker
    // thread here. Each thread should make a call to mandelbrotSerial()
    // to compute a part of the output image.  For example, in a
    // program that uses two threads, thread 0 could compute the top
    // half of the image and thread 1 could compute the bottom half.
    double stime = CycleTimer::currentSeconds();
    mandelbrotSerial(args->x0, args->y0, args->x1, args->y1, args->width, args->height, args->startRow, args->numRows, args->maxIterations, args->output);
    double etime = CycleTimer::currentSeconds();
    double traveltime = etime - stime;
    printf("[thread %d] time used:%.3f ms\n", args->threadId, traveltime*1000);
    //printf("Hello world from thread %d\n", args->threadId);
}
```
拿线程数为3的情况来说,执行情况如下:
![](/graph/thread3.png)

可以看到线程1的执行时间是远超线程0和线程2的,这种时间不均匀导致了最终时间由线程1的执行时间所决定,加速比无法达到预期

这是由于尽管给每个线程均匀分配了相同的行数来计算,其工作量却不一样.处理算法中计算该像素的复杂度与每个像素的亮度正相关，图像的中间远比上下两侧更亮，而1号线程被分配计算该图像的中间区域，所以计算耗时远远大于另外两个线程

对于其他奇数线程数的并行程序来说是一样的,并行一方面让每一个线程的平均时间减少,一方面奇数线程数在这张中间亮度很大的图里必然会使某一个线程(负责图像中间几行)的时间远超其他线程,这样总体时间就会与该线程数减一的线程数的时间差别不大,甚至会降低.但是随着线程数增多,前面一方面的影响会更大,总体时间还是减少.

为了更好地分配负载,代码修改为:
```c++
void mandelbrotStepSerial(
    float x0, float y0, float x1, float y1,
    int width, int height,
    int startRow, int step,
    int maxIterations,
    int output[])
{
    float dx = (x1 - x0) / width;
    float dy = (y1 - y0) / height;

    for (int j = startRow; j < height; j+=step) {
        for (int i = 0; i < width; ++i) {
            float x = x0 + i * dx;
            float y = y0 + j * dy;

            int index = (j * width + i);
            output[index] = mandel(x, y, maxIterations);
        }
    }
}


void workerThreadStart(WorkerArgs * const args) {

    // TODO FOR CS149 STUDENTS: Implement the body of the worker
    // thread here. Each thread should make a call to mandelbrotSerial()
    // to compute a part of the output image.  For example, in a
    // program that uses two threads, thread 0 could compute the top
    // half of the image and thread 1 could compute the bottom half.
    double stime = CycleTimer::currentSeconds();
    mandelbrotinterleavedSerial(args->x0, args->y0, args->x1, args->y1,args->width, args->height, args->threadId, args->numThreads, args->maxIterations, args->output);
    //mandelbrotSerial(args->x0, args->y0, args->x1, args->y1, args->width, args->height, args->startRow, args->numRows, args->maxIterations, args->output);
    double etime = CycleTimer::currentSeconds();
    double traveltime = etime - stime;
    printf("[thread %d] time used:%.3f ms\n", args->threadId, traveltime*1000);
    //printf("Hello world from thread %d\n", args->threadId);
}
```
主要思路是将负载分配变成interleaved,即每一个线程均匀地负责图像中的部分,基本上从头到尾.限于机器原因,view1加速比最终是5.5左右,view2加速比最终是4.5左右.本机cpu i5-10210u 4核8线程 1.60GHz

将线程数变成16,加速比并没有变更高,因为本地机器本身就是8线程的.

## Program2:Vectorizing Code Using SIMD Intrinsics
    实验要求:写出向量版本的两种运算

参照给出的absVector函数可以把exp的向量版本写出来,代码如下:
```c++
void clampedExpVector(float* values, int* exponents, float* output, int N) {

  //
  // CS149 STUDENTS TODO: Implement your vectorized version of
  // clampedExpSerial() here.
  //
  // Your solution should work for any value of
  // N and VECTOR_WIDTH, not just when VECTOR_WIDTH divides N
  //
  int i;
  __cs149_vec_float x;
  __cs149_vec_float result;
  __cs149_vec_int exp;
  __cs149_vec_int int_zero = _cs149_vset_int(0);
  __cs149_vec_int int_one = _cs149_vset_int(1);
  __cs149_vec_float clampva = _cs149_vset_float(9.999999f);
  __cs149_mask maskAll, maskIsPositive, maskIsclamped;
  for(i=0; i<N-N%VECTOR_WIDTH; i+=VECTOR_WIDTH) {
    // All ones
    maskAll = _cs149_init_ones();

    // All zeros
    maskIsPositive = _cs149_init_ones(0);
    maskIsclamped = _cs149_init_ones(0);
    result = _cs149_vset_float(1.f);
    // Load vector of values from contiguous memory addresses
    _cs149_vload_float(x, values+i, maskAll);               // x = values[i];
    _cs149_vload_int(exp, exponents+i, maskAll);          //exp = exponents[i];
    _cs149_vgt_int(maskIsPositive, exp, int_zero, maskAll); // if(exp > 0)
    while (_cs149_cntbits(maskIsPositive)) {
      _cs149_vmult_float(result, result, x, maskIsPositive);
      _cs149_vsub_int(exp, exp, int_one, maskIsPositive);
      _cs149_vgt_int(maskIsPositive, exp, int_zero, maskIsPositive);
    }

    _cs149_vgt_float(maskIsclamped, result, clampva, maskAll);
    _cs149_vset_float(result, 9.999999f, maskIsclamped);
    _cs149_vstore_float(output + i, result, maskAll);
  }

  
  clampedExpSerial(values+i, exponents+i, output+i, N-i);
  
}
```
主要思路就是使用cntbits保证将一个向量里的所有值都进行处理后再处理下一个向量,当然要注意当N剩下的数不到一个VECTOR_WIDTH时需要单独把其送入串行.

把VECTOR_WIDTH调整2,4,8,16后,可以看到vector utilization相应的变化如下:
|  VECTOR_WIDTH   | vector utilization  |
|  :----:  | :----:  |
| 2  | 79.8% |
| 4  | 72.1% |
| 8  | 68.1% |
|16  | 66.3% |

随着VECTOR_WIDTH的增大,利用率减少,这是因为在用向量处理时,是等待一个向量里所有元素计算结束才进行下一个向量的计算,相对而言利用起来的lanes就会变少.

arraysum的向量化主要就是对_cs149_hadd_float和_cs149_interleave_float的使用,把最终所得一个向量的元素相加的值转到第一个元素来,最终return output[0].详见代码.

## Program3:Parallel Fractal Generation Using ISPC
    实验要求1:运行测试程序,分析为什么加速比不如预期.
我运行view1的加速比是4.79,运行view2的加速比是4.23.都不如预期的8,view2比view1表现更差.

这是因为ispc里面用的foreach里是一个二维的.每一个program instance负责一个像素点,而每8个instance被送入一个simd一起计算,在这8个lane中计算速度是不均衡的(尽管按照foreach分配中每一个simd中的8个lane的总时间是差不多的),导致速度就是最慢lane的速度,这种情况发生在图中黑白相交的位置(因为像素点处理复杂度与亮度正相关),可以看到view2比view1黑白分界更多,所以不均衡更多,加速比更慢.
    
    实验要求2:使用--tasks参数运行程序,分析问题

我使用tasks参数运行view1加速比达到8.28,通过修改mandelbrot_ispc_withtasks中的task数,我们可以看到加速比的变化

|  tasks   | speedup  |
|  :----:  | :----:  |
| 2  | 8.29 |
| 4  | 10.47 |
| 8  | 14.31 |
|16  | 27.29 |
|32  | 24.01 |

我的结果在tasks数达到16时达到最高
## Program 4: Iterative sqrt
    实验要求1:分别跑单核与多核,说明simd贡献的加速与多核贡献的加速

我的单核加速比为4.38,多核加速比为30.14,相当于simd贡献4.38,多核贡献30.14/4.38=6.88

    实验要求2:构造输入序列,获得最坏和最好的ispc加速比

首先由于初始的猜测值都一样,为1.所以如图所示
![](/handout-images/sqrt_graph.jpg)

不同input value对应的迭代次数不同,其速度也就不同.

注意到ispc是8-wide simd instructions,只要指令中8条lane上的速度是一样的(即对应迭代次数一样),那么加速比就可以达到8倍.同时要考虑simd并行带来的开销,这个开销可以认为是固定的,那么当有效计算时间越长,优化时间加速比就更会达到理论值.选用每一个value都是1和每一个都是2.999所带来加速比分别是1.35和6.60,验证了我的猜想.这个改变对多task的运行没有什么改变,考虑到taskassign的开销,也是每一个task计算时间越多,优化时间加速比越达到理论值(我是8线程,也就是ispc*8)

为了获得最差的表现,把8个lane改的不均衡即可,每8个lane都有7个1(开销很小)1个2.999(开销很大),这样总开销就从7*1的开销+1*2.999的开销变成2.999的开销加simd并行额外开销.

结果不出所料,ispc加速比为0.92,甚至比直接串行还差!

## Program 5:BLAS saxpy
    实验要求:运行saxpy,分析为什么加速比不如预期.

运行后可以看到ISPC和task ISPC加速比分别为0.93和1.06,所以从使用tasks的加速比是1.14.是远远不如预期的.这是因为内存读写速度成了瓶颈,带宽是被占满了的,正如lecture3中所提一样.

所以在带宽问题被解决前,这个速度无法被改善.

至于TOTAL_BYTES = 4 * N * sizeof(float);中的4是因为,loadX一次loadY一次,store result时需要先把内存调入cache,一次,下一个result进入时再把cache写回内存,一次,共4次.