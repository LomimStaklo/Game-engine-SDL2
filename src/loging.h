#ifndef _LOGING_H
#define _LOGING_H

#include <stdio.h>
//#include "macros.h"

#define GAME_LOG_FILE stderr

#define game_log(log_type, fmt, ...) \
    fprintf(GAME_LOG_FILE,"\033[31m [" log_type "]:\033[0m"" %s:%d:%s(): " fmt "\n", \
    __FILE__, __LINE__, __func__, __VA_ARGS__)

#ifdef ENABLE_TIME_LOGING
#include <time.h>

#define game_log_timer(__expr_, __fmt_, ...) \
do { \
    double _pp_start = (double)clock() / CLOCKS_PER_SEC; \
    __expr_ ; \
    double _pp_end = (double)clock() / CLOCKS_PER_SEC; \
    game_log(TIMER, "%.3f sec " __fmt_ "\n", \
    _pp_end - _pp_start, __VA_ARGS__); \
} while (0)

#endif /* ENABLE_TIME_LOGING */

#endif /* !_LOGING_H */
