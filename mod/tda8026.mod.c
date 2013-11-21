#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
 .arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x88d2b212, "module_layout" },
	{ 0x12da5bb2, "__kmalloc" },
	{ 0xfbc74f64, "__copy_from_user" },
	{ 0x67c2fa54, "__copy_to_user" },
	{ 0x2e5810c6, "__aeabi_unwind_cpp_pr1" },
	{ 0xad8f4bf1, "del_timer" },
	{ 0x27bbf221, "disable_irq_nosync" },
	{ 0x1ed0b17e, "i2c_del_driver" },
	{ 0x9a87d4b1, "slab_buffer_size" },
	{ 0x9686698f, "omap_dm_timer_set_load_start" },
	{ 0xc362548b, "malloc_sizes" },
	{ 0xd95ffc0b, "omap_dm_timer_start" },
	{ 0xf3495634, "clk_disable" },
	{ 0x2ae04e01, "i2c_transfer" },
	{ 0x47229b5c, "gpio_request" },
	{ 0x3dfab183, "schedule_work" },
	{ 0x4aabc7c4, "__tracepoint_kmalloc" },
	{ 0x88f3dcfa, "down_interruptible" },
	{ 0x432fd7f6, "__gpio_set_value" },
	{ 0x2e1ca751, "clk_put" },
	{ 0xeae3dfd6, "__const_udelay" },
	{ 0x84438f1e, "init_timer_key" },
	{ 0x7d11c268, "jiffies" },
	{ 0xe707d823, "__aeabi_uidiv" },
	{ 0x5e23e398, "misc_register" },
	{ 0xfa2a45e, "__memzero" },
	{ 0x5f754e5a, "memset" },
	{ 0xda788071, "kmem_cache_alloc_notrace" },
	{ 0xea147363, "printk" },
	{ 0xa8f59416, "gpio_direction_output" },
	{ 0xf33d7b20, "mod_timer" },
	{ 0x2a37decb, "add_timer" },
	{ 0x859c6dc7, "request_threaded_irq" },
	{ 0xc49f3b43, "omap_dm_timer_request_specific" },
	{ 0xfd609b07, "i2c_register_driver" },
	{ 0xa900a632, "omap_dm_timer_free" },
	{ 0xe9ce8b95, "omap_ioremap" },
	{ 0x15331242, "omap_iounmap" },
	{ 0x62c818f, "omap_dm_timer_set_match" },
	{ 0xfe990052, "gpio_free" },
	{ 0x37a0cba, "kfree" },
	{ 0x9d669763, "memcpy" },
	{ 0x7507ec41, "omap_dm_timer_set_source" },
	{ 0x39a84c7b, "omap_dm_timer_set_pwm" },
	{ 0x16c7f7e4, "up" },
	{ 0x16f9b492, "lockdep_init_map" },
	{ 0xefd6cf06, "__aeabi_unwind_cpp_pr0" },
	{ 0xbf0b8fcc, "misc_deregister" },
	{ 0x9e7d6bd0, "__udelay" },
	{ 0xf20dabd8, "free_irq" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "533BB7E5866E52F63B9ACCB");
