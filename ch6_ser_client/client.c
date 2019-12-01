//回射函数-客服端v1.1(使用select 监听 可读的描述字 + 增强了对tcp读操作的处理)
#include "../lib/dora.h"

int contract1(int cfd, int ifd)
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

int contract2(int cfd, int ifd)
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
        fd_set read_set;
        FD_ZERO(&read_set);
        FD_SET(ifd, &read_set);
        FD_SET(cfd, &read_set);
        int maxfd = max(ifd, cfd) + 1;
        int fret = 0;
        Select(maxfd, &read_set, NULL, NULL, NULL);
        if (FD_ISSET(cfd, &read_set))
        {
            if (0 == (fret = Read(cfd, buffer, MAX_CHAR_LEN)))
            {
                return;
            }
            if (fret < 0)
            {
                printf("read fail! error:`%d`, error:`%s`\n", errno, error);
            }
            printf("server-ret:%s", buffer);
        }
        if (FD_ISSET(ifd, &read_set))
        {
            if (0 == (fret = Read(ifd, recv, MAX_CHAR_LEN)))
            {
                return; //EOF
            }
            if (fret < 0)
            {
                printf("read fail! error:`%d`, error:`%s`\n", errno, error);
            }
            if (0 > writen(cfd, recv, strlen(recv)))
            {
                printf("contract2 write fail! errno:`%d`, error:`%s`\n", errno, error);
            }
        }
    }
}

int contract3(int cfd, int ifd)
{
    int rcnt = 0;
    int ofd = 1;
    char buffer[MAX_CHAR_LEN];
    char recv[MAX_CHAR_LEN];
    // char dt[] = "haaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaahahahahahahahahahahahahahahahahahahahahahhaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaahahahahahahahahahahahahahahahahahahahahahhaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaahahahahahahahahahahahahahahahahahahahahahhaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaahahahahahahahahahahahahahahahahahahahahahhaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaahahahahahahahahahahahahahahahahahahahahahhaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaahahahahahahahahahahahahahahahahahahahahahhaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaahahahahahahahahahahahahahahahahahahahahahhaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaahahahahahahahahahahahahahahahahahahahahahhaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaahahahahahahahahahahahahahahahahahahahahahhaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaahahahahahahahahahahahahahahahahahahahahahhaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaahahahahahahahahahahahahahahahahahahahahahhaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaahahahahahahahahahahahahahahahahahahahahahhaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaahahahahahahahahahahahahahahahahahahahahahhaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaahahahahahahahahahahahahahahahahahahahahahhaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaahahahahahahahahahahahahahahahahahahahahahhaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaahahahahahahahahahahahahahahahahahahahahahhaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaahahahahahahahahahahahahahahahahahahahahahhaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaahahahahahahahahahahahahahahahahahahahahahhaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaahahahahahahahahahahahahahahahahahahahahahhaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaahahahahahahahahahahahahahahahahahahahahahhaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaahahahahahahahahahahahahahahahahahahahahahhaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaahahahahahahahahahahahahahahahahahahahahahhaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaahahahahahahahahahahahahahahahahahahahahahhaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaahahahahahahahahahahahahahahahahahahahahahhaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaahahahahahahahahahahahahahahahahahahahahahhaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaahahahahahahahahahahahahahahahahahahahahahhaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaahahahahahahahahahahahahahahahahahahahahahhaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaahahahahahahahahahahahahahahahahahahahahahhaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaahahahahahahahahahahahahahahahahahaahahahahahahhaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaahahahahahahahahahahahahahahahahahahahahah\n";
    int flag_input_end = 0;
    while (1)
    {
        memset(&recv, 0, MAX_CHAR_LEN * sizeof(char));
        memset(&buffer, 0, MAX_CHAR_LEN * sizeof(char));
        fd_set read_set;
        FD_ZERO(&read_set);
        FD_SET(ifd, &read_set);
        FD_SET(cfd, &read_set);
        int maxfd = max(ifd, cfd) + 1;
        int fret = 0;
        Select(maxfd, &read_set, NULL, NULL, NULL);
        if (FD_ISSET(cfd, &read_set))
        {
            if (0 == (fret = Read(cfd, buffer, MAX_CHAR_LEN)))
            {
                if (1 == flag_input_end)
                {
                    //正常情况下关闭的状态
                    printf("slef-close: read fin!\n");
                    return 0;
                }
                else
                {
                    //服务器关闭的状态
                    printf("type end!");
                    sys_quit("server closed!");
                }
            }
            if (fret < 0)
            {
                printf("read fail! error:`%d`, error:`%s`\n", errno, error);
            }
            printf("server-ret:%s", buffer);
        }
        if (FD_ISSET(ifd, &read_set))
        {
            if (0 == (fret = Read(ifd, recv, MAX_CHAR_LEN)))
            {
                printf("input end\n");
                flag_input_end = 1;
                Shutdown(cfd, SHUT_WR);
                // FD_CLR(ifd, &read_set);
                continue;
            }
            if (fret < 0)
            {
                printf("read fail! error:`%d`, error:`%s`\n", errno, error);
            }
            /*------------------------------------ 服务端 进行测试*/
            // int i = 0;
            // for(; i < 10; i++) {
            //     if (0 > writen(cfd, dt, strlen(dt)))
            //     {
            //         printf("contract write fail! errno:`%d`, error:`%s`\n", errno, error);
            //     }
            // }
            /*--- */
            if (0 > writen(cfd, cfd, strlen(dt)))
            {
                printf("contract write fail! errno:`%d`, error:`%s`\n", errno, error);
            }
        }
    }
    return 0;
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
    contract3(cfd, ifd);
    close(cfd);
}