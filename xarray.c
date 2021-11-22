#include <linux/xarray.h>
#include <linux/module.h>
#include <linux/slab.h>
 
static DEFINE_XARRAY(array);

static void xa_user_dump(const struct xarray *xa)
{
	void *entry = xa->xa_head;
	unsigned int shift = 0; 
	printk("xarray: %px head %px flags %x marks %d %d %d\n", xa, entry,
		xa->xa_flags, xa_marked(xa, XA_MARK_0),
		xa_marked(xa, XA_MARK_1), xa_marked(xa, XA_MARK_2));
}
 
void xarray_test(void)
{
	unsigned long i;
	void *ret;
	struct item {
		unsigned long index;
		unsigned int order;
	};
	struct item *item;
	
	printk("xarray_user_test() starting...\n");
	item = kmalloc(sizeof(*item), GFP_KERNEL);
	item->index = 0;
	item->order = 100;
	printk("[0] item=%p\n", item);

	ret = xa_store(&array, 0, (void *)item, GFP_KERNEL); 
	for (i = 1; i <= 10; i += 2) { 
		item = kmalloc(sizeof(*item), GFP_KERNEL);
		printk("[%d] item=%p, ", i, item);
		item->index = i;
		item->order = i + 100;
		ret = xa_store(&array, i, (void *)item, GFP_KERNEL); 
		printk("ret=%p\n", ret); 
		xa_user_dump(&array);
	}	
/*
	ret = xa_load(&array, 0);
	printk("load ret=%p, %d, %d\n",
		ret, ((struct item *)ret)->index, ((struct item *)ret)->order); 
	ret = xa_load(&array, 8); 
	printk("load ret=%p, %d, %d\n",
		ret, ((struct item *)ret)->index, ((struct item *)ret)->order); 
 
	ret = xa_erase(&array, 4); 
	printk("erase ret=%p, %d, %d\n",
		ret, ((struct item *)ret)->index, ((struct item *)ret)->order);
	ret = xa_erase(&array, 7); 
	printk("erase ret=%p, %d, %d\n",
		ret, ((struct item *)ret)->index, ((struct item *)ret)->order);
*/
	i = 9;
	ret = xa_find(&array, &i, ULONG_MAX, XA_PRESENT); 
	printk("find ret=%p, %d, %d\n",
		ret, ((struct item *)ret)->index, ((struct item *)ret)->order);
 
	xa_for_each(&array, i, ret) { 
		printk("each ret=%p, %d, %d\n",
			ret, ((struct item *)ret)->index, ((struct item *)ret)->order);
	}	
 
	xa_destroy(&array); 
	printk("xarray_test() end.\n");
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
