#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kthread.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/xarray.h>
#include <linux/random.h>
#include <linux/completion.h>
#include <linux/sched.h>
#include "calclock.h"

#define NUM_OF_ITEM			100000
#define NUM_OF_THREAD		4

static DEFINE_XARRAY(array);

struct item {
	unsigned long index;
	unsigned int order;
};

struct my_thread_data {
	struct completion *comp;
	unsigned int order;
};

int count;
struct item items[NUM_OF_ITEM];
int size_per_thread;

void generate_data(void)
{
	int i, j;
	unsigned int random;
	int tmp;

	for (i = 0; i < NUM_OF_ITEM; i++) {
		items[i].order = i;
	}
	for (i = 0; i < NUM_OF_ITEM; i++) {
		get_random_bytes(&random, sizeof(int));
		j = random % NUM_OF_ITEM;
		tmp = items[j].order;
		items[j].order = items[i].order;
		items[i].order = tmp;
	}
}

int run_insert(void *arg)
{
	struct item *p, *start;
	int count;
	struct item element;
	int i;

	struct my_thread_data *thread_data = arg;
	struct completion *comp = thread_data->comp;
	long data = (long)thread_data->order;

	p = items + data * size_per_thread;
	start = p;
	count = size_per_thread;

	printk("Starting insert... (PID : %d)", current->pid);
	for (i = 0; i < count; i++) {
		element = start[i];
		xa_store(&array, data * size_per_thread + i, (void *)&element,  GFP_KERNEL); //xa_store(&array, index, item, FLAG);
	}

	complete(comp);

	return 0;
}

int run_multi_thread_insert(int thread_count, int num_of_data)
{
	int i;

	struct task_struct *thread[16];
	struct completion comps[16];
	struct my_thread_data data[16];

	struct timespec64 spclock[2];
	unsigned long long time = 0;
	unsigned long long count = 0;

	size_per_thread = num_of_data / thread_count;

	ktime_get_ts64(&spclock[0]);

	for (i = 0; i < thread_count; i++) {
		init_completion(&comps[i]);
		data[i].comp = &comps[i];
		data[i].order = i;
		thread[i] = kthread_run(&run_insert, &data[i], "insert");
	}

	for (i = 0; i < thread_count; i++) {
		wait_for_completion(&comps[i]);
	}

	ktime_get_ts64(&spclock[1]);
	calclock(spclock, &time, &count);

	for (i = 0; i < thread_count; i++) {
		kthread_stop(thread[i]);
	}

	printk("time taken by insert with %d thread: %lld nsec\n", thread_count, time);

	return 0;
}

int run_remove(void *arg)
{
	struct item *p, *start;
	int count;
	struct item element;
	int i;

	struct my_thread_data *thread_data = arg;
	struct completion *comp = thread_data->comp;
	long data = (long)thread_data->order;

	p = items + data * size_per_thread;
	start = p;
	count = size_per_thread;

	printk("Starting remove... (PID : %d)", current->pid);
	for (i = 0; i < count; i++) {
		element = start[i];
		xa_erase(&array, data * size_per_thread + i);
	}

	complete(comp);

	return 0;
}

int run_multi_thread_remove(int thread_count, int num_of_data)
{
	int i;

	struct task_struct *thread[16];
	struct completion comps[16];
	struct my_thread_data data[16];

	struct timespec64 spclock[2];
	unsigned long long time = 0;
	unsigned long long count = 0;

	size_per_thread = num_of_data / thread_count;

	ktime_get_ts64(&spclock[0]);

	for (i = 0; i < thread_count; i++) {
		init_completion(&comps[i]);
		data[i].comp = &comps[i];
		data[i].order = i;
		thread[i] = kthread_run(&run_remove, &data[i], "remove");
	}

	for (i = 0; i < thread_count; i++) {
		wait_for_completion(&comps[i]);
	}

	ktime_get_ts64(&spclock[1]);
	calclock(spclock, &time, &count);

	for (i = 0; i < thread_count; i++) {
		kthread_stop(thread[i]);
	}

	printk("time taken by remove with %d thread: %lld nsec\n", thread_count, time);

	return 0;
}

void xarray_test(void)
{
	int i, j;
	int threads_num[3] = {1, 2, 4};
	int data[4];
	int num_processes_i = 1;
	int num_processes_r = 1;
	int thread_num;

	generate_data();

	for (i = 0; i < 4; i++) {
		data[i] = NUM_OF_ITEM / NUM_OF_THREAD * (i + 1);
	}

	for (i = 0; i < 4; i++) {
		printk("total size: %d\n", data[i]);
		for (j = 0; j < 3; j++) {
			thread_num = threads_num[j];
			num_processes_r = num_processes_i = thread_num;
			
			run_multi_thread_insert(num_processes_i, data[i]);
			run_multi_thread_remove(num_processes_r, data[i]);
		}
		printk("\n");
	}
	printk("\n");
}

static int __init xarray_mod_init(void)
{
	printk("%s, Entering module\n", __func__);
	xarray_test();

	return 0;
}

static void __exit xarray_mod_exit(void)
{
	printk("%s, Exiting module\n", __func__);
}

module_init(xarray_mod_init);
module_exit(xarray_mod_exit);

MODULE_LICENSE("GPL");
