#### 美杜莎（希腊语：Μέδουσα、英语：Medusa）是古希腊神话中的蛇发女妖，戈耳工三姐妹之一

##### 我不在意GDT和IDT到底有没有被修改 也不想重复process hacker已经有的功能 更不在乎某个进程连接了哪个IP

##### 重点关注进程 内存 线程 内核等重灾区 大多ARK为了功能足够多舍弃了一些不好维护或添加或不够稳定的功能 

##### 添加其他ARK等没有的更加激进功能成为主要方向(虚表钩子 指针替换扫描 IOCTL检查 回调钩子检查 虚拟化环境检查 进程断链检查 驱动断链检查 内存加载驱动检查 未知内存扫描 安全dump 更多并且更加强硬的注入方式 模仿反作弊或仿真检查 ....)

##### 在不加载PDB以及不加载驱动的情况下 保障一些基础功能的正常使用和运行 尽可能提供不依赖以上两样的最大支持


#### 已完成(应用层):

##### ---进程 线程 模块枚举列表

##### ---文件内存代码段对比(hook扫描)

##### ---应用层虚拟化检测(包括但不限于各类虚拟机以及虚拟化框架及驱动)

<h1 align="center">
	<img src="1.png" >
	<br>
	<br>
</h1>

#### 已完成(内核部分):

##### ---进程断链检查 一共4次检查 4次结果汇总 第三次以ObjectTable不为空为标志 第四次即便ObjectTable不为空也依旧加入列表并且标红 4次检查以后绝大部分进程断链方式都会被筛查出

###### -----应用层枚举第一次

###### -----内核调用ZwQuerySystemInformation枚举第二次

###### -----内核调用PsLookupProcessByProcessId枚举进程ID第三次检查

###### -----内核调用PsLookupThreadByThreadId枚举线程ID第四次检查

<h1 align="center">
	<img src="2.png" >
	<br>
	<br>
</h1>



##### 代码开源 已有代码的情况下我只缝合 基础功能齐全后上传release

##### 用来办培训开班的群 介意的就不要加了 729338597