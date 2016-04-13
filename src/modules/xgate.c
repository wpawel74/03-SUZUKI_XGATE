#include <string.h>
#include "os_compat.h"
#include "config.h"
#include "tm_stm32_delay.h"
#include "xgate.h"

static int req_notifications = 0x0000;

void xgate_set_notification( int flag ){
	req_notifications |= flag;
}

/**
 * TODO: something with validation size
 */
int xgate_get_notifications(char *buff, int size){
	memset( buff, 0, size );
	if( req_notifications & XGATE_SPEED && size > strlen(buff) )
		sprintf( buff + strlen(buff), "SPEED %d\n", (int)speedo_get_kmph() );
	if( req_notifications & XGATE_ODOMETER && size > strlen(buff) )
		sprintf( buff + strlen(buff), "ODOMETER %d\n", odometer_get() );
	if( req_notifications & XGATE_INPUTS && size > strlen(buff) )
		sprintf( buff + strlen(buff), "INPUTS %d\n", inputs_get() );
	if( req_notifications & XGATE_VOLTAGE && size > strlen(buff) )
		sprintf( buff + strlen(buff), "VOLTAGE %d\n", (int)(voltage_get_input()*1000.f) );
	if( req_notifications & XGATE_TEMPERATURE && size > strlen(buff) )
		sprintf( buff + strlen(buff), "TEMPERATURE %.f\n", temperature_get(0));

	// clear notifications
	req_notifications = 0;

	return strlen( buff );
}

void xgate_send_notification( ATTR_NON_NULL const char *name ){
	_D(("I: broadcast message: \"%s\"\n", name ));
}
