#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kern_levels.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/ioctl.h>        //add for ioctl

MODULE_LICENSE("GPL");

#define ECE_BUF_SIZE    256
#define ECE_IOC_MAGIC   'E'     //ECE driver magic ASCII number 0x00004500
#define ECE_IOCTL_RST_R _IO(ECE_IOC_MAGIC, 0)   //reset read pointer
#define ECE_IOCTL_RST_W _IO(ECE_IOC_MAGIC, 1)   //reset write pointer 


static char ece_buffer[ECE_BUF_SIZE];
int isReg;
int major;
int ece_offset_w;
int ece_offset_r;
int ece_size;

int ece_init(void);
void ece_end(void);
static ssize_t ece_write(struct file*, const char*, size_t, loff_t*);
static ssize_t ece_read(struct file*, char*, size_t, loff_t*);
static long ece_ioctl(struct file*, unsigned int cmd, unsigned long arg);   //prototype the ioctl

static struct file_operations ece_fops =
{
    .read = ece_read,
    .write = ece_write,
    .unlocked_ioctl = ece_ioctl,    //updated ece_fops with ioctl
};

int ece_init(void)
{
    major = register_chrdev(0, "test_mod01", &ece_fops);
    ece_offset_w = 0;
    ece_offset_r = 0;
    ece_size = 0;

    if(major < 0){
        isReg = 0;
        printk(KERN_INFO " ECE4310: Start FAIL \n" );
    }
    else{
        isReg = 1;
        printk(KERN_INFO " ECE4310: Start HERE \n");
    }
    return 0;
}

void ece_end(void){
    if(isReg){
        unregister_chrdev(major, "test_mod01");
    }
    printk(KERN_INFO " ECE4310: End HERE \n");
}

module_init(ece_init);
module_exit(ece_end);

static ssize_t ece_write(struct file *fp, const char *buf, size_t count, loff_t *op)
{
    int ret = 0;
    if(ece_offset_w + count >= ECE_BUF_SIZE)
    {
        printk(KERN_INFO " ECE4310: Write overflow. Abort. \n");
        return -1;
    }
    printk(KERN_INFO " ECE4310: Copy from user. \n");
    ret = copy_from_user(&ece_buffer[ece_offset_w],buf,count);
    if(ret != 0)
    {
        printk(KERN_INFO " ECE4310: ERR copy from user. \n");
        return -1;
    }
    ece_offset_w = ece_offset_w + count;

    return count;
}

static ssize_t ece_read(struct file *fp, char *buf, size_t count, loff_t *offset)
{
    int ret;
    if(ece_offset_r + count >= ECE_BUF_SIZE)
    {
        printk(KERN_INFO " ECE4310: Read overflow. Abort. \n");
        return -1;
    }
    printk(KERN_INFO " ECE4310: Copy from user. \n");
    ret = copy_to_user(buf, &ece_buffer[ece_offset_r],count);
    if(ret != 0)
    {
        printk(KERN_INFO " ECE4310: ERR copy to user. \n");
        return -1;
    }
    ece_offset_r = ece_offset_r + count;

    return count;
}

static long ece_ioctl(struct file *fp, unsigned int cmd, unsigned long arg)
{
    switch (cmd){
        case ECE_IOCTL_RST_R:
            printk(KERN_INFO " ECE4310: RESET READ POINTER TO 0\n");

            ece_offset_r = 0;

            return 0;
        case ECE_IOCTL_RST_W:
            printk(KERN_INFO " ECE4310: RESET WRITE POINTER TO 0\n");
            ece_offset_w = 0;

            return 0;
        default:
            return -ENOTTY; //typical ioctl return for Error: No TTY
    }    

}
