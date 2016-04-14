#include "os_compat.h"
#include "config.h"
#include "xgate.h"
#include "tm_stm32_delay.h"

#ifdef WITH_PCF8574
#include "stm32_PCF8574.h"
#define PCF8574_GPIO_HIGH_BEAM					0x0001
#define PCF8574_GPIO_LOW_BEAM					0x0002
#define PCF8574_GPIO_TURN_LIGHT					0x0004
#define PCF8574_GPIO_ALERT						0x0008
#define PCF8574_GPIO_ENGINE_WARNING				0x0010
#define PCF8574_GPIO_FUEL_WARNING				0x0020
#define PCF8574_GPIO_TEMP_WARNING				0x0040
#define PCF8574_GPIO_BATTERY_WARNING			0x0080
#endif // WITH_PCF8574

void inputs_init(void){
#ifdef WITH_PCF8574
	PW_PCF8574_Init( I2C1 );
#endif // WITH_PCF8574
	_D(("I: INPUTS module loaded ... OK\n"));
}

static u16 inputs_gpio = 0;

u16 inputs_get(void){
	return inputs_gpio;
}

void inputs_poll(void){
#ifdef WITH_PCF8574
	uint8_t ext_gpio = 0;
	static uint32_t next_time = 0;

	PW_PCF8574_ReadPort( I2C1, &ext_gpio );

	if( (ext_gpio & PCF8574_GPIO_HIGH_BEAM) ^ ( inputs_gpio & PCF8574_GPIO_HIGH_BEAM) ||
		(ext_gpio & PCF8574_GPIO_LOW_BEAM) ^ ( inputs_gpio & PCF8574_GPIO_LOW_BEAM) ||
		(ext_gpio & PCF8574_GPIO_TURN_LIGHT) ^ ( inputs_gpio & PCF8574_GPIO_TURN_LIGHT) ||
		(ext_gpio & PCF8574_GPIO_ALERT) ^ ( inputs_gpio & PCF8574_GPIO_ALERT) ||
		(ext_gpio & PCF8574_GPIO_ENGINE_WARNING) ^ ( inputs_gpio & PCF8574_GPIO_ENGINE_WARNING) ||
		HAL_GetTick() > next_time ){

		char notification[ 15 ];
		sprintf( notification, "INPUT %d\n", ext_gpio );
		xgate_set_notification( XGATE_INPUTS );
		xgate_send_notification( notification );
	}

	inputs_gpio = ext_gpio;
	if( HAL_GetTick() > next_time ){
		_D(("I: [%ld] PCF8574 [GPIO/R] %x\n", HAL_GetTick(), inputs_gpio ));
		next_time = HAL_GetTick() + (3 * 1000);
	}
#endif // WITH_PCF8574
}
