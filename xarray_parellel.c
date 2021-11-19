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

#define NUM_OF_ITEM		1000
#define NUM_OF_THREAD	4

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
struct item items[NUM_OF_DATA];

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
	int *p, *start;
	int count, element;
	int i;

	struct my_thread_data *thread_data = arg;
	struct completion *comp = thread_data->comp;
	long data = (long)thread_data->num;

	p = numbers + data * size_per_thread;
	start = p;
	count = size_per_thread;

	printk("Starting insert... (PID : %d)", current->pid);
	for (i = 0; i < count; i++) {
		element = start[i];
		xa_store(&array

void xarray_test(void)
{

}

static int __init xarray_mod_init(void)
{
	printk("%s, Entering module\n", __func__);
	xarray_test();

	return 0;
}

static void __exit xarray_mod_test(void)
{
	printk("%s, Exiting module\n", __func__);
}

module_init(xarray_mod_init);
module_exit(xarray_mod_exit);

MODULE_LICENSE("GPL");
