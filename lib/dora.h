
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
#include <math.h>

#include <strings.h>    
#include <string.h>

#include <errno.h>
#include <error.h>

#include <sys/select.h> //IO复用

#ifndef MAX_CHAR_LEN
#define MAX_CHAR_LEN 1024
#endif

#ifndef S_IFSOCK
#error S_IFSOCK not defined
#endif

#ifndef FD_SETSIZE
#define FD_SETSIZE 256
#endif

typedef void Sigfunc(int);

//程序终止函数
void err_sys(const char * buffer) 
{
    printf("sys_error: `%s` errno:`%d`, error:`%s`\n", buffer, errno, error);
    exit(1);
}

void sys_quit(const char *buffer)
{
    printf("sys_stop: `%s` \n", buffer);
    exit(0);
}

int max(int a, int b) {
    return a >= b ? a : b;
}

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

ssize_t Read(int fd, void *vptr, size_t nbytes)
{
    int nread;
begin:
    if ((nread = read(fd, vptr, nbytes)) < 0)
    {
        if (errno == EINTR)
        {
            goto begin;
        }
        else
        {
            return -1;
        }
    }
    vptr += nread;
    vptr = 0;
    return nread;
}

    //字节流操作
ssize_t readn(int fd, void *vptr, size_t nbytes)
{
    ssize_t nleft;
    ssize_t nread;
    char *buffer;
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
            printf("base-read:`%s`\n", buffer);
            break; //EOF
        }
        nleft -= nread;
        buffer += nread;
    }
    printf("bytes:`%d`, pos:`%d`\n", nbytes, nleft);
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

//进程信号异步通知处理函数
Sigfunc * signal(int signo, Sigfunc * func)
{   
    struct sigaction act, oact;
    act.sa_handler = func;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    if(SIGALRM == signo) {
#ifdef SA_INTERRUPT
        act.sa_flags |= SA_INTERRUPT;
#endif
    } else {
#ifdef SA_RESTART
        act.sa_flags |= SA_RESTART;
#endif
    }
    if(sigaction(signo, &act, &oact) < 0) {
        return SIG_ERR;
    }
    return oact.sa_handler;
}

int Select(int maxfd, fd_set * readset, fd_set * writeset, fd_set * exceptset, struct timeval * timeout)
{

    if(select(maxfd, readset, writeset, exceptset, timeout) < 0) {
        err_sys("[select] 发生错误");
    }
    return 1;
}

int Shutdown(int fd, int howto)
{
    if(0 > shutdown(fd, howto)) {
        printf("shutdown fail! errno:`%d`, error:`%s`\n", errno, error);
    }
    return 0;
}