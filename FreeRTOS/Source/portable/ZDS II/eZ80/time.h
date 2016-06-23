
#ifndef _TIME_H_
#define _TIME_H_
#include <stdint.h>

typedef uint32_t	time_t;

extern time_t iTimeZone;

typedef struct {
	uint16_t tm_year;
	uint8_t  tm_mon;
	uint8_t  tm_mday;
	uint8_t  tm_hour;
	uint8_t  tm_min;
	uint8_t  tm_sec;
} FF_TimeStruct_t;

time_t FreeRTOS_time( time_t *t);
time_t FreeRTOS_get_secs_msec( time_t *t);
time_t FreeRTOS_set_secs_msec( time_t *uxCurrentSeconds, time_t *uxCurrentMS );

void FreeRTOS_gmtime_r( time_t *uxCurrentSeconds, FF_TimeStruct_t *xTimeStruct );

void vStartNTPTask( uint16_t usTaskStackSize, UBaseType_t uxTaskPriority );

#endif	/* _TIME_H_ */
