
# libredisae 
## 为什么创建这个项目
libredisae 是从 [Redis](https://github.com/antirez/redis) 所使用的事件驱动库，支持 Freebsd、Linux 等多个操作系统，只有1000 行左右的代码。其设计简洁，功能强大，故而在其他项目中也被广泛使用。比如，HTTP基准测试工具 [wrk](https://github.com/wg/wrk)。

我把这部分代码独立出来，建立了这个仓库，并提供了简单的示例程序。目的是希望能帮助大家快速理解这个库的设计原理，并掌握使用这个库的方法；从而在自己的项目使用。

## 示例程序
我们的示例程序是一个简单的UDP回声服务器，主要演示一下 libredisae 库的使用。实现了一下功能：

- 收包 监听指定端口，接收 udp 包，并统计接收到的字节数
- 回声 将接收到的数据返回给客户端
- 定时 每隔两秒打印报告接收到的字节数


## 美好的东西应该分享
美好的东西应该分享。希望你能喜欢！