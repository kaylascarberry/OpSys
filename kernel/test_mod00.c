#include <linux/kernel.h>
#include <linux/kern_levels.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");

int ece_init(void)
{
    printk(KERN_INFO " ECE4310: Module loaded \n");
    return 0;
}

void ece_end(void){
    printk(KERN_INFO " ECE4310: Module removed \n");
}

module_init(ece_init);
module_exit(ece_end);