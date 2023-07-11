#include <linux/module.h>      // Заголовочный файл для создания модулей ядра
#include <linux/kernel.h>      // Заголовочный файл для работы с ядром
#include <linux/fs.h>          // Заголовочный файл для работы с файловой системой
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");


#define DEVICE_NAME "alina-dev"  // Имя символьного устройства
#define SUCCESS 0

static int device_open(struct inode *inode, struct file *file);
static int device_release(struct inode *inode, struct file *file);
static ssize_t device_read(struct file *filp, char *buffer, size_t length, loff_t *offset);
static int Major; /* Старший номер, присвоенный драйверу устройства */


static struct file_operations fops = {
    .open = device_open,
    .release = device_release,
    .read = device_read
};

static char message[] = "Hello!";  // Строка, которую будет выдавать драйвер
int message_size = sizeof(message) - 1;  // Размер строки без учета завершающего символа '\0'
static char *message_ptr = message;  // Указатель на текущую позицию в строке

static int device_open(struct inode *inode, struct file *file)
{
    return 0;
}

static int device_release(struct inode *inode, struct file *file)
{
    return 0;
}

static ssize_t device_read(struct file *filp, char *buffer, size_t length, loff_t *offset)
{
    int bytes_read = 0;

    while (length && *message_ptr) {
        put_user(*(message_ptr++), buffer++);
        //printk(KERN_INFO "Message ptr %c\n", *message_ptr);
        length--;
        bytes_read++;
    }

    if (bytes_read == 0) {
        // Достигнут конец строки, возвращаем 0, чтобы указать конец чтения
        message_ptr = message;  // Сбрасываем указатель на начало строки
    }

    return bytes_read;
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
