#include <tm_stm32_adc.h>
#include "os_compat.h"
#include "config.h"

#define VALUE_ADC_VREF					3.3f
#define VALUE_DIV_R1_OHM				10000.0f
#define VALUE_DIV_R2_OHM				1290.0f

void voltage_init(void){
	TM_ADC_Init( ADC1, TM_ADC_Channel_8 );
	_D(("I: VOLTAGE module loaded ... OK\n"));
}

static uint16_t voltage_readADC1( TM_ADC_Channel_t ch ){
	//_D(("-- %d\n", TM_ADC_Read( ADC1, ch )));
	return TM_ADC_Read( ADC1, ch );
}

/**
 * get current volatge direct on ADC channel (based on Vref and )
 * retval (float)			- voltage
 */
static float voltage_get(void){
	return (VALUE_ADC_VREF / 4095.f) * voltage_readADC1( TM_ADC_Channel_8 );
}

float voltage_get_input(void){
	return ((voltage_get() * VALUE_DIV_R1_OHM) / VALUE_DIV_R2_OHM);
}
