#ifndef __DEBUG_H__

#ifdef __PNET_DEBUG__
#define dbg_printf printf
#else
#define dbg_printf(...)
#endif

#ifdef __PNET_ALL_DEBUG__
#define dbg_all_printf printf
#else
#define dbg_all_printf(...)
#endif

#endif

