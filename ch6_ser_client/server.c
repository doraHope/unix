//服务端程序(多进程版本 v1.0)
#include "../lib/dora.h"

//wait() 函数的缺陷是因为, wait()本身阻塞, 但是由于异常信号不会缓存, 所以可能遗漏对某些子进程资源的释放
void sig_chld(int signo)
{
    printf("who in\n");
    pid_t pid;
    int stat;
    pid = wait(&pid);
    printf("child %d terminated\n", pid);
}

void sig_chld(int signo)
{
    pid_t pid;
    int stat;
    while ((pid = waitpid(-1, &stat, WNOHANG)) > 0)
    {
        printf("child %d terminated\n", pid);
    }
    return;
}

int main(int argc, char **argv)
{
    const int BACKLOG_LEN = 1024;
    int sfd, cfd, sret;
    socklen_t len;
    struct sockaddr_in servaddr, cliaddr, localaddr, perraddr;
    char buffer[MAX_CHAR_LEN];
    char recv[MAX_CHAR_LEN];
    bzero(&buffer, 0);
    bzero(&recv, 0);
    sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd < 0)
    {
        printf("获取socket失败!\n");
        close(sfd);
        exit(0);
    }
    bzero(&servaddr, 0);
    bzero(&cliaddr, 0);
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(13);
    sret = bind(sfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    if (sret < 0)
    {
        printf("bind失败! %s\n", error);
        close(sfd);
        exit(0);
    }
    listen(sfd, BACKLOG_LEN);
    signal(SIGCHLD, sig_chld);
    while (1)
    {
        cfd = accept(sfd, (struct sockaddr *)&cliaddr, &len);
        if (cfd < 0)
        {
            if (EINTR == errno)
            {
                continue;
            }
            else
            {
                printf("accept error!\n");
            }
        }
        if (fork() == 0)
        {
            close(sfd);
            //打印客户端套接口字信息
            printf("connection from %s.port %d\n", inet_ntop(AF_INET, &(cliaddr.sin_addr.s_addr), buffer, MAX_CHAR_LEN), ntohs(cliaddr.sin_port));
            Read(cfd, recv, MAX_CHAR_LEN);
            printf(" from port `%d`, recv-line:%s", ntohs(cliaddr.sin_port), recv);
            writen(cfd, recv, sizeof(recv));
            close(cfd);
            exit(0);
        }
        else
        {
            close(cfd);
        }
    }
}
