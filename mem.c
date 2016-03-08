#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");

struct test_struct {
	int num;
};

void testobj_constructor(void *buf)
{
	struct test_struct *obj = (struct test_struct *)buf;
	obj->num = 0;
}

int my_init(void)
{
	struct test_struct *obj = NULL;
	struct kmem_cache *cache = NULL;
	int err = 0;

	printk(KERN_INFO"Creating new cache");
	cache = kmem_cache_create("test_cache", sizeof(struct test_struct),
				  0,
				  (SLAB_RECLAIM_ACCOUNT|
                                   SLAB_PANIC|SLAB_MEM_SPREAD|SLAB_NOLEAKTRACE),
				  testobj_constructor);
	obj = kmem_cache_alloc(cache, GFP_KERNEL);
	obj->num = 12;
	printk(KERN_INFO"int = %d", obj->num);
	kmem_cache_free(cache, obj);
	kmem_cache_destroy(cache);
	return err;
}

void my_exit(void)
{
	return ;
}

module_init(my_init);
module_exit(my_exit);
