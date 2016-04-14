#ifndef XGATE_H
#define XGATE_H
#include "os_compat.h"

#define XGATE_SPEED			0x0001
#define XGATE_ODOMETER		0x0002
#define XGATE_INPUTS		0x0004
#define XGATE_VOLTAGE		0x0010
#define XGATE_TEMPERATURE	0x0020
#define XGATE_IGNITION		0x0040

void xgate_set_notification( int flag );

int xgate_get_notifications(char *buff, int size);

void xgate_send_notification( const char *name );

#ifdef WITH_SPEEDO
float speedo_get_kmph(void);
#endif // WITH_SPEEDO
#ifdef WITH_INPUTS
u16 inputs_get(void);
#endif // WITH_INPUTS
#ifdef WITH_VOLTAGE
float voltage_get_input(void);
int is_ignition(void);
#endif // WITH_VOLTAGE
#ifdef WITH_TEMPERATURE
float temperature_get( int id );
#endif // WITH_TEMPERATURE
#ifdef WITH_ODOMETER
u32 odometer_get(void);
#endif // WITH_ODOMETER

#endif // XGATE_H
