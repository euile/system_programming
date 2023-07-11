#include <linux/module.h>      // Заголовочный файл для создания модулей ядра
#include <linux/kernel.h>      // Заголовочный файл для работы с ядром
#include <linux/fs.h>          // Заголовочный файл для работы с файловой системой
#include <linux/uaccess.h>
#include <linux/device.h>
#include <asm/uaccess.h>
#include <linux/circ_buf.h>
MODULE_LICENSE("GPL");


#define DEVICE_NAME "alina-dev"  // Имя символьного устройства
#define SUCCESS 0

static int device_open(struct inode *inode, struct file *file);
static int device_release(struct inode *inode, struct file *file);
static ssize_t device_read(struct file *filp, char *buffer, size_t length, loff_t *offset);
static ssize_t device_write(struct file *filep, const char *buffer, size_t len, loff_t *offset);  /*User->Kernel*/
static int Major; /* Старший номер, присвоенный драйверу устройства */


static struct file_operations fops = {
    .open = device_open,
    .release = device_release,
    .read = device_read,
    .write = device_write
};

static char message[] = "Hello!";  // Строка, которую будет выдавать драйвер
static int lenght = sizeof(message) - 1;  // Размер строки без учета завершающего символа '\0'
static char *message_ptr = message;  // Указатель на текущую позицию в строке
static int ind_write;
static int ind_read;
static int BUF_LEN = 10;

static int device_open(struct inode *inode, struct file *file)
{
    static int counter = 0;
    /*if (Device_Open)
        {return -EBUSY;} 
    Device_Open++;*/
    sprintf(message, "I already told you %d times Hello world!\n", counter++);
    message_ptr = message;
    try_module_get(THIS_MODULE);

    return SUCCESS;
}

static int device_release(struct inode *inode, struct file *file)
{
    /*Device_Open--;*/ /* We're now ready for our next caller */
    /** Decrement the usage count, or else once you opened the file, you'll
    never get get rid of the module. */
    printk(KERN_INFO "Device successfully closed\n");
    module_put(THIS_MODULE);
    return 0;
}

static ssize_t device_read(struct file *filp, char *buffer, size_t length, loff_t *offset) /*Kernel->User*/
{
    /*
    int bytes_copied = copy_to_user (buffer, const message, lenght);
    // копирует данные из пространства ядра в пространство пользователя 
    if (bytes_copied == 0)
    {
        message_ptr = message;
    }
    return bytes_copied;
    */
    ind_read = 0;
    int bytes_read = 0;
    while (length && *message_ptr) {
        put_user(*(message_ptr++), buffer++);  // Write a simple value into user space. 
        //printk(KERN_INFO "Message ptr %c\n", *message_ptr);
        length--;
        bytes_read++;
        ind_read++;
        ind_read %= lenght;
    }
    if (bytes_read == 0) {
        // Достигнут конец строки, возвращаем 0, чтобы указать конец чтения
        message_ptr = message;  // Сбрасываем указатель на начало строки
       
    }
    return bytes_read; 
}

static ssize_t device_write(struct file *filep, const char *buffer, size_t len, loff_t *offset)  /*User->Kernel*/
{
    ind_write = 0;
    int bytes_write = 0;
    while (lenght && *message_ptr) {
        get_user(message_ptr[ind_write], buffer++);  
        printk(KERN_INFO "Message ptr %c\n", *message_ptr);
        lenght--;
        bytes_write++;
        ind_write++;
        ind_write %= BUF_LEN;
    }
  
    lenght = strlen(message);                
    printk(KERN_INFO "Received %zu characters from the user\n", len);
    return len;
}

int init_module()
{
    Major = register_chrdev(0, DEVICE_NAME, &fops);
    if (Major < 0) {
        printk(KERN_ALERT "Registering char device failed with %d\n", Major);
        return Major;
    }
    printk(KERN_INFO "The device %s was registered\n", DEVICE_NAME);
    printk(KERN_INFO "To talk to the driver, create a dev file with\n");
    printk(KERN_INFO "'mknod /dev/%s c %d 0'.\n", DEVICE_NAME, Major);
    printk(KERN_INFO "ALINA!!!!!!!!!!!!!!!!!!!\n");

    return 0;
}
void cleanup_module()
{
    unregister_chrdev(0, DEVICE_NAME);
    printk(KERN_INFO "The device %s was unregistered\n", DEVICE_NAME);

}
