#include <asm/uaccess.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kdev_t.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/semaphore.h>
#include <linux/types.h>
#include <linux/wait.h>
#define MAXNUM 100
#define MAXUSER 10
#define MAJOR_NUM 290  // 主设备号 ，没有被使用
MODULE_LICENSE("GPL");
struct Scull_Dev {
    struct cdev devm;         // 字符设备
    struct semaphore sem;     // 信号量，实现读写时的 PV 操作
    wait_queue_head_t outq;   // 等待队列，实现阻塞操作
    char buffer[MAXNUM + 1];  // 字符缓冲区
    char *rd, *wr, *end;      // 读，写，尾指针
};

struct Scull_Dev globalvar;
static struct class* my_class;
static int r_len = 0;
static int user[MAXUSER];
int major = MAJOR_NUM;
static ssize_t globalvar_read(struct file*, char*, size_t, loff_t*);
static ssize_t globalvar_write(struct file*, const char*, size_t, loff_t*);
static int globalvar_open(struct inode* inode, struct file* filp);
static int globalvar_release(struct inode* inode, struct file* filp);

struct file_operations globalvar_fops = {
    .read = globalvar_read,
    .write = globalvar_write,
    .open = globalvar_open,
    .release = globalvar_release,
};

static int get_current_pid(void) {
    return task_tgid_vnr(current);
}

static void insert_user(int pid) {
    int i;
    for (i = 0; i < MAXUSER; ++i) {
        if (user[i] == pid) {
            break;
        }
        if (user[i] == -1) {
            user[i] = pid;
            break;
        }
    }
}

static void my_clear_user(void) {
    int i;
    for (i = 0; i < MAXUSER; ++i) {
        user[i] = -1;
    }
}

static bool could_read(int pid) {
    int i;
    for (i = 0; i < MAXUSER; ++i) {
        if (user[i] == pid) {
            return false;
        }
    }
    return true;
}

static int globalvar_init(void) {
    int result = 0;
    int err = 0;
    dev_t dev = MKDEV(major, 0);
    if (major) {
        result = register_chrdev_region(dev, 1, "chardev");
    } else {
        result = alloc_chrdev_region(&dev, 0, 1, "chardev");
        major = MAJOR(dev);
    }
    if (result < 0)
        return result;
    cdev_init(&globalvar.devm, &globalvar_fops);
    globalvar.devm.owner = THIS_MODULE;
    err = cdev_add(&globalvar.devm, dev, 1);
    if (err)
        printk(KERN_INFO "Error %d adding char_mem device", err);
    else {
        printk("globalvar register success\n");
        sema_init(&globalvar.sem, 1);
        init_waitqueue_head(&globalvar.outq);
        globalvar.rd = globalvar.buffer;
        globalvar.wr = globalvar.buffer;
        globalvar.end = globalvar.buffer + MAXNUM;
        my_clear_user();
    }
    my_class = class_create(THIS_MODULE, "chardev0");
    device_create(my_class, NULL, dev, NULL, "chardev0");
    return 0;
}

static int globalvar_open(struct inode* inode, struct file* filp) {
    try_module_get(THIS_MODULE);
    printk("This chrdev is in open\n");
    return (0);
}

static int globalvar_release(struct inode* inode, struct file* filp) {
    module_put(THIS_MODULE);
    printk("This chrdev is in release\n");
    return (0);
}

static void globalvar_exit(void) {
    device_destroy(my_class, MKDEV(major, 0));
    class_destroy(my_class);
    cdev_del(&globalvar.devm);
    unregister_chrdev_region(MKDEV(major, 0), 1);
}

static ssize_t globalvar_read(struct file* filp,
                              char* buf,
                              size_t len,
                              loff_t* off) {
    int pid = get_current_pid();
    if (down_interruptible(&globalvar.sem)) {
        return -ERESTARTSYS;
    }
    if (could_read(pid)) {
        insert_user(pid);
        if (globalvar.rd < globalvar.wr)
            r_len = min(len, (size_t)(globalvar.wr - globalvar.rd));
        else
            r_len = min(len, (size_t)(globalvar.end - globalvar.rd));
        printk("the len is %d\n", len);
        if (raw_copy_to_user(buf, globalvar.rd, len)) {
            printk(KERN_ALERT "copy failed\n");
            up(&globalvar.sem);
            return -EFAULT;
        }
        printk("the read buffer is %s\n", globalvar.buffer);
        up(&globalvar.sem);
        return len;
    } else {
        if (raw_copy_to_user(buf, "\0", 1)) {
            printk(KERN_ALERT "copy failed\n");
            up(&globalvar.sem);
            return -EFAULT;
        }
        up(&globalvar.sem);
        return 0;
    }
}
static ssize_t globalvar_write(struct file* filp,
                               const char* buf,
                               size_t len,
                               loff_t* off) {
    printk("into the write function\n");
    if (down_interruptible(&globalvar.sem)) {
        printk("p in write fail\n");
        return -ERESTARTSYS;
    }
    if (globalvar.rd <= globalvar.wr)
        len = min(len, (size_t)(globalvar.end - globalvar.wr));
    else
        len = min(len, (size_t)(globalvar.rd - globalvar.wr - 1));
    printk("the write len is %d\n", len);
    if (raw_copy_from_user(globalvar.wr, buf, len)) {
        up(&globalvar.sem);
        return -EFAULT;
    }
    printk("the write buffer is %s\n", globalvar.buffer);
    printk("the len of buffer is %d\n", strlen(globalvar.buffer));
    globalvar.wr = globalvar.wr + len;
    globalvar.rd = globalvar.rd + r_len;
    if (globalvar.rd == globalvar.end)
        globalvar.rd = globalvar.buffer;
    if (globalvar.wr == globalvar.end)
        globalvar.wr = globalvar.buffer;
    up(&globalvar.sem);
    my_clear_user();
    return len;
}
module_init(globalvar_init);
module_exit(globalvar_exit);