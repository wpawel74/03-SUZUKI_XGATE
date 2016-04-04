#include <tm_stm32_exti.h>
#include "os_compat.h"
#include "config.h"

volatile static uint32_t speedo_ticks = 0;
volatile static uint32_t speedo_tick_irq = 0;
static float RADIUS_METERS = 30.0; // meters
static uint32_t SYSCLK_HZ = 2;

void speedo_init(void){
	SYSCLK_HZ = HAL_RCC_GetSysClockFreq();
	/* Attach EXTI pin, enable both edges because of different boards support */
	if (TM_EXTI_Attach(GPIOA, GPIO_PIN_0, TM_EXTI_Trigger_Rising) == TM_EXTI_Result_Ok) {
		_D(("I: SPEEDO (SYSCLK %ldHz) module loaded ... OK\n", SYSCLK_HZ));
	} else {
		_D(("I: SPEEDO module loaded ... FAILED\n"));
	}
}

/* Handle all EXTI lines */
void TM_EXTI_Handler(uint16_t GPIO_Pin) {
	/* Check proper line */
	if (GPIO_Pin == GPIO_PIN_0) {
		uint32_t tmp_tick = HAL_GetTick();

		if( tmp_tick > speedo_tick_irq )
			speedo_ticks = tmp_tick - speedo_tick_irq;
		else
			speedo_ticks = tmp_tick + (0xffffffff - speedo_tick_irq);

		speedo_tick_irq = tmp_tick;

#ifdef WITH_ODOMETER
		extern u32 odometer_irqc;
		odometer_irqc++;
#endif // WITH_ODOMETER

	}
}

float speedo_get_kmph(void){
	return  (SYSCLK_HZ * 1.f)/(speedo_ticks * 1.f) * 3600.f * (RADIUS_METERS / 1000.f);
}
