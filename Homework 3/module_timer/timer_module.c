#include "linux/init.h"
#include "linux/module.h"
#include "linux/kernel.h"
#include "linux/timer.h"

struct timer_list sart_timer;
char* name = "Sarthak";
static int count;
static long time_p = 500;
void timer_handler(unsigned long);

static int timer_init(void)
{
	printk(KERN_ALERT "\nWelcome to Sarthak's timer module!!\n");
	sart_timer.expires = jiffies + time_p*HZ/1000;
	sart_timer.data = 0;
	sart_timer.function = timer_handler;
	init_timer(&sart_timer);
	add_timer(&sart_timer);	
	return 0;
}

module_param(name, charp, 0);
module_param(time_p, long, 0);

void timer_handler(unsigned long data)
{
	count++;
	printk(KERN_ALERT "\nUser is %s\n", name);
	printk(KERN_ALERT "Count is: %d\n", count);
	sart_timer.expires = jiffies + time_p*HZ/1000;
	sart_timer.data = 0;
	sart_timer.function = timer_handler;
	add_timer(&sart_timer);
}

static void timer_exit(void)
{
	printk(KERN_ALERT "\nGoodbye from your now old module:(\n");
	del_timer_sync(&sart_timer);
}

module_init(timer_init);
module_exit(timer_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("SARTHAK");
MODULE_DESCRIPTION("HELLO WORLD");
