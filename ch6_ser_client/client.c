#include "dora.h"

int contract(int cfd, int ifd)
{
    int rcnt = 0;
    int ofd = 1;
    char buffer[MAX_CHAR_LEN];
    char recv[MAX_CHAR_LEN];
    //首先读取
    while (1)
    {
        bzero(&buffer, 0);
        bzero(&recv, 0);
        if ((rcnt = Read(ifd, buffer, MAX_CHAR_LEN)) >= 0)
        {
            if (0 == rcnt)
            {
                printf("input line end!\n");
                return; //结束符
            }
            printf("len:`%d`, cli-input:%s", strlen(buffer), buffer);
            writen(cfd, buffer, strlen(buffer));
            if (0 == (rcnt = Read(cfd, recv, MAX_CHAR_LEN)))
            { //FIN
                printf("server send FIN\n");
                return;
            }
            printf("server-ret: read:%s", recv);
        }
        else
        {
            printf("read failed!\n");
            return;
        }
    }
}

int main(int argc, char **argv)
{
    int cfd, ifd;
    int cret;
    ifd = 0;
    socklen_t len;
    struct sockaddr_in client;
    char buffer[1024];
    bzero(&client, 0);
    client.sin_family = AF_INET;
    client.sin_port = htons(13);
    if (inet_pton(AF_INET, argv[1], &client.sin_addr) < 0)
    {
        printf("创建socket失败, error:`%s`", error);
        exit(0);
    }
    cfd = socket(AF_INET, SOCK_STREAM, 0);
    if (cfd < 0)
    {
        printf("创建socket失败, error:`%s`", error);
        exit(0);
    }
    if ((cret = connect(cfd, (struct sockaddr *)&client, sizeof(client))) < 0)
    {
        printf("连接失败, error:`%s`", error);
        exit(0);
    }
    contract(cfd, ifd);
    close(cfd);
}