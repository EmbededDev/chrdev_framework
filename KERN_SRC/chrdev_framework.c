/*
===============================================================================
Driver Name		:		chrdev_framework
Author			:		CJC
License			:		GPL
Description		:		LINUX DEVICE DRIVER PROJECT
===============================================================================
*/

#include"chrdev_framework.h"

#define CHRDEV_FRAMEWORK_N_MINORS 1   // how much sub devices
#define CHRDEV_FRAMEWORK_FIRST_MINOR 0  // first sub device index
#define CHRDEV_FRAMEWORK_BUFF_SIZE 1024

MODULE_LICENSE("GPL");
MODULE_AUTHOR("CJC");

int chrdev_framework_major=0;

dev_t chrdev_framework_device_num;

/* data struct for a device */
typedef struct privatedata {
	int nMinor;

	char buff[CHRDEV_FRAMEWORK_BUFF_SIZE];
	int pointer;
	struct cdev cdev;   // cdev instance

} chrdev_framework_private;

chrdev_framework_private devices[CHRDEV_FRAMEWORK_N_MINORS];	// devices array

/* get called when device node "open" by user thread */
static int chrdev_framework_open(struct inode *inode,struct file *filp)
{
	/* inode->i_cdev 就是 cdev_add() 添加到内核到 cdev 数据实例；
	 * 本模块定义来一个私有到数据结构 chrdev_framework_private， 包含一个 cdev 成员， 通过 open 传回到 cdev 实例， 使用 container_of()
	 * 反向查找 chrdev_framework_private 实例
	 * 通常使用 filp->private_data 成员来存储 chrdev_framework_private 实例，以便其他 file operation 方法使用， 每个方法都有 filp 参数*/

	/* TODO Auto-generated Function */

	chrdev_framework_private *priv = container_of(inode->i_cdev ,
									chrdev_framework_private ,cdev);
	filp->private_data = priv;

//	memset(priv->buff, 0, CHRDEV_FRAMEWORK_BUFF_SIZE);
	priv->pointer = 0;

	PINFO("In char driver open() function\n");

	return 0;
}					

/* get called when device node "close" by user thread*/
static int chrdev_framework_release(struct inode *inode,struct file *filp)
{
	/* 释放设备 */

	/* TODO Auto-generated Function */

	chrdev_framework_private *priv;

	priv=filp->private_data;
	priv->pointer = 0;
	PINFO("In char driver release() function\n");

	return 0;
}

/* get called when device node "read" by user thread */
static ssize_t chrdev_framework_read(struct file *filp, 
	char __user *ubuff,size_t count,loff_t *offp)
{
	/* 从设备读 count 字节数据到 ubuff, offp 返回新到偏移量 */

	/* TODO Auto-generated Function */
	int n=0;

	chrdev_framework_private *priv;

	PINFO("In char driver read() function.\n");
	PINFO("args: count=%d, offp=%d\n", count, *offp);
	priv = filp->private_data;

	if(*offp >= CHRDEV_FRAMEWORK_BUFF_SIZE)
		return 0;
	if(count > CHRDEV_FRAMEWORK_BUFF_SIZE - *offp){
		count = CHRDEV_FRAMEWORK_BUFF_SIZE - *offp;
	}

	// 返回“不成功”拷贝到字节数
	if(copy_to_user(ubuff, priv->buff + *offp, count)){
		return -EFAULT;
	}else{
		*offp += count;
		n = count;
	}

	PINFO("read: %d\n", n);

	return n;
}

/* get called when device node "write" by user thread */
static ssize_t chrdev_framework_write(struct file *filp, 
	const char __user *ubuff, size_t count, loff_t *offp)
{
	/* 把 ubuff 中 count 字节的数据写入设备 */

	/* TODO Auto-generated Function */
	int n=0;

	chrdev_framework_private *priv;
	PINFO("In char driver write() function.\n");
	PINFO("args: count=%d, offp=%d\n", count, *offp);
	priv = filp->private_data;

	if(*offp >= CHRDEV_FRAMEWORK_BUFF_SIZE){
		n = 0;
	}else if(count > CHRDEV_FRAMEWORK_BUFF_SIZE - *offp){
		count = CHRDEV_FRAMEWORK_BUFF_SIZE - *offp;
	}

	if(copy_from_user(priv->buff, ubuff + *offp, count)){
		// 返回“不成功”拷贝到字节数
		return -EFAULT;
	}else{
		*offp += count;
		n = count;
	}

	PINFO("write: %d\n", n);
	PINFO("last wrote char %c\n", priv->buff[*offp - 1]);
	return n;  // 如果返回 0 会导致 echo 不断地 write ？？ 应该是因为这是非阻塞地写 返回0表示为没有写入，而所以 echo 没有完成，所以一直loop直到完成
}

/* get called when device node "fctl" by user thread */
static long chrdev_framework_ioctl(
		struct file *filp, unsigned int cmd , unsigned long arg)
{
	/* 根据用户传入到命令 cmd 和 命令参数 arg 控制设备到行为 */
	/* TODO Auto-generated Function */

	chrdev_framework_private *priv;

	priv = filp->private_data;

	PINFO("In char driver ioctl() function, cmd: %d, arg: %ld\n", cmd, arg);

	return 0;
}

/* get called when device file "seek" */
static loff_t chrdev_framework_llseek(struct file *filp, loff_t off, int whence)
{
	/* TODO Auto-generated Function */

	loff_t newposition;

	switch (whence) {
		case 0: /* SEEK_SET */
			if(off >= CHRDEV_FRAMEWORK_BUFF_SIZE)
				off = CHRDEV_FRAMEWORK_BUFF_SIZE;
			newposition = off;
			break;

		case 1: /* SEEK_CUR */
			if(filp->f_pos + off >= CHRDEV_FRAMEWORK_BUFF_SIZE)
				newposition = CHRDEV_FRAMEWORK_BUFF_SIZE;
			else
				newposition = filp->f_pos + off;
			break;

		case 2: /* SEEK_END */
			newposition = CHRDEV_FRAMEWORK_BUFF_SIZE + off;
			break;

		default:
			return -EINVAL;
	}

	if (newposition < 0)
		return -EINVAL;

	filp->f_pos = newposition;

	return newposition;
}

static const struct file_operations chrdev_framework_fops= {
	.owner				= THIS_MODULE,
	.open				= chrdev_framework_open,
	.release			= chrdev_framework_release,
	.read				= chrdev_framework_read,
	.write				= chrdev_framework_write,
	.unlocked_ioctl		= chrdev_framework_ioctl,   // no more "ioctl", been replaced with call unlocked_iotcl
	.llseek				= chrdev_framework_llseek,
};

// 在内核函数装载时被调用
static int __init chrdev_framework_init(void)
{
	/* 整个内核模块到初始化， 初始化驱动程序相关数据结构 */

	/* TODO Auto-generated Function Stub */

	int i;
	int res;

	// allocate device number region dynamically
	// 是一个范围 数量有 CHRDEV_FRAMEWORK_N_MINORS 指定， 起始偏移量由 CHRDEV_FRAMEWORK_FIRST_MINOR 指定
	res = alloc_chrdev_region(&chrdev_framework_device_num,CHRDEV_FRAMEWORK_FIRST_MINOR,CHRDEV_FRAMEWORK_N_MINORS ,DRIVER_NAME);
	if(res) {
		PERR("register device no failed\n");
		return -1;
	}
	chrdev_framework_major = MAJOR(chrdev_framework_device_num);

	// 主设备号与子设备号组合成具体设备 cdev 到设备号， 这里到偏移量应于 alloc_cdev_region 的参数一致
	for(i=0;i<CHRDEV_FRAMEWORK_N_MINORS;i++) {
		chrdev_framework_device_num= MKDEV(chrdev_framework_major ,CHRDEV_FRAMEWORK_FIRST_MINOR+i);
		cdev_init(&devices[i].cdev , &chrdev_framework_fops);
		cdev_add(&devices[i].cdev,chrdev_framework_device_num,1);  // 关联 cdev 到 分配到 设备号， 真正注册到内核

		devices[i].nMinor = CHRDEV_FRAMEWORK_FIRST_MINOR+i;
	}

	PINFO("INIT\n");

	return 0;
}

/* 在内核模块卸载时被调用 */
static void __exit chrdev_framework_exit(void)
{	
	/* 卸载模块之前的释放操作， 是 _init 函数到逆过程 */

	/* TODO Auto-generated Function Stub */

	int i;

	PINFO("EXIT\n");

	for(i=0;i<CHRDEV_FRAMEWORK_N_MINORS;i++) {
		chrdev_framework_device_num= MKDEV(chrdev_framework_major ,CHRDEV_FRAMEWORK_FIRST_MINOR+i);

		cdev_del(&devices[i].cdev);

	}

	unregister_chrdev_region(chrdev_framework_device_num ,CHRDEV_FRAMEWORK_N_MINORS);	

}

module_init(chrdev_framework_init);
module_exit(chrdev_framework_exit);

