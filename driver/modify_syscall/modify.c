#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/unistd.h>
#include <linux/time.h>
#include <asm/uaccess.h>
#include <linux/sched.h>
#include <linux/kallsyms.h>
#define sys_No 178
MODULE_LICENSE("GPL");
unsigned int clear_and_return_cr0(void);
void setback_cr0(unsigned int val);

int orig_cr0;
unsigned long* sys_call_table = 0;
static int (*anything_saved)(void);

unsigned int clear_and_return_cr0(void) {
    unsigned int cr0 = 0;
    unsigned int ret;
    asm volatile("mrs %0, daif" : "=r"(cr0));
    ret = cr0;
    cr0 &= 0xfffeffff;
    asm volatile("msr daif, %0" ::"r"(cr0));
    return ret;
}

void setback_cr0(unsigned int val) {
    asm volatile("msr daif, %0" ::"r"(val));
}

static int hello(void) {
    int i = 666;
    printk("hello sys_No 178 !");
    return i;
}

static int modify_init(void) {
    sys_call_table = (unsigned long*)kallsyms_lookup_name("sys_call_table");
    anything_saved = (int (*)(void))(sys_call_table[sys_No]);
    orig_cr0 = clear_and_return_cr0();
    sys_call_table[sys_No] = (unsigned long)&hello;
    setback_cr0(orig_cr0);
    return 0;
}

static void modify_exit(void) {
    orig_cr0 = clear_and_return_cr0();
    sys_call_table[sys_No] = (unsigned long)anything_saved;
    setback_cr0(orig_cr0);
}

module_init(modify_init);
module_exit(modify_exit);
