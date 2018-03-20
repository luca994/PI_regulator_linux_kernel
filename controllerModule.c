#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <asm/msr.h>

#define TCC_CELSIUS 100
#define IA32_THERM_STATUS 0x19c
#define IA32_PERF_CTL 0x199
#define IA32_PERF_STATUS 0x198
#define TIMER 1000

static struct task_struct *thread = NULL;

static int freq_param = 8;
module_param(freq_param, int, S_IWUSR);


/*
 *reads the temperature from the processor register IA32_THERM_STATUS
 *using rdmsr function, then returns it
 */
static int get_temperature_msr(void){
	u32 val, dummy;
	rdmsr(IA32_THERM_STATUS, val, dummy);
	val = val & 0x7F0000;
	val = val >> 16;
	val = TCC_CELSIUS - val;
	return val;
}


static void write_frequency_msr(int8_t mult_freq){
	unsigned long val, dummy;
	unsigned short reset = 0xFF;
	unsigned short temp;
	rdmsr(IA32_PERF_CTL, val, dummy);
	//printk(KERN_INFO "val before reset:%i\n", val);
	val = val & reset;
	//printk(KERN_INFO "val after reset:%i\n", val);
	temp = mult_freq << 8;
	//printk(KERN_INFO "temp:%i\n", temp);
	val = val | temp;
	//printk(KERN_INFO "val final:%i\n", val);
	wrmsr(IA32_PERF_CTL, val, dummy);
}

/*
 *this function runs a loop that prints a message (for the moment)
 *
 */
static int controller_thread(void *data){
	while(!kthread_should_stop()){
		printk(KERN_INFO "I'm running!\n");
		int temp = get_temperature_msr();
		printk(KERN_INFO "Temperature = %i°C\n", temp);
		msleep(TIMER);
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
