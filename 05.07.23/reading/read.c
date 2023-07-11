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
    int readed_bytes = read(fd, buf, 100);
    buf[100] = 0;
    printf("Readed %d bytes\n", readed_bytes);
    printf("%s\n", buf);
    close(fd);

    return 0;
}
