#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
MODULE_LICENSE("GPL");
MODULE_AUTHOR("ALINA YUZLIBAEVA");
MODULE_DESCRIPTION("My first homework - simple example Linux module.");
MODULE_VERSION("30.06.2023");

static char *whom = "world";
module_param(whom, charp, 0);
static int howmany = 1;
module_param(howmany, int, 0);  // Передается пара параметров-сколько рaз и кому передается привет
static int __init hello_init(void) {
    int i;
    for (i = 0; i < howmany; i++)
        printk(KERN_INFO "(%d) HELLO, %s\n", i, whom);
    return 0;
}

static void __exit hello_exit(void) {
    printk(KERN_INFO "GOODBYE, CRUEL %s\n", whom);
}
module_init(hello_init);
module_exit(hello_exit);
