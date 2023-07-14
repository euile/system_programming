#ifndef DRIVER_H
#define DRIVER_H
#include <linux/ioctl.h>
#define MAGIC_NUM 0xF1
#define IOCTL_GET _IOR(MAGIC_NUM, 0, int)
/* Эта IOCTL используется для вывода с целью получить сообщение  
 * драйвера устройства. Однако нам все равно нужен буфер для размещения 
 * этого сообщения в качестве ввода при его передаче процессом. 
 */ 
#define IOCTL_RESET _IO (MAGIC_NUM, 1)
/*
Имя файла устройства.
#define DEVICE_FILE_NAME "char_dev" 
#define DEVICE_PATH "/dev/char_dev" 
*/

#endif // DRIVER_H
