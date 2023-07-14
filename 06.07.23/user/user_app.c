#include "../driver/driver.h"
#include <sys/types.h>
#include <fcntl.h> /* Открытие. */ 
#include <unistd.h> /* Закрытие. */ 
#include <stdio.h>
#include <sys/ioctl.h>
#include <stdio.h> /* Стандартный ввод-вывод. */ 


int main()
{
    int fd, cmd;
    printf("\nOpening Driver\n");
    fd = open("/dev/alina-dev6", O_RDWR);
    if(fd < 0) {
        printf("Cannot open device file...\n");
        return 0;
    }
 
    ioctl(fd, IOCTL_RESET);
    printf("Reading Value from Driver\n");
    ioctl(fd, IOCTL_GET, &cmd);
 
    printf("Closing Driver\n");
    close(fd);

    return 0;
}

