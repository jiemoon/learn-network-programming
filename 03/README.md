## 使用 Socket 非阻塞 + Epoll 事件驱动模型实现并发的网络通信

### Usage

> epoll.h need Linux

#### start service

```bash
gcc -o service service.c && ./service
```

#### start client

```bash
gcc -o client client.c && ./client 127.0.0.1
```
