#include <linux/init.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/ioport.h>
//#include <stdint.h>

#define VENDOR_ID 0x10ec
#define DEVICE_ID 0xc821
MODULE_LICENSE("GPL");
// 10ec:c821

// pci драйвер 
static struct pci_device_id pci_id_table[];
static int pci_probe(struct pci_dev *dev, const struct pci_device_id *id);
static void pci_remove(struct pci_dev *dev);
int pci_init_module(void);
void pci_cleanup_module(void);

static struct pci_device_id pci_id_table[] = {
	{ PCI_DEVICE(VENDOR_ID, DEVICE_ID) },
	{0,}
};
MODULE_DEVICE_TABLE (pci, pci_id_table);


static struct pci_driver my_pci_driver = {
    .name = "pci_driver",
    .id_table = pci_id_table,
    .probe = pci_probe,
    .remove = pci_remove
};

static int pci_probe(struct pci_dev *dev, const struct pci_device_id *id)
{
    /*
    port_addr = pci_resource_start(dev,0);
    major = register_chrdev(0,"MyPCI",&fops);
    printk(KERN_INFO "Load driver PCI %d\n",major);
    */
    printk(KERN_INFO "LOAD PCI DRIVER!\n");

    printk ("Get physics BAR0...");
    unsigned int mem_start = pci_resource_start(dev,0);
    unsigned int mem_size = pci_resource_len(dev,0);
    if (mem_start == 0 || mem_size == 0)
    {
        printk(KERN_INFO "failed\n");
        return -1;
    } else {
        printk (KERN_INFO "%u OK.\n", (uint32_t)mem_start);
    }

    printk ("Get virtual BAR0...");
    unsigned char* mem_ptr = ioremap(mem_start, mem_size);
    if (mem_ptr == 0)
    {
        printk(KERN_INFO "failed.\n");
        return -1;
    } else {
        printk(KERN_INFO "%u OK.\n", (uint32_t)mem_ptr);
    }
    
    for (unsigned char* sm = 1; sm < 1000; sm++)
    {
        printk(KERN_INFO "%c ", mem_ptr + sm);
        /*
        if (mem_ptr + sm == 'c' && mem_ptr + sm + 1== '0')
        {
            printk(KERN_INFO "SM = %u\n", sm);
            break;
        }
        */
    }
    

    return 0;
};

static void pci_remove(struct pci_dev *dev)
{
    struct my_driver_priv *drv_priv = pci_get_drvdata(dev);

	if (drv_priv) {
		kfree(drv_priv);
	}

	pci_disable_device(dev);
};

int pci_init_module(void) {
    return pci_register_driver(&my_pci_driver);
};

void pci_cleanup_module(void)
{
    pci_unregister_driver(&my_pci_driver);
};


module_init(pci_init_module);
module_exit(pci_cleanup_module);
