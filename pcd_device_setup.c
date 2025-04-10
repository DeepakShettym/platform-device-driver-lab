#include<linux/module.h>
#include<linux/platform_device.h>
#include "platform.h"

//1. create 2 platform data

struct pcddev_platform pcdev_pdata[2] = {
	 {.size = 512, .perm = RDWR , .serial_number = "PCDEVABC1111"}, {.size = 1024, .perm = RDWR , .serial_number = "PCDEVABC2222"}
};
//2. create 2 platform devices

struct platform_device platform_pcddev_1 = {
	.name = "pseudo-char-device",
	.id = 0,
	.dev = {
		.platform_data = &pcdev_pdata[0]
	}
};

struct platform_device platform_pcddev_2 = {
	.name = "pseudo-char-device",
	.id = 0,
	.dev = {
		.platform_data = &pcdev_pdata[1]
	}
};


static int __init pcddev_platform_init(void){
	platform_device_register(&platform_pcddev_1);
	platform_device_register(&platform_pcddev_2);
	return 0;
}


static void __exit pcddev_platform_exit(void){
	platform_device_unregister(&platform_pcddev_1);
	platform_device_unregister(&platform_pcddev_2);

}

module_init(pcddev_platform_init);
module_exit(pcddev_platform_exit);


MODULE_LICENSE("GPL");              
MODULE_AUTHOR("Deepak");
MODULE_DESCRIPTION("Module which registers platform devices");
