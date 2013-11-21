

#ifndef DEBUG_H
#define DEBUG_H

#include <linux/kernel.h>




#define LEVEL1 			1
#define LEVEL2 			2
#define LEVEL3 			3
#define LEVEL4 			4



#define iprintk(level, arg...)		printk(arg)

#define TRACE_TO(arg...)	iprintk(LEVEL1, arg)
#define WARN_TO(arg...)		iprintk(LEVEL2, arg)
#define ERROR_TO(arg...)	iprintk(LEVEL3, arg)
#define INFO_TO(arg...)		iprintk(LEVEL4, arg)



#endif
