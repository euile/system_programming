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
#define BUFFER_SIZE 1024

/*
static int init_module();
static void cleanup_module();
*/
static int device_open(struct inode *inode, struct file *file);
static int device_release(struct inode *inode, struct file *file);
static ssize_t device_read(struct file *filp, char *buffer, size_t length, loff_t *offset);
static ssize_t device_write(struct file *filep, const char *buffer, size_t len, loff_t *offset);  /*User->Kernel*/
static int Major; /* Старший номер, присвоенный драйверу устройства */
static int Device_Open = 0;


static struct file_operations fops = {
    .open = device_open,
    .release = device_release,
    .read = device_read,
    .write = device_write
};

//static char message[BUF_LEN];  // Строка, которую будет выдавать драйвер
static int ind_write;
static int ind_read;
static char *device_buffer;
static int buffer_head = 0;
static int buffer_tail = 0;
static int buffer_count = 0;

static int device_open(struct inode *inode, struct file *file)
{
    static int counter = 0;
    
    if (Device_Open)
        {return -EBUSY;} 
    Device_Open++;
    
    
    //sprintf(message, "I already told you %d times Hello world!\n", counter++);
    //message_ptr = message;
    try_module_get(THIS_MODULE);

    return SUCCESS;
}

static int device_release(struct inode *inode, struct file *file)
{
    Device_Open--;
    /* We're now ready for our next caller */
    /** Decrement the usage count, or else once you opened the file, you'll
    never get get rid of the module. */
    printk(KERN_INFO "Device successfully closed\n");
    module_put(THIS_MODULE);
    return 0;
}

static ssize_t device_read(struct file *filp, char *buffer, size_t length, loff_t *offset) /*Kernel->User*/
{
    
    ssize_t bytes_copied = 0;
    
    // Проверяем, что буфер не пустой
    if (buffer_count == 0)
        return 0;
    
    // Определяем количество байтов, которые можно прочитать
    bytes_copied = min(length, (size_t)buffer_count);
    
    // Копируем данные из буфера ядра в пространство пользователя
    if (copy_to_user(buffer, &device_buffer[buffer_tail], bytes_copied) != 0)
        return -EFAULT;
    
    // Обновляем указатель на начало буфера и количество байтов в буфере
    buffer_tail = (buffer_tail + bytes_copied) % BUFFER_SIZE;
    buffer_count -= bytes_copied;

    printk(KERN_INFO "1) %d, 2) %d\n", buffer_head - buffer_count, lenght - 1);
    wait_event_interruptible(wq, buffer_head - buffer_count < lenght - 1 );
    
    return bytes_copied;
}

static ssize_t device_write(struct file *file, const char __user *buffer, size_t length, loff_t *offset)
{

    ssize_t bytes_copied = 0;
    
    // Проверяем, что длина записываемых данных не превышает размер буфера
    if (length > BUFFER_SIZE - buffer_count)
        return -EINVAL;
    
    // Копируем данные из пространства пользователя в буфер ядра
    bytes_copied = copy_from_user(&device_buffer[buffer_head], buffer, length);
    
    // Обновляем указатель на начало буфера и количество байтов в буфере
    buffer_head = (buffer_head + length) % BUFFER_SIZE;
    buffer_count += length - bytes_copied;
    wake_up_interruptible(&wq);
    return length - bytes_copied;
 
}

/*User->Kernel*/

int init_module()
{
    // Выделяем память для буфера
    device_buffer = kmalloc(BUFFER_SIZE, GFP_KERNEL);
    if (!device_buffer)
        return -ENOMEM;
    
    // Инициализируем переменные
    buffer_head = 0;
    buffer_tail = 0;
    buffer_count = 0;

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
    // Удаляем символьное устройство
    unregister_chrdev(Major, DEVICE_NAME);
    
    // Освобождаем память, выделенную для буфера
    kfree(device_buffer);

    printk(KERN_INFO "The device %s was unregistered\n", DEVICE_NAME);

}
