本文件夹为分布式温控计费系统的工程源文件和一些源码相关的说明文件。

目录结构：
源文件
|
|----AC_Server:空调服务器工程源码，提供了3个版本供选择，1min/1s/500ms为温度金额刷新间隔，请根据自己的需要选择使用
|	|
|	|----AC_Server_1min
|	|----AC_Server_1s
|	|----AC_Server_500ms
|
|----AC_Client:空调客户端工程源码，同样提供了3个版本供选择，在选择时请与服务器的温度金额刷新间隔保持对应
|	|
|	|----AC_Client_1min
|	|----AC_Client_1s
|	|----AC_Client_500ms
|
|----数据库相关:包含数据库账号密码在源码中的位置，需要的可以自行修改，编译。还有QT编译所需的数据库驱动，下面有详细说明
|	|
|	|----数据库参数.png
|	|----QT数据库驱动
|		|----libmysql.dll
|		|----libqsqlmysql.a
|		|----qsqlmysql.dll
|		|----qsqlmysql.dll.debug
|
|----Readme.txt:本说明文件
|

电脑硬件配置：
处理器：Intel i7 7700HQ
显卡：NVIDIA GeForce GTX 1050 Ti
内存：16GB

编程语言：
C++

QT：
Qt Creator 4.11.1
Based on Qt 5.14.1 (MSVC 2017, 32 bit)

编译器：
Desktop Qt 5.14.2 MinGW 7.3.0 64-bit

数据库：
mysql  Ver 8.0.20 for Win64 on x86_64 (MySQL Community Server - GPL)

其它说明：
1、关于QT数据库驱动
libmysql.dll和libqsqlmysql.a放入【...\Qt5.14.2\5.14.2\mingw73_64\bin】下
qsqlmysql.dll和qsqlmysql.dll.debug放入【...\Qt5.14.2\5.14.2\mingw73_64\plugins\sqldrivers】下
2、由于QT的限制，编译构建时路径不允许有中文
3、关于设计
由于在写动态模型和静态模型时没有进行编码，在真正编码时发现对部分逻辑不合理，难以实现的部分进行了修改，但总体设计依然是完全依照动态模型和静态模型的指导。调度策略也是基于老师测调度策略，进行了优化和改进，实现了长期较为公平的调度策略。