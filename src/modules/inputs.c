#include "os_compat.h"
#include "config.h"
#include "tm_stm32_delay.h"

#ifdef WITH_PCF8574
#include "stm32_PCF8574.h"
#define PCF8574_GPIO_HIGH_BEAM					0x01
#define PCF8574_GPIO_LOW_BEAM					0x02
#define PCF8574_GPIO_TURN_LIGHT					0x04
#define PCF8574_GPIO_ALERT						0x08
#define PCF8574_GPIO_ENGINE_WARNING				0x10
#endif // WITH_PCF8574

void inputs_init(void){
#ifdef WITH_PCF8574
	PW_PCF8574_Init( I2C1 );
#endif // WITH_PCF8574
	_D(("I: INPUTS module loaded ... OK\n"));
}

static void inputs_send_notification( ATTR_NON_NULL const char *name ){
	_D(("I: broadcast message: \"%s\"\n", name ));
}

void inputs_poll(void){
#ifdef WITH_PCF8574
	uint8_t ext_gpio = 0;
	static uint8_t inputs_gpio = 0;
	static uint32_t next_time = 0;

	PW_PCF8574_ReadPort( I2C1, &ext_gpio );

	if( (ext_gpio & PCF8574_GPIO_HIGH_BEAM) ^ ( inputs_gpio & PCF8574_GPIO_HIGH_BEAM) ){
		if( ext_gpio & PCF8574_GPIO_HIGH_BEAM )
			inputs_send_notification( "" );
		else
			inputs_send_notification( "" );
	}
	if( (ext_gpio & PCF8574_GPIO_LOW_BEAM) ^ ( inputs_gpio & PCF8574_GPIO_LOW_BEAM) ){
		if( ext_gpio & PCF8574_GPIO_LOW_BEAM )
			inputs_send_notification( "" );
		else
			inputs_send_notification( "" );
	}
	if( (ext_gpio & PCF8574_GPIO_TURN_LIGHT) ^ ( inputs_gpio & PCF8574_GPIO_TURN_LIGHT) ){
		if( ext_gpio & PCF8574_GPIO_TURN_LIGHT )
			inputs_send_notification( "" );
		else
			inputs_send_notification( "" );
	}
	if( (ext_gpio & PCF8574_GPIO_ALERT) ^ ( inputs_gpio & PCF8574_GPIO_ALERT) ){
		if( ext_gpio & PCF8574_GPIO_ALERT )
			inputs_send_notification( "" );
		else
			inputs_send_notification( "" );
	}
	if( (ext_gpio & PCF8574_GPIO_ENGINE_WARNING) ^ ( inputs_gpio & PCF8574_GPIO_ENGINE_WARNING) ){
		if( ext_gpio & PCF8574_GPIO_ENGINE_WARNING )
			inputs_send_notification( "" );
		else
			inputs_send_notification( "" );
	}

	inputs_gpio = ext_gpio;
	if( HAL_GetTick() > next_time ){
		_D(("I: [%ld] PCF8574 [GPIO/R] %x\n", HAL_GetTick(), inputs_gpio ));
		next_time = HAL_GetTick() + (3 * 1000);
	}
#endif // WITH_PCF8574
}
