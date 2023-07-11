#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>


int main()
{
    int fd;
    char buf[100];
    fd = open("/dev/alina-dev",O_RDWR);
    if (fd < 0)
    {
        printf("Cannot open\n");
        return 0;
    }
    read(fd,buf,20);
    buf[20] = 0;
    printf("Input: >>> %s <<<\n",buf);
    close(fd);

    return 0;
}
