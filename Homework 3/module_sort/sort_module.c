#include "linux/init.h"
#include "linux/module.h"
#include "linux/kernel.h"
#include "linux/timer.h"
#include "linux/list.h"
#include "linux/sort.h"
#include "linux/slab.h"

static char* animals[] = {"dog", "cat", "bee", "bat","dolphin",
		     	 "elephant", "frog", "whale", "tiger", "giraffe",
			 "squirrel", "ant", "wasp", "bat", "tiger",
			 "cat", "whale", "dog", "giraffe", "elephant",
			 "bat", "giraffe", "penguin", "whale", "dolphin",
			 "dog", "penguin", "squirrel", "frog", "dolphin",
			 "penguin", "dolphin", "dolphin", "elephant", "dog",
			 "giraffe", "whale", "penguin", "tiger", "frog",
			 "bee", "dolphin", "wasp", "frog", "dog",
			 "bat", "bee", "penguin", "ant", "penguin"};

struct pest {
		uint8_t count;
		char* ani_name;
		struct list_head list;
};
struct pest *many_pest;
struct pest *head;
struct pest *filtered_pest;
struct pest *filtered_head;
char* param_char = "none";
int param_count = 0;
uint8_t n;
int n2 = 50;
uint8_t i;
unsigned long start, stop;

static int32_t compar(const void* a, const void* b)
{
	return strcmp (*(const char**)a, *(const char**)b);
}

static int sorter_init(void)
{
	start = jiffies;
	printk(KERN_ALERT "\nWelcome to Sarthak's zoo sorting module\n");

	n = sizeof(animals)/sizeof(animals[0]);

	printk("Given array is: \n");
	for(i=0; i<n; i++)
	{
		printk("Animal %d: %s\n", (i+1), animals[i]);
	}
	printk("\n");

	sort(animals, n, sizeof(animals[0]), compar, NULL);

	printk("Sorted array is: \n");
	for(i=0; i<n; i++)
	{
		printk("Animal %d: %s\n", (i+1), animals[i]);
	}
	printk("\n");


	head = kmalloc(sizeof(struct pest), GFP_KERNEL);
	INIT_LIST_HEAD(&head->list);
	many_pest = kmalloc(sizeof(struct pest), GFP_KERNEL);
	many_pest->count = 0;
	list_add_tail(&many_pest->list, &head->list);

	for(i=0;i<n;i++)
	{
		many_pest->ani_name = animals[i];
		(many_pest->count)++;
		if((i!=n-1) && (animals[i] == animals[i+1]));
		else
		{
			if((i!=n-1))
			{
				many_pest = kmalloc(sizeof(struct pest), GFP_KERNEL);
				many_pest->count = 0;			
				list_add_tail(&many_pest->list, &head->list);
			}
			else;
		}
	}

	filtered_head = kmalloc(sizeof(struct pest), GFP_KERNEL);
	INIT_LIST_HEAD(&filtered_head->list);
	i = 0;
	printk("Linked list of animals:");
	list_for_each_entry(many_pest, &(head->list), list)
	{	
		i++;
		printk(KERN_ALERT "Animal is %s with count of %d\n", many_pest->ani_name, many_pest->count);
		if((many_pest->count>param_count) && (!(strcmp(param_char, "none")) || (!strcmp(param_char, many_pest->ani_name))))
		{
			filtered_pest = kmalloc(sizeof(struct pest), GFP_KERNEL);
			filtered_pest->ani_name = many_pest->ani_name;			
			filtered_pest->count = many_pest->count;
			list_add_tail(&filtered_pest->list, &filtered_head->list);
		}
	}
	printk("Number of nodes: %d\n\n", i);
	printk("Total amount of memory allocated to unique list is: %d bytes\n", i*sizeof(*many_pest));

	i = 0;
	printk("Filtered list of animals:");
	list_for_each_entry(filtered_pest, &(filtered_head->list), list)
	{	
		i++;
		printk(KERN_ALERT "Animal is %s with count of %d\n", filtered_pest->ani_name, filtered_pest->count);
	}
	printk("Number of filtered nodes: %d\n\n", i);
	printk("Total amount of memory  allocated to filtered list is: %d bytes\n", i*sizeof(*filtered_pest));

	stop = jiffies;
	stop = (stop-start)*1000/HZ;
	printk("\nTime for module insertion = %lu milliseconds\n", stop);

	return 0;		
}

module_param_array(animals, charp, &n2, 0);
module_param(param_char, charp, 0);
module_param(param_count, int, 0);

static void sorter_exit(void)
{
	start = jiffies;
	i=0;
	list_for_each_entry(many_pest, &(head->list), list)
	{	
		i++;
		kfree(many_pest);	
	}
	printk("\nTotal amount of memory freed by unique list is: %d bytes\n", i*sizeof(*many_pest));

	i=0;
	list_for_each_entry(filtered_pest, &(filtered_head->list), list)
	{	
		i++;
		kfree(filtered_pest);	
	}
	printk("Total amount of memory freed by filtered list is: %d bytes\n", i*sizeof(*filtered_pest));

	stop = jiffies;
	stop = (stop-start)*1000/HZ;
	printk("\nTime for module removal = %lu milliseconds\n", stop);

	printk(KERN_ALERT "\nThe Animals shall miss you:(\n");
}

module_init(sorter_init);
module_exit(sorter_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SARTHAK JAIN");
MODULE_DESCRIPTION("THE ZOO SORTER");
