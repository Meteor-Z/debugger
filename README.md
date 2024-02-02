# debugger

简易的C/C++ debugger软件

## 前提库

需要的两个库

- [linenoise](https://github.com/antirez/linenoise)
- [libelfin ](https://github.com/TartanLlama/libelfin/tree/fbreg)
  - 目前还没有找到好用的库，这个库不支持最新版本的`DWARF 5`，所以调试的是一定要加入参数 `-gdwarf-3` or `gdwarf-4`
  - 安装的时候切换到这个`fbreg`的分支
  - 又发现了新问题，gcc13.2生成的版本很多都是`DWARF 5`, 导致直接寄了，这里还是直接使用 gcc 11版本吧（ubuntu 22.04）


## 参考资料

- [gdb教程](https://blog.tartanllama.xyz/writing-a-linux-debugger-setup/)
- [gdb中文教程](https://paper.seebug.org/2051/)
- [调试器工作原理](https://abcdxyzk.github.io/blog/2013/11/29/debug-debuger-3/)

## 原理

### ptrace

基本上依赖于这个函数`ptrace`，

- `long ptrace(request, pid_t pid, void *addr, void *data)`
  - request: 宏定义，比如说`PTRACE_TRACEME`, `PTRACE_PEEKDATA`等，表示一个动作
  - pid: 要调试的进程号
  - addr: 要写入的地址
  - data: 写入地址的话 data就是数据，读出地址的话，就会赋值给data。

### 中断原理

有一条汇编语言 INT 3(interrupt 3)的缩写，将其插入到代码中，就会发生中断，然后父进程就可以使用`ptrace()`进行操纵相关的数据，在原来的代码上进行修改代码。插入0xcc(INT 3的数字)，然后要将原来的信息进行保留，运行之后才会再放回原位，

### 寻找C/C++函数和变量等，并且打断点

打断点的地方可以是地址，所以说你需要知道C变量中的变量的地址，所以就需要一定的调试信息 -> elf文件(编译和链接可执行文件)和Dwarf文件（调试信息文件

- 变量存放在在哪？
  - 可以在全局数据区，在栈内存上，寄存器上也有可能（立即数），具有相同名称的变量在不同的词法作用域中可能有不同的值
- DIE中含有很多信息：
  - DW_TAG_subprogram: 用来描述函数（也称为子程序,main函数，或者其他函数不都是函数么，所以从中可以知道很多信息
    - DW_AT_name  : (...): main， 很显然，是这个名字，
    - DW_AT_low_pc： 函数的寄存器的入口    
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


