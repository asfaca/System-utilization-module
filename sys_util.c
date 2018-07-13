#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/cpumask.h>
#include <linux/kernel_stat.h>
#include <linux/tick.h>
#include <linux/time.h>
#include <linux/timer.h>

static struct timer_list sys_util_timer;

u64 prev_total_time, prev_used_time;

void sys_util_timer_function(unsigned long data)
{
	u64 user, nice, system, idle, iowait, irq, softirq, steal;
	u64 guest, guest_nice;
	u64 total_time = 0, used_time = 0;
	u64 tmp_total, tmp_used;
	u64 sys_util, int_sys_util, flo_sys_util;
	int i;
	
	user = nice = system = idle = iowait = irq = softirq = steal = 0;
	guest = guest_nice = 0;

	for_each_possible_cpu(i) {
		user += kcpustat_cpu(i).cpustat[CPUTIME_USER];
		nice += kcpustat_cpu(i).cpustat[CPUTIME_NICE];
		system += kcpustat_cpu(i).cpustat[CPUTIME_SYSTEM];
		idle += kcpustat_cpu(i).cpustat[CPUTIME_IDLE];
		iowait += kcpustat_cpu(i).cpustat[CPUTIME_IOWAIT];
		irq += kcpustat_cpu(i).cpustat[CPUTIME_IRQ];
		softirq += kcpustat_cpu(i).cpustat[CPUTIME_SOFTIRQ];
		steal += kcpustat_cpu(i).cpustat[CPUTIME_STEAL];
		guest += kcpustat_cpu(i).cpustat[CPUTIME_GUEST];
		guest_nice += kcpustat_cpu(i).cpustat[CPUTIME_GUEST_NICE]; 
	}
	
	total_time = user + nice + system + idle + iowait + irq + softirq + steal
			+ guest + guest_nice;
	used_time = total_time - idle;

	if (prev_total_time == 0) {
		prev_total_time = total_time;
		prev_used_time = used_time;
	} else {
		tmp_total = total_time - prev_total_time;
		tmp_used = used_time - prev_used_time;
		sys_util = (tmp_used * 10000) / tmp_total;
		int_sys_util = sys_util / 100;
		flo_sys_util = sys_util % 100;
		prev_total_time = total_time;
		prev_used_time = used_time;
		printk("sys util : %lu.%lu\n", int_sys_util, flo_sys_util);
	}

	printk("user : %lu nice : %lu system : %lu idle : %lu iowait : %lu irq : %lu softirq : %lu\n", user, nice, system, idle, iowait, irq, softirq);
	


	sys_util_timer.expires = jiffies + (2 * HZ);
	add_timer(&sys_util_timer);
}


static int __init sys_util_init(void)
{
	printk("sys_util: init module\n");
	
	setup_timer(&sys_util_timer, sys_util_timer_function, 0);
	mod_timer(&sys_util_timer, jiffies + (2 * HZ));
	
	return 0;
}


static void __exit sys_util_exit(void)
{
	printk("sys_util: exit module\n");
	del_timer_sync(&sys_util_timer);
}

module_init(sys_util_init);
module_exit(sys_util_exit);

MODULE_AUTHOR("swKim");
MODULE_DESCRIPTION("sys_util module. This module is for linux-4.4.1");
MODULE_LICENSE("GPL");
MODULE_VERSION("NEW");


