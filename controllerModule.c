#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/delay.h>

static struct task_struct *thread = NULL;

/*
 *this function runs a loop that prints a message (for the moment)
 *
 */
static int controller_thread(void *data){
	while(!kthread_should_stop()){
		printk(KERN_INFO "I'm running!\n");
		msleep(100);
	}
	printk(KERN_INFO "Thread end\n");
	return 0;
}

/*
 *Init function called at module insertion, it creates a thread
 *that runs the controller_thread function, then it returns.
 *
 */
static int __init controller_module_init(void){
	printk(KERN_INFO "Thread started\n");
	thread = kthread_run(controller_thread, NULL, "control_module_thread");
	return 0;
}

/*
 *Exit function called when the module is removed, it stops the main
 *loop in controller_thread and returns.
 *
 */
static void __exit controller_module_exit(void){
	kthread_stop(thread);
	printk(KERN_INFO "Module exit\n");
}

module_init(controller_module_init);
module_exit(controller_module_exit);



MODULE_LICENSE("GPL");