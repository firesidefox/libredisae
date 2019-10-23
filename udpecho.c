/*
 * Copyrigin(c) 2019 firesidefox. All rights reserved.
 */
/* simple example for libredisae */
#include <stdio.h>
#include <unistd.h>
#include <malloc.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <time.h>
#include <assert.h>

#include "ae.h"

#define MAX_BUFF_SIZE   1500

typedef struct request {
    struct sockaddr addr;
    size_t size;
    char buff[MAX_BUFF_SIZE];
} request;

typedef struct state {
    unsigned long long recv_bytes;
    request *r;
} state;

static void
udp_echo_handler(struct aeEventLoop *loop, int fd, void *clientData, int mask)
{
    state *s = (state *)clientData;

    if(s->r) {
        sendto(fd, s->r->buff, s->r->size, 0, &s->r->addr, sizeof(s->r->addr));
        free(s->r);
        s->r = NULL;
    }
    
    aeDeleteFileEvent(loop, fd, mask);
}

static void
udp_wait_request_handler(struct aeEventLoop *loop, int fd, void *clientData, int mask)
{
    request *r;
    int ret;
    socklen_t socklen;
    state *s = (state *)clientData;

    r = malloc(sizeof(request));
    if(!r) {
        perror("malloc failed\n");
        return ;
    }
    socklen = sizeof(r->addr);
    ret = recvfrom(fd, r->buff, MAX_BUFF_SIZE, 0, &r->addr, &socklen);
    if (ret < 0) {
        // 发生错误，直接退出
        printf("read failed, errno = %d(%s)\n", errno, strerror(errno));
        aeStop(loop);
        return;
    }
    
    // C 类字符串，以'\0'结尾
    r->buff[ret] = '\0';
    r->size = ret;
    s->recv_bytes += ret;

    assert(s->r == NULL);
    s->r = r;
    
    aeCreateFileEvent(loop, fd, AE_WRITABLE, udp_echo_handler, s);
}

static int
timer_report_handler(struct aeEventLoop *eventLoop, long long id, void *clientData)
{
    state *s = (state *)clientData;
    printf("timestamp %ld | recv %llu bytes\n", time(NULL), s->recv_bytes);
    return (2 * 1000);
}

static void
timer_report_finalizer(struct aeEventLoop *eventLoop, void *clientData) {
    state *s = (state *)clientData;
    printf("timer_report_handler, timestamp %ld, recv %llu bytes \n",
        time(NULL), s->recv_bytes);
}

state gstate = {0};

int main(int argc, char *argv[])
{
    aeEventLoop *loop;
    long long report_timer_id;
    int fd, ret;
    

    struct sockaddr_in saddr = {0};

    if(argc != 2) {
        printf("Usage: %s <port> \n", argv[0]);
        return -1;
    }

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(fd < 0) {        
        perror("create socket failed!\n");
        return -1;
    }

    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    saddr.sin_port = htons(atoi(argv[1]));

    ret = bind(fd, (struct sockaddr *)&saddr, sizeof(saddr));
    if(ret < 0) {
        perror("socket bind fail!\n");
        return -1;
    }

    // 创建 EventLoop 对象
    loop = aeCreateEventLoop(1024);
    if (!loop)
    {
        printf("create event loop error\n");
        goto err;
    }

    /* 添加文件IO事件 */
    ret = aeCreateFileEvent(loop, fd,
        AE_READABLE, udp_wait_request_handler, (void *)&gstate);
    if(ret) {
        printf("create FileEvent error\n");
        goto err;
    }

    // 添加定时器事件,  每隔2秒打印流量信息
    report_timer_id = aeCreateTimeEvent(loop, 2 * 1000,
        timer_report_handler, (void *)&gstate, timer_report_finalizer);
    if (report_timer_id < 0)
    {
        printf("create TimeEvent error\n");
        goto err;
    }

    // 进入事件循环
    aeMain(loop);

    close(fd);
    aeDeleteEventLoop(loop);

    return 0;

err:
    close(fd);
    if(loop) {
        aeDeleteEventLoop(loop);
    }
    return -1;
}