#ifndef XGATE_H
#define XGATE_H
#include "os_compat.h"

#define XGATE_SPEED			0x0001
#define XGATE_ODOMETER		0x0002
#define XGATE_INPUTS		0x0004
#define XGATE_VOLTAGE		0x0010
#define XGATE_TEMPERATURE	0x0020

void xgate_set_notification( int flag );

int xgate_get_notifications(char *buff, int size);

void xgate_send_notification( const char *name );

float speedo_get_kmph(void);
u8 inputs_get(void);
float voltage_get_input(void);
float temperature_get( int id );
u32 odometer_get(void);

#endif // XGATE_H
