#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <asm/msr.h>

#define TCC_CELSIUS 100
#define LOGICAL_CORES_N 8
#define IA32_THERM_STATUS 0x19c
#define IA32_PERF_CTL 0x199
#define IA32_PERF_STATUS 0x198
#define IA32_MISC_ENABLE 0x1a0
#define TIMER 1000

static struct task_struct *thread = NULL;

static int freq_param = 8;
module_param(freq_param,int, S_IWUSR);


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

/*
 * this function reads the current value for the frequency in the msr register
 * called IA32_PERF_STATUS
 */
static int read_frequency_msr(void){
	u32 val, dummy;
	rdmsr(IA32_PERF_STATUS, val, dummy);
	val &= 0xFFFF;
	return (val >> 8);
}

/*
 * this function disbles the bit related to TCC in msr register
 * called IA32_MISC_ENABLE
 */
static void disable_tcc(void){
    u64 val, dummy;
    rdmsr(IA32_MISC_ENABLE, val, dummy);
    val &= ~(1UL<<3); 
    wrmsr(IA32_MISC_ENABLE, val, dummy);
}

/*
 * this function enables the bit related to TCC in msr register
 * called IA32_MISC_ENABLE
 */
static void enable_tcc(void){
    u64 val, dummy;
    rdmsr(IA32_MISC_ENABLE, val, dummy);
    val |= (1UL<<3);
    wrmsr(IA32_MISC_ENABLE, val, dummy);
}

/*
 * this function writes the new frequency in the msr register called
 * IA32_PERF_CTL in all the cpu cores
 * 
 */
static void write_frequency_msr(u64 mult_freq){
	u64 val, dummy;
    int i,j;
    rdmsr(IA32_PERF_CTL, val, dummy);
    for(i=8;i<16;i++)
        val &= ~(1UL<<i);
    val |= (mult_freq<<8);
    for(j=0;j<LOGICAL_CORES_N;j++)
    wrmsr_on_cpu(j,IA32_PERF_CTL, val, dummy);
}

/*
 *this function runs a loop that prints a message (for the moment)
 *
 */
static int controller_thread(void *data){
    u64 val, dummy;
    u64 mul = freq_param;
    int temp;
    disable_tcc();
    write_frequency_msr(mul);
    printk(KERN_INFO "Setting freq to: %i", mul);
	while(!kthread_should_stop()){
		temp = get_temperature_msr();
		printk(KERN_INFO "Frequency Multiplier = %i\n", read_frequency_msr());
		printk(KERN_INFO "Temperature = %i°C\n", temp);
		msleep(TIMER);
	}
	enable_tcc();
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
