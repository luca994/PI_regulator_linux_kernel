#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <asm/processor.h>
#include <asm/msr-index.h>

/*
 * This function call cpuid to check if some processor features are active and prints the results
 * Opportunistic processor operations are for example: Intel Dynamic Acceleration
 * Intel Turbo boost.
 * IA32_MPERF : Maximum Performance Frequency Clock Count
 * IA32_APERF : Actual Performance Frequency Clock Count 
 */
static int check_active_features(void *data){
    
    unsigned int eax, ebx, ecx, edx;
    u32 low, high;
    cpuid(0x06,&eax,&ebx,&ecx,&edx);
    printk(KERN_INFO "----------------------------------------------------\n");
    if(eax & (1<<1))
        printk(KERN_INFO "Intel Turbo Boost Technology ENABLED \n");
    else
        printk(KERN_INFO "Intel Turbo Boost Technology DISABLED\n");
    if(eax & (1<<7))
		printk(KERN_INFO "HWP ENABLED\n");
	else
		printk(KERN_INFO "HWP DISABLED\n");
    if(eax & (1<<8))
        printk(KERN_INFO "HWP Notification ENABLED\n");
	else
		printk(KERN_INFO "HWP Notification DISABLED\n");
    if(eax & (1<<9))
		printk(KERN_INFO "HWP Activity window control ENABLED\n");
	else
		printk(KERN_INFO "HWP Activity window control DISABLED\n");
    if(eax & (1<<10))
		printk(KERN_INFO "HWP energy/performance preference control ENABLED\n");
	else
		printk(KERN_INFO "HWP energy/performance preference control DISABLED\n");    
    if(eax & (1<<11))
		printk(KERN_INFO "HWP package level control ENABLED\n");
	else
		printk(KERN_INFO "HWP package level control DISABLED\n");       

    rdmsr(MSR_IA32_THERM_CONTROL,low,high);
    if(low & (1<<4))
        printk(KERN_INFO "Software On-Demand clock modulation ENABLED");
    else
        printk(KERN_INFO "Software On-Demand clock modulation DISABLED");  
    
    rdmsr(MSR_IA32_MISC_ENABLE,low,high);
    if(low & (1<<3))
        printk(KERN_INFO "Automatic Thermal Control Circuit ENABLED");
    else
        printk(KERN_INFO "Automatic Thermal Control Circuit DISABLED");
    if(low & (1<<16))
        printk(KERN_INFO "Enhanched Intel SpeedStep Technology ENABLED");
    else
        printk(KERN_INFO "Enhanched Intel SpeedStep Technology DISABLED");
    if(low & (1<<20))
        printk(KERN_INFO "Enhanched Intel SpeedStep Technology select lock ACTIVE");
    else
        printk(KERN_INFO "Enhanched Intel SpeedStep Technology select lock not ACTIVE");
    
    rdmsr(MSR_IA32_TEMPERATURE_TARGET, low, high);
    printk(KERN_INFO "Temperature Junction is = %d°",((low&(0xff<<16))>>16));
    
    return 0;
}



static int __init check_module_init(void){
	kthread_run(check_active_features, NULL, "check_active_features");
	return 0;
}

static void __exit check_module_exit(void){
	printk(KERN_INFO "----------------------------------------------------\n");
}

module_init(check_module_init);
module_exit(check_module_exit);


MODULE_LICENSE("GPL");
