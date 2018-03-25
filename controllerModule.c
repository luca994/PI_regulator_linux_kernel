#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <asm/msr.h>
#include <asm/msr-index.h>
#include <linux/kfifo.h>
#include <linux/sched.h>

#define LOGICAL_CORES_N 8
#define TIMER 1000
#define PRINTK_QUEUE_SIZE 512
#define INTEL_PERF_STATUS_MASK 0xffff

static struct task_struct *controller_thread = NULL;
static struct task_struct *printk_thread = NULL;

typedef struct values_queue {
	int core;
	int temp;
	int mult_freq;
} values_queue;

static bool full_queue;
static int freq_param = 8;
module_param(freq_param, int, S_IWUSR);

DEFINE_KFIFO(printk_queue, values_queue, PRINTK_QUEUE_SIZE);


/*
 * reads the temperature from the processor register MSR_IA32_THERM_STATUS
 * using rdmsr function, then returns it
 */
static int get_temperature_msr(unsigned int core){
	unsigned int tjunction_celsius;
    u32 val, dummy;
    rdmsr(MSR_IA32_TEMPERATURE_TARGET,val,dummy);
    tjunction_celsius = ((val&(0xff<<16))>>16);
	rdmsr_on_cpu(core,MSR_IA32_THERM_STATUS, &val, &dummy);
	val = (val&THERM_MASK_THRESHOLD1) >> THERM_SHIFT_THRESHOLD1;
	val = tjunction_celsius - val;
	return val;
}

/*
 * this function reads the current value for the frequency in the msr register
 * called MSR_IA32_PERF_STATUS
 */
static int read_frequency_msr(unsigned int core){
	u32 val, dummy;
	rdmsr_on_cpu(core,MSR_IA32_PERF_STATUS, &val, &dummy);
	val &= INTEL_PERF_STATUS_MASK;
	return (val >> 8);
}

/*
 * this function disbles the bit related to TCC in msr register
 * called MSR_IA32_MISC_ENABLE
 */
static void disable_tcc(void){
    u64 val, dummy;
    rdmsr(MSR_IA32_MISC_ENABLE, val, dummy);
    val &= ~(1UL<<3); 
    wrmsr(MSR_IA32_MISC_ENABLE, val, dummy);
}

/*
 * this function enables the bit related to TCC in msr register
 * called MSR_IA32_MISC_ENABLE
 */
static void enable_tcc(void){
    u64 val, dummy;
    rdmsr(MSR_IA32_MISC_ENABLE, val, dummy);
    val |= (1UL<<3);
    wrmsr(MSR_IA32_MISC_ENABLE, val, dummy);
}

/*
 * this function writes the new frequency in the msr register called
 * MSR_IA32_PERF_CTL in all the cpu cores
 * 
 */
static void write_frequency_msr(unsigned int mult_freq,unsigned int core){
	u32 val, dummy;
    //TODO check the frequency before write (control if it is too high)
    rdmsr_on_cpu(core, MSR_IA32_PERF_CTL, &val, &dummy);
    val &= INTEL_PERF_CTL_MASK<<16;
    val |= (mult_freq<<8);    
    wrmsr_on_cpu(core, MSR_IA32_PERF_CTL, val, dummy);
}

/*
 * this function takes the values from the printk_queue and prints them,
 * if the queue is empty the thread goes to sleep.
 * If the queue is full prints a warning message.
 * 
 */
static int thread_printk(void *data){
	values_queue values={-1,-1,-1};
	while(!kthread_should_stop()){
		set_current_state(TASK_UNINTERRUPTIBLE);
  		if(kfifo_is_empty(&printk_queue)) {
        	schedule();
  		}
		kfifo_out(&printk_queue, &values, 1);
		printk(KERN_INFO "Core:%i\nFrequency Multiplier = %i\nTemperature = %i°C\n-------------------------\n", values.core, values.mult_freq, values.temp);
		if(full_queue)
			printk(KERN_WARNING "The queue of log is full");
	}
	return 0;
}

/*
 * this function takes the temperature of the cores, passes it to the controller and modifies the
 * frequencies based on the value returned by the controller.
 *
 */
static int thread_controller(void *data){
    int count_cores;
    unsigned int mult_freq = freq_param;
    values_queue values;
    //disable_tcc();
    
    printk(KERN_INFO "Setting freq to: %i", mult_freq);
	while(!kthread_should_stop()){

		for(count_cores=0; count_cores<LOGICAL_CORES_N; count_cores++){
			values.core=count_cores;
			write_frequency_msr(mult_freq, count_cores);
			values.temp=get_temperature_msr(count_cores);		
			values.mult_freq=read_frequency_msr(count_cores);
			
			//alternative method to print the messages (used for debugging, it should takes tenth of microseconds). 
			//You have to read the file /sys/kernel/debug/tracing/options/trace.
			//trace_printk("Core:%i\nFrequency Multiplier = %i\nTemperature = %i°C\n", values.core, values.mult_freq, values.temp);
			
			if(!kfifo_is_full(&printk_queue)){
				full_queue=false;
				kfifo_in(&printk_queue, &values, 1);
			}
			else{
				full_queue=true;	
			}		
			wake_up_process(printk_thread);
		}

		msleep(TIMER);
	}
	//enable_tcc();
	return 0;
}

/*
 * Init function called at module insertion, it creates a thread
 * that runs the thread_controller function, then it returns.
 *
 */
static int __init controller_module_init(void){
	printk(KERN_INFO "Module init\n");
	printk_thread = kthread_run(thread_printk, NULL, "printk_thread");
	controller_thread = kthread_run(thread_controller, NULL, "control_module_thread");
	return 0;
}

/*
 * Exit function called when the module is removed, it stops the loops
 * in the two threads and returns.
 *
 */
static void __exit controller_module_exit(void){
	kthread_stop(controller_thread);
	kthread_stop(printk_thread);
	printk(KERN_INFO "Module exit\n");
}

module_init(controller_module_init);
module_exit(controller_module_exit);



MODULE_LICENSE("GPL");
