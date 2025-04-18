#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/kdev_t.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include "platform.h"

#undef pr_fmt
#define pr_fmt(fmt) "%s : " fmt, __func__

#define MAX_DEVICES 10

struct pcdev_private_data {
	struct pcdev_platform_data pdata;
	char *buffer;
	dev_t dev_num;
	struct cdev cdev;
};

struct pcdvr_private_data {
	int total_devices;
	dev_t device_num_base;
	struct class *class_pcd;
};

static struct pcdvr_private_data pcdrv_data;

#define DEV_MEM_SIZE 512

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Deepak M");
MODULE_DESCRIPTION("Deepak's Kernel Module");
MODULE_VERSION("1.0");

// File operations

static loff_t dms_llseek(struct file *file, loff_t off, int whence) {
	pr_info("File pointer changed\n");
	return 0;
}

static int dms_open(struct inode *inode, struct file *file) {
	pr_info("Device opened\n");
	return 0;
}

static int dms_release(struct inode *inode, struct file *file) {
	pr_info("Device closed\n");
	return 0;
}

static ssize_t dms_read(struct file *file, char __user *buff, size_t count, loff_t *f_pos) {
	return 0;
}

static ssize_t dms_write(struct file *file, const char __user *buff, size_t count, loff_t *f_pos) {
	return 0;
}

static struct file_operations pcd_fops = {
	.owner = THIS_MODULE,
	.open = dms_open,
	.read = dms_read,
	.write = dms_write,
	.release = dms_release,
	.llseek = dms_llseek
};

// Platform Driver
static int pcd_platform_driver_remove(struct platform_device *pdev){
	struct pcdev_private_data *dev_data = dev_get_drvdata(&pdev->dev);

	device_destroy(pcdrv_data.class_pcd, dev_data->dev_num);
	cdev_del(&dev_data->cdev);
	pcdrv_data.total_devices--;

	pr_info("a device is removed\n");
	return 0;
}

static int pcd_platform_driver_probe(struct platform_device *pdev) {
	int ret;
	struct pcdev_private_data *dev_data;
	struct pcdev_platform_data *pdata;

	pdata = (struct pcdev_platform_data *)dev_get_platdata(&pdev->dev);
	if (!pdata) {
		pr_info("platform data not available\n");
		return -EINVAL;
	}

	dev_data = devm_kzalloc(&pdev->dev,sizeof(*dev_data), GFP_KERNEL);
	if (!dev_data) {
		pr_info("cannot allocate memory\n");
		return -ENOMEM;
	}

	dev_set_drvdata(&pdev->dev, dev_data);

	dev_data->pdata.size = pdata->size;
	dev_data->pdata.perm = pdata->perm;
	dev_data->pdata.serial_number = pdata->serial_number;

	pr_info("device serial number = %s\n", dev_data->pdata.serial_number);
	pr_info("device size = %d\n", dev_data->pdata.size);
	pr_info("device permission = %d\n", dev_data->pdata.perm);

	dev_data->buffer = devm_kzalloc(&pdev->dev,dev_data->pdata.size, GFP_KERNEL);
	if (!dev_data->buffer) {
		pr_info("cannot allocate memory for device buffer\n");
		devm_kfree(&pdev->dev,dev_data);
		return -ENOMEM;
	}

	dev_data->dev_num = pcdrv_data.device_num_base + pdev->id;

	cdev_init(&dev_data->cdev, &pcd_fops);
	dev_data->cdev.owner = THIS_MODULE;
	ret = cdev_add(&dev_data->cdev, dev_data->dev_num, 1);
	if (ret < 0) {
		pr_info("cdev add failed\n");
		devm_kfree(&pdev->dev,dev_data->buffer);
		devm_kfree(&pdev->dev,dev_data);
		return ret;
	}

	device_create(pcdrv_data.class_pcd, NULL, dev_data->dev_num, NULL, "pcdev-%d", pdev->id);

	pcdrv_data.total_devices++;
	pr_info("probe was successful\n");

	return 0;
}

static struct platform_driver pcd_platform_driver = {
	.probe = pcd_platform_driver_probe,
	.remove = pcd_platform_driver_remove,
	.driver = {
		.name = "pseudo-char-device"
	}
};

// Module Initialization

static int __init hello_init(void) {
	int result;

	pr_info("Kernel Module Loaded\n");

	result = alloc_chrdev_region(&pcdrv_data.device_num_base, 0, MAX_DEVICES, "pcdevs");
	if (result < 0) {
		pr_err("Cannot allocate char dev\n");
		return result;
	}

	pcdrv_data.class_pcd = class_create(THIS_MODULE, "pcd_class");
	if (IS_ERR(pcdrv_data.class_pcd)) {
		pr_err("Failed to create class\n");
		unregister_chrdev_region(pcdrv_data.device_num_base, MAX_DEVICES);
		return PTR_ERR(pcdrv_data.class_pcd);
	}

	platform_driver_register(&pcd_platform_driver);

	pr_info("Device created successfully\n");
	return 0;
}

// Module Cleanup

static void __exit hello_exit(void) {
	pr_info("Unloading module\n");

	platform_driver_unregister(&pcd_platform_driver);
	class_destroy(pcdrv_data.class_pcd);
	unregister_chrdev_region(pcdrv_data.device_num_base, MAX_DEVICES);

	pr_info("Kernel Module Unloaded\n");
}

module_init(hello_init);
module_exit(hello_exit);

