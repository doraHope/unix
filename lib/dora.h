
#include <stdio.h>  //C I/O库
#include <stdlib.h> //C标准库
#include <unistd.h> //unix标准库

#include <sys/stat.h>
#include <sys/types.h>

#include <sys/socket.h> //套接口库
#include <arpa/inet.h>  //
#include <netinet/in.h> //
#include <signal.h>     //信号

#include <fcntl.h> //文件流

#include <strings.h>
#include <string.h>

#include <errno.h>
#include <error.h>

#ifndef MAX_CHAR_LEN
#define MAX_CHAR_LEN 1024
#endif

#ifndef S_IFSOCK
#error S_IFSOCK not defined
#endif

typedef void Sigfunc(int);

//判断描述字是否为套接口描述字
int isfdtype(int fd, int fdtype)
{
    struct stat buf;
    if (fstat(fd, &buf) < 0)
    {
        return -1;
    }
    if ((buf.st_mode & S_IFMT) == fdtype)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

//read包装函数
ssize_t Read(int fd, void *vptr, size_t nbytes)
{
    int nread;
begin:
    if ((nread = read(fd, vptr, nbytes)) < 0)
    {
        //当读操作(阻塞)被中断时的处理
        if (errno == EINTR)
        {
            goto begin;
        }
        else
        {
            return -1;
        }
    }
    else if (nread == 0)
    {
        //读取到EOF
        return 0;
    }
    //读取字符末尾追加`\0`字符
    vptr += nread;
    vptr = 0;
    return nread;
}

//字节流操作
ssize_t readn(int fd, void *vptr, size_t nbytes)
{
    ssize_t nleft;  //目前待读取长度
    ssize_t nread;  //一次read读取的长度
    char *buffer;   //指向buffer指针
    buffer = (char *)vptr; 
    nleft = nbytes;
    while (nleft > 0)
    {
        if ((nread = read(fd, buffer, nleft)) < 0)
        {
            if (errno == EINTR)
            {
                nread = 0;
            }
            else
            {
                return -1;
            }
        }
        else if (nread == 0)
        {
            break; //EOF
        }
        nleft -= nread;
        buffer += nread;    //指针地址向后移动
    }
    return nbytes - nleft;
}

//捕获终端的写函数
ssize_t writen(int fd, const void *vptr, size_t nbytes)
{
    size_t nleft;
    ssize_t nwritlen;
    const char *buffer;
    buffer = (char *)vptr;
    nleft = nbytes;
    while (nleft > 0)
    {
        if ((nwritlen = write(fd, buffer, nleft)) <= 0)
        {
            if (errno == EINTR)
            {
                nwritlen = 0;
            }
            else
            {
                return -1;
            }
        }
        nleft -= nwritlen;
        buffer += nwritlen;
    }
    return nbytes - nleft;
}

//将读取内容缓存, 返回一行数据
ssize_t buffered_read(int fd, char *chr)
{
    static int read_cnt = 0;
    static char *read_ptr;
    static char read_buffer[MAX_CHAR_LEN];
    if (read_cnt <= 0)
    {
    again:
        if ((read_cnt = read(fd, read_buffer, MAX_CHAR_LEN)) < 0)
        {
            if (errno == EINTR)
            {
                goto again;
            }
            else
            {
                return -1;
            }
        }
        else if (read_cnt == 0)
        { //EOF
            return 0;
        }
        read_ptr = read_buffer;
    }
    *chr = *read_ptr++;
    return 1;
}

//读取指定长度字符串中的一行数据`以\n结尾`
ssize_t readline(int fd, void *vptr, size_t nbytes)
{
    int rc;    //my_read()返回值
    ssize_t i; //读取字符数
    char *buffer = (char *)vptr;
    char chr;
    for (i = 1; i < nbytes; i++)
    {
        if ((rc = buffered_read(fd, &chr)) == 1)
        {
            if (chr == '\r')
            { //兼容 \r\n 换行
                continue;
            }
            if (chr == '\n')
            {
                *buffer++ = chr;
                break;
            }
            *buffer++ = chr;
        }
        else if (rc == 0)
        {
            if (i == 1)
            {
                return 0;
            }
            else
            {
                break;
            }
        }
        else
        {
            return -1;
        }
    }
    *buffer = 0;
    return i;
}

//返回的是一个`void (*)(int)`类型的函数指针
Sigfunc *signal(int signo, Sigfunc *func)
{
    struct sigaction act, oact;
    act.sa_handler = func;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    if (SIGALRM == signo)
    {
#ifdef SA_INTERRUPT
        act.sa_flags |= SA_INTERRUPT;
#endif
    }
    else
    {
#ifdef SA_RESTART
        act.sa_flags |= SA_RESTART;
#endif
    }
    if (sigaction(signo, &act, &oact) < 0)
    {
        return SIG_ERR;
    }
    return oact.sa_handler;
}