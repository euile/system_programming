#include <linux/module.h>      // Заголовочный файл для создания модулей ядра
#include <linux/kernel.h>      // Заголовочный файл для работы с ядром
#include <linux/fs.h>          // Заголовочный файл для работы с файловой системой
#include <linux/uaccess.h>
#include <linux/device.h>
#include <asm/uaccess.h>
#include <linux/circ_buf.h>
#include <asm/atomic.h>
#include <linux/ioport.h>
#include <linux/wait.h>
#include <linux/kthread.h>
#include <asm/io.h>
#include <linux/irq.h> 
#include <linux/delay.h> 
#include <linux/poll.h> 
#include "driver.h"
MODULE_LICENSE("GPL");


#define DEVICE_NAME "alina-dev6"  // Имя символьного устройства
#define SUCCESS 0
#define BUFFER_SIZE 1024

static enum { 
    CDEV_NOT_USED = 0, 
    CDEV_EXCLUSIVE_OPEN = 1, 
}; 
 
/* Открыто ли сейчас устройство? Служит для предотвращения 
 * конкурентного доступа к одному устройству.
 */ 
static atomic_t already_open = ATOMIC_INIT(CDEV_NOT_USED); 

static atomic_t my_cnt; // Объявление атомарного счетчика

//ret = kernel_thread (mykthread,NULL,CLONE_FS|CLONE_FILES,CLONE_SIGHAND,SIGCHLD)

static struct task_struct *ts;
int thread(void *data) {
    while(1) 
    {
        printk("Hello. I am kernel thread! \n");
        atomic_inc(&my_cnt); // инкрментируем счетчик в потоке ядра
        msleep(100);
        if (kthread_should_stop())
            break;
    }
    return 0;
}

static long device_ioctl(struct file *filp, unsigned int cmd,unsigned long arg);
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
    .write = device_write,
    .unlocked_ioctl = device_ioctl  // Реализация IOCTL
};

//static char message[BUF_LEN];  // Строка, которую будет выдавать драйвер
static int ind_write;
static int ind_read;
static char *device_buffer;
static int buffer_head = 0;
static int buffer_tail = 0;
static int buffer_count = 0;

static long device_ioctl(struct file *file, /* То же самое. */ 
             unsigned int cmd, /* Число и параметр для ioctl */ 
             unsigned long arg) 
{ 
    int ret = SUCCESS; 
 
    /* Мы не хотим взаимодействовать с двумя процессами одновременно */ 
    if (atomic_cmpxchg(&already_open, CDEV_NOT_USED, CDEV_EXCLUSIVE_OPEN)) 
        return -EBUSY; 
 
    /* Переключение согласно вызванной ioctl. */ 
    switch (cmd)
    { 
    case IOCTL_RESET: { 
        /* Получение указателя на сообщение (в пользовательском  
         * пространстве) и установка его как сообщения устройства. 
         * Получение параметра, передаваемого ioctl процессом. 
         */ 
        atomic_set(&my_cnt, 0 );
        break; 
    } 
    case IOCTL_GET: { 
        if (copy_to_user(arg, &my_cnt, sizeof(atomic_t)) != 0)
        {
            return -EFAULT;
        }  // куда что сколько
        break;
    } 
 
    /* Теперь можно принимать следующий вызов. */ 
    atomic_set(&already_open, CDEV_NOT_USED); 
    }
    return ret; 
}
 

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

static ssize_t device_read(struct file *filp, char __user *buffer, size_t lenght, loff_t *offset) /*Kernel->User*/
{
    
    ssize_t bytes_copied = 0;
    
    // Проверяем, что буфер не пустой
    if (buffer_count == 0)
        return 0;
    
    // Определяем количество байтов, которые можно прочитать
    bytes_copied = min(lenght, (size_t)buffer_count);
    
    // Копируем данные из буфера ядра в пространство пользователя
    if (copy_to_user(buffer, &device_buffer[buffer_tail], bytes_copied) != 0)  // куда что сколько
        return -EFAULT;
    
    // Обновляем указатель на начало буфера и количество байтов в буфере
    buffer_tail = (buffer_tail + bytes_copied) % BUFFER_SIZE;
    buffer_count -= bytes_copied;

    printk(KERN_INFO "1) %d, 2) %d\n", buffer_head - buffer_count, lenght - 1);
    
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

   
    ts = kthread_run(thread,NULL,"foo kthread");  // запуск потока
    atomic_set(&my_cnt, 0); // инициализация атомарного счетчика

    return 0;
}
void cleanup_module()
{
    // Удаляем символьное устройство
    unregister_chrdev(Major, DEVICE_NAME);
    
    // Освобождаем память, выделенную для буфера
    kfree(device_buffer);

    printk(KERN_INFO "The device %s was unregistered\n", DEVICE_NAME);

    kthread_stop(ts); // Остановка потока

}
