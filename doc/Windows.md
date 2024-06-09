# Windows

这里是Windows相关的总结

## 调试的基本原理

`DEBUG_EVENT`会显示出具体的调试内容的信息，调试器通过`WaitForDebugEvent`函数获取调试信息，然后通过`ContinueDebugEvent(debugEvent.dwProcessId, debugEvent.dwThreadId, DBG_CONTINUE)`这样的格式继续将被调试的程序进行执行，自身程序会发布相关通知，将自身挂起，然后发送给调试器的，然后调试器进行相关命令的回复。调试信息主要是是放在了`DEBUG_EVENT` 这个结构体上了

## 异常

这里有点迷

## 调试符号

其实就是读取ELF文件中的相关信息，只不过微软上是在一个PDB文件中，这个PDB文件是单独生成的，跟产生出来的相关文件没有关联，比如说生成一个`a.exe`，那么在同等目录下就会生成`a.pdb`这样的文件，主要就是调试信息，比如说Windows上的pdb文件，哪里就会有相关的调试信息，如果需要调试这个程序，就需要链接一个Dll程序，这个程序是DbgHelper，

## 相关API

- 获取线程的上下文环境：`GetThreadContext()`，注意：要设置想要获取的寄存器数值数值