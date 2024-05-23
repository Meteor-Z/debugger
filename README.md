# debugger

模仿GDB的 C/C++ Mini Debugger

## 必要软件

- vscode
- cmake
- clang/clangd

## 前提库

需要的两个库

- 在linux上
  - [linenoise](https://github.com/antirez/linenoise)
    - 作为C/C++的一个小型输入的。两个文件，直接链接就能使用了
  - [libelfin](https://github.com/TartanLlama/libelfin/tree/fbreg)
    - 目前还没有找到好用的库，这个库不支持最新版本的`DWARF 5`，所以调试的是一定要加入参数 `-gdwarf-3` or `gdwarf-4`
    - 安装的时候切换到这个`fbreg`的分支
    - 又发现了新问题，gcc13.2生成的版本很多都是`DWARF 5`, 导致直接寄了，这里还是直接使用 gcc 11版本吧（ubuntu 22.04）
    - 有一些文件，建议自行编译安装到本地，然后进行链接
  - [fmt](https://github.com/fmtlib/fmt)
    - 很平常的C/C++ 输出的库，主要是不想切换到C++20了，但是也不想带这个库了，主要是还要编译相关的库，后面把这个去掉

## 内容文件信息

这里是待补充的，项目中的文件信息。

## 调试原理

这里的知识点我写到了我的知乎上了，可以看一下: [gdb调试的原理](https://www.zhihu.com/question/578172542/answer/3389041105)，收获的赞还不少捏（

调试器最重要的原理就是ptrace这个系统调用函数，

基本上依赖于这个函数`ptrace`，

- `long ptrace(request, pid_t pid, void *addr, void *data)`
  - request: 宏定义，比如说`PTRACE_TRACEME`, `PTRACE_PEEKDATA`等，表示一个动作
  - pid: 要调试的进程号
  - addr: 要写入的地址
  - data: 写入地址的话 data就是数据，读出地址的话，就会赋值给data。

可以`接管`一个子程序的执行，跟踪一个程序，Linux上的程序执行差不多是这个样子：父进程fork()一遍一个几乎一摸一样的自己，然后excel()一个新进程，这时候，就会将fork出来的进程替换成要执行的进程，使用`ptrace`可以跟踪这个新产生的进程，然后进行调试，代码如下

```c++
#include <sys/ptrace.h>
#include <unistd.h>
#include <sched.h>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        return -1;
    }

    std::string program_name { argv[1] };
    pid_t pid = fork();

    // 子进程

    if (pid == 0) {
        // 设置跟踪状态
        ptrace(PTRACE_TRACEME, 0, nullptr, nullptr);
        execl(program_name.c_str(), program_name.c_str(), nullptr);
    }
    // 父进程

    if (pid > 0) {
        // 进行调试等操作
    }
}
```

### 设置中断

`ptrace()`函数调用可以对函数的汇编代码进行一定的修改，在x86上有一条汇编指令，叫做 INT 3（这里的INT 不是 int，而是interrupt的缩写），当出现这个指令的时候，程序就会发生中断，也就是trap了，然后父进程就可以捕获这个信号，子进程就会中断在那里，等待下一步的命令，也就是说当gdb进行打断点的时候，就是将一条汇编代码修改一下，将其修改成INT 3的机器码，而INT 3的机器码是多少呢？是0xcc, 所以说当打断点的时候，就会将原来的汇编代码对应的机器码中的机器码指令换成0xcc，当程序执行到一步的时候，就会触发断点，等到调试，那如何跳出这个断点？，将要被替换的机器码指令保存下来就可以了，等到执行的时候将其重新替换上去就可以了（相当于恢复现场）。

### 恢复中断

回复中断的时候，就是要将这个0xcc取消掉，将原来的数值放上去，然后继续执行。ptrace一次是四个字节的修改，但是0xcc是一个字节，（八位），你就需要修改四字节的低位进行处理，然后替换的时候就是将其替换到低位（也就是末位），替换到低位是因为大部分机器是小端存储，等到一执行到这里的时候就会立马发生中断，等到恢复的时候，不仅要恢复现场（将修改的数值修改到原来的位置上），并且要将寄存器的PC指针 - 1（pc寄存器向上递增的，这里要将寄存器的数值 -1 就是0xcc之前的那个位置，然后继续执行，执行到下一个断点）

### 寻找相关信息

打断点的本质上是在机器码上进行打断点的，那么如何知道机器码和你写的代码之间的关系呢，有一个调试信息文件，也就是dwarf文件,下文中提到了。可以通过objdump这个工具反汇编得到的信息和dwarf文件上的信息对比一下，刚好是对应上的）函数名所在的机器码指令在哪，都可以找到，所以说在gdb上就可以直接进行在函数名上打断点了。

### 寻找C/C++函数和变量等，并且打断点

打断点的地方可以是地址，所以说你需要知道C变量中的变量的地址，所以就需要一定的调试信息 -> elf文件(编译和链接可执行文件)和Dwarf文件（调试信息文件

- 变量存放在在哪？
  - 可以在全局数据区，在栈内存上，寄存器上也有可能（立即数），具有相同名称的变量在不同的词法作用域中可能有不同的值，设置编译器可以直接将常数进行优化
  - 所以要全部捕捉他们呢。
- DIE中含有很多信息：
  - DW_TAG_subprogram: 用来描述函数（也称为子程序,main函数，或者其他函数不都是函数么，所以从中可以知道很多信息
  - DW_AT_name  : (...): main， 很显然，是这个名字，
  - DW_AT_low_pc： 函数的寄存器的入口
  - DW_AT_location: 变量在给定时刻在内存中的位置
  - .debug_line: 行号和机械码之间的映射

### 栈

系统调用的时候，自动会进行压栈出栈，系统调用栈大部分组成是：

```text
    High
        |   ...   |
        +---------+
     +24|  Arg 1  |
        +---------+
     +16|  Arg 2  |
        +---------+
     + 8| Return  |
        +---------+
EBP+--> |Saved EBP|
        +---------+
     - 8|  Var 1  |
        +---------+
ESP+--> |  Var 2  |
        +---------+
        |   ...   |
            Low
```

- 通过使用链表的形式，进行上下连接，（居然先是局部变量，然后才是参数列表）
- 栈指针是保存在`%rbp`指针上的

## 名词解释

- DIE: 调试信息表项 （Debugging Information Entry），每个DIE有一个标签 ——> 包含它的类型，以及一组属性。各个DIE之间通过兄弟和孩子结点互相链接，属性值可以指向其他的DIE。

## 参考资料

- [gdb教程](https://blog.tartanllama.xyz/writing-a-linux-debugger-setup/)
- [gdb中文教程](https://paper.seebug.org/2051/)
- [调试器工作原理](https://abcdxyzk.github.io/blog/2013/11/29/debug-debuger-3/)
