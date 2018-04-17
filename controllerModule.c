#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <asm/msr.h>
#include <asm/msr-index.h>
#include <linux/sched.h>

#define LOGICAL_CORES_N 8
#define TIMER 5
#define SET_POINT 80
#define ALPHA 10
#define INTEL_PERF_STATUS_MASK 0xffff

static struct task_struct *controller_thread = NULL;

static int freq_param = 8;
module_param(freq_param, int, S_IWUSR);


/*
 * reads the temperature from the processor register MSR_IA32_THERM_STATUS
 * using rdmsr function, then returns it
 */
static int read_temperature_msr(unsigned int core){
	unsigned int tjunction_celsius;
    u32 low, high;
    rdmsr(MSR_IA32_TEMPERATURE_TARGET,low,high);
    tjunction_celsius = ((low&(0xff<<16))>>16);
	rdmsr_on_cpu(core,MSR_IA32_THERM_STATUS, &low, &high);
	low = (low&THERM_MASK_THRESHOLD1) >> THERM_SHIFT_THRESHOLD1;
	low = tjunction_celsius - low;
	return low;
}
/*
 * return the min value among the two lowues taken in input
 */
static int min_value(int val1,int val2){
    return val1<val2 ? val1:val2;
}

/*
 * return the max value among the two lowues taken in input
 */
static int max_value(int val1,int val2){
    return val1>val2 ? val1:val2;
}

/*
 * it returns the temperature of the core with the highest temperature in celsius degree
 */
static int get_max_core_temperature(void){
    int core_count;
    int temp=0;
    for(core_count=0;core_count<LOGICAL_CORES_N;core_count++){
        temp=max_value(temp,read_temperature_msr(core_count));
    }
    return temp;
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
    u32 low, high;
    rdmsr(MSR_IA32_MISC_ENABLE, low, high);
    low &= ~(1UL<<3); 
    wrmsr(MSR_IA32_MISC_ENABLE, low, high);
}

/*
 * this function enables the bit related to TCC in msr register
 * called MSR_IA32_MISC_ENABLE
 */
static void enable_tcc(void){
    u32 low, high;
    rdmsr(MSR_IA32_MISC_ENABLE, low, high);
    low |= (1UL<<3);
    wrmsr(MSR_IA32_MISC_ENABLE, low, high);
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
}

/*
 * this function takes the temperature of the cores, passes it to the controller and modifies the
 * frequencies based on the value returned by the controller.
 *
 */
static int thread_controller(void *data){
    //disable_tcc();
    
	while(!kthread_should_stop()){
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
	printk(KERN_INFO "Module exit\n");
}

module_init(controller_module_init);
module_exit(controller_module_exit);



MODULE_LICENSE("GPL");
