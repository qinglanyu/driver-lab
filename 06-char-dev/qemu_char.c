#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h> // 提供用户空间与内核空间互相访问的接口

#define DEVICE_NAME "qemu_char"
#define BUFFER_SIZE 256

static dev_t qemu_char_dev;             // 设备号
static struct cdev qemu_char_cdev;      // 设备号与操作方法的关联
static struct class *qemu_char_class;   // 用于生成设备节点

static char qemu_char_buffer[BUFFER_SIZE];
static size_t qemu_char_size;


static int qemu_char_open(struct inode *inode, struct file *file)
{
    pr_info("qemu_char: device opened!\n");
    return 0;
}

static int qemu_char_release(struct inode *inode, struct file *file)
{
    pr_info("qemu_char: device closed!\n");
    return 0;
}

static ssize_t qemu_char_write(struct file *file,
                               const char __user *buf,
                               size_t count,
                               loff_t *ppos)
{
    size_t len = min(count, (size_t)(BUFFER_SIZE - 1));

    if (copy_from_user(qemu_char_buffer, buf, len))
        return -EFAULT;

    qemu_char_buffer[len] = '\0';
    qemu_char_size = len;
    *ppos = 0;

    pr_info("qemu_char: received %zu bytes\n", len);
    return len;
}


static ssize_t qemu_char_read(struct file *file,
                              char __user *buf,
                              size_t count,
                              loff_t *ppos)
{
    size_t remaining;
    size_t len;

    if (*ppos >= qemu_char_size)
        return 0;

    remaining = qemu_char_size - *ppos;
    len = min(count, remaining);

    if (copy_to_user(buf, qemu_char_buffer + *ppos, len))
        return -EFAULT;

    *ppos += len;
    return len;
}


static const struct file_operations qemu_char_fops = {
    .owner = THIS_MODULE,
    .open = qemu_char_open,
    .read = qemu_char_read,
    .write = qemu_char_write,
    .release = qemu_char_release,
    .llseek = no_llseek,
};


static int __init qemu_char_init(void)
{
    struct device *device;
    int ret;

    // 申请设备号，主设备号，次设备号写入qemu_char_dev，次设备号从0开始，只申请1个设备号
    ret = alloc_chrdev_region(&qemu_char_dev, 0, 1, DEVICE_NAME);
    if (ret)
        return ret;

    cdev_init(&qemu_char_cdev, &qemu_char_fops);    // 关联设备号与文件操作表
    qemu_char_cdev.owner = THIS_MODULE;             // 当前模块的字符设备

    ret = cdev_add(&qemu_char_cdev, qemu_char_dev, 1);  // 字符设备对象注册到字符设备框架，并将申请到的字符设备号与设备对应
    if (ret)
        goto err_unregister;

    qemu_char_class = class_create(DEVICE_NAME);    // 创建一个class对象
    if (IS_ERR(qemu_char_class)) {
        ret = PTR_ERR(qemu_char_class);
        goto err_cdev;
    }

    device = device_create(qemu_char_class, NULL,
                           qemu_char_dev, NULL, DEVICE_NAME);   // 创建并注册一个设备对象
    if (IS_ERR(device)) {
        ret = PTR_ERR(device);
        goto err_class;
    }

    pr_info("qemu_char: registered major=%d minor=%d\n",
            MAJOR(qemu_char_dev), MINOR(qemu_char_dev));
    return 0;

err_class:
    class_destroy(qemu_char_class);
err_cdev:
    cdev_del(&qemu_char_cdev);
err_unregister:
    unregister_chrdev_region(qemu_char_dev, 1);
    return ret;
}

static void __exit qemu_char_exit(void)
{
    device_destroy(qemu_char_class, qemu_char_dev);
    class_destroy(qemu_char_class);
    cdev_del(&qemu_char_cdev);
    unregister_chrdev_region(qemu_char_dev, 1);

    pr_info("qemu_char: removed\n");
}

module_init(qemu_char_init);
module_exit(qemu_char_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("linux-study");
MODULE_DESCRIPTION("Minimal in-memory character device");
