# diff

总结一下Windows和Linux相关的差异，并且系统总结一下

## 调试的基本原理

- `DEBUG_EVENT`会显示出具体的调试内容的信息，调试器通过`WaitForDebugEvent`函数获取调试信息，然后通过`ContinueDebugEvent(debugEvent.dwProcessId, debugEvent.dwThreadId, DBG_CONTINUE)`这样的格式继续将被调试的程序进行执行，自身程序会发布相关通知，将自身挂起，然后发送给调试器的，然后调试器进行相关命令的回复。调试信息主要是是放在了`DEBUG_EVENT` 这个结构体上了
- 也就是说Windows上主要是通过Windows API来进行调试，
- 但是Linux上，主要就是ptrace这个函数进行调试的，基本上啥都能做了

## 相关符号的调用

- 微软的相关调试符号是在PDB文件中的，这个PDB文件是单独生成的，跟产生出来的相关文件没有关联，比如说生成一个`a.exe`，那么在同等目录下就会生成`a.pdb`这样的文件，主要就是调试信息，比如说Windows上的pdb文件，哪里就会有相关的调试信息，如果需要调试这个程序，就需要链接一个Dll程序，这个程序是DbgHelper.dll，并且根据这个dll文件，获取到相关代码的差异
- Linux上主要是是放在Dwarf文件和ELF文件中，需要专门的文件对其进行解析，解析相关相关的程序，然后使用他们，

## 设置断点

- 具体参考这篇我写的文章：[gdb调试的原理](https://www.zhihu.com/question/578172542/answer/3389041105)，Linux上和Windows上，都是Int3断点，也就是0xcc，只不过设置的时候调用的API不一样，
  - Windows上的Api主要是`GetThreadContext`和`SetThreadContext`这些函数，`ReadProcessMemory`和`WriteProcessMemory`这样的函数
  - Linux主要是还是`ptrace`这个函数，第三个参数是一个void*，根据传入的参数，来获得对应的东西，感觉比较万能，但是也太多了

