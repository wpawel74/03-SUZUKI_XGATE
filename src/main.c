/* Include core modules */
#include "stm32fxxx_hal.h"
/* Include my libraries here */
#include "defines.h"
#include "tm_stm32_delay.h"
#include "tm_stm32_usart.h"
#include "config.h"
#ifdef WITH_SELFHOLD
#include "tm_stm32_gpio.h"
#endif // WITH_SELFHOLD

__attribute__ ((used)) int putchar(int ch) {
	/* Send over debug USART */
	TM_USART_Putc(USART1, ch);

	/* Return OK */
	return ch;
}

static double fabs( double n ){
	return (n >= 0) ? n: 0 - n;
}

#include "os_compat.h"
#include "xgate.h"

#define PROJECT_NAME	"SUZUKI_GATE"
#define AUTHOR			"Pablo <w_pawel74@tlen.pl>"

#ifdef WITH_VOLTAGE
#define IGNITION_DETECTION_TRIGER_LEVEL				4.0
static int ignition = 0;

/**
 * is ignition active or not. detected based on measure voltage
 * NOTE: see trigger level IGNITION_DETECTION_TRIGER_LEVEL in Volts
 */
int is_ignition(void){
	return ignition;
}
#endif // WITH_VOLTAGE

int main(void) {
	/* Init system */
	TM_RCC_InitSystem();

	TM_USART_Init(USART1, TM_USART_PinsPack_1, 115200);

	_D(("---------------------------------------------\n"));
	_D((" Project: %s\n", PROJECT_NAME));
	_D((" Author: %s\n", AUTHOR));
	_D((" Release date: %s %s\n", __DATE__, __TIME__));
	_D(("---------------------------------------------\n"));

	/* Init HAL layer */
	HAL_Init();

	/* Init delay */
	TM_DELAY_Init();

#ifdef WITH_ODOMETER
extern void odometer_init(void);
extern void odometer_poll(void);
extern void odometer_push_eeprom(void);
	odometer_init();
#endif // WITH_ODOMETER

#ifdef WITH_TEMPERATURE
extern void temperature_init(void);
extern void temperature_poll(void);
extern u8 owire_devices_count;
	while( 1 ) {
		temperature_init();
		if( owire_devices_count != 0 ) break;
	}
#endif // WITH_TEMPERATURE

#ifdef WITH_SPEEDO
extern void speedo_init(void);
extern float speedo_get_kmph(void);
	uint32_t	speedo_notification = 0;
	speedo_init();
#endif // WITH_SPEEDO

#ifdef WITH_VOLTAGE
extern void voltage_init(void);
extern float voltage_get_input(void);
	voltage_init();

#ifdef VOLTAGE_NOTIFICATION_PERIODICALLY
	uint32_t	voltage_notification = 0;
#define	VOLTAGE_NOTIFICATION_PERIOD_SECONDS			3
#endif // VOLTAGE_NOTIFICATION_PERIODICALLY

#ifdef VOLTAGE_NOTIFICATION_ABS
	float		voltage_last = voltage_get_input();
#define VOLTAGE_NOTIFICATION_HISTERESIS_VOLTAGE		0.3
#endif // VOLTAGE_NOTIFICATION_ABS

#endif // WITH_VOLTAGE

#ifdef WITH_INPUTS
extern void inputs_init(void);
extern void inputs_poll(void);
	inputs_init();
#endif // WITH_INPUTS

#ifdef WITH_WIFI_AP
extern void AP_init(void);
extern int AP_poll(void);
		AP_init();
#endif // WITH_WIFI_AP

#ifdef WITH_SELFHOLD
#	define OFF_DELAY_SEC			60
	u32 off_delay = HAL_GetTick();
	TM_GPIO_Init( GPIOA, GPIO_Pin_8, TM_GPIO_Mode_OUT, TM_GPIO_OType_PP, TM_GPIO_PuPd_UP, TM_GPIO_Speed_Low );
	TM_GPIO_SetPinHigh( GPIOA, GPIO_Pin_8 );

#	define SEFLHOLD_DISABLE()				TM_GPIO_SetPinLow(GPIOA, GPIO_Pin_8 )
#	define SEFLHOLD_IS_ENABLE()				(TM_GPIO_GetOutputPinValue( GPIOA, GPIO_Pin_8))
#endif // WITH_SELFHOLD

	while (1) {

#ifdef WITH_WIFI_AP
		AP_poll();
#endif // WITH_WIFI_AP

#ifdef WITH_INPUTS
		inputs_poll();
#endif // WITH_INPUTS

#ifdef WITH_SPEEDO
		if( HAL_GetTick() > (speedo_notification + 1 * 1000) ){
			char notification[ 25 ];
			sprintf( notification, "SPEED %d\n", (int)speedo_get_kmph() );
			xgate_set_notification( XGATE_SPEED );
			xgate_send_notification( notification );

			//_D(("D: speed %.6f [KMPH]\n", speedo_get_kmph()));
			speedo_notification = HAL_GetTick();
		}
#endif // WITH_SPEEDO

#ifdef WITH_TEMPERATURE
		temperature_poll();
#endif // WITH_TEMPERATURE

#ifdef WITH_ODOMETER
		odometer_poll();
#endif // WITH_ODOMETER

#ifdef WITH_VOLTAGE
		if( voltage_get_input() > IGNITION_DETECTION_TRIGER_LEVEL ){
			if( ignition == 0 ){
				char notification[ 15 ];
				sprintf( notification, "IGNITION %d\n", 1 );
				xgate_set_notification( XGATE_IGNITION );
				xgate_send_notification( notification );
			}
			ignition = 1;
#ifdef WITH_SELFHOLD
			off_delay = HAL_GetTick();
#endif // WITH_SELFHOLD
		} else {
			if( ignition == 1 ){
				char notification[ 15 ];
				sprintf( notification, "IGNITION %d\n", 0 );
				xgate_set_notification( XGATE_IGNITION );
				xgate_send_notification( notification );
			}
			ignition = 0;
		}

		if(
#ifdef VOLTAGE_NOTIFICATION_PERIODICALLY
			(HAL_GetTick() > (voltage_notification + (VOLTAGE_NOTIFICATION_PERIOD_SECONDS * 1000)))
#endif // VOLTAGE_NOTIFICATION_PERIODICALLY
#if defined VOLTAGE_NOTIFICATION_PERIODICALLY && defined VOLTAGE_NOTIFICATION_ABS
			||
#endif // defined VOLTAGE_NOTIFICATION_PERIODICALLY && defined VOLTAGE_NOTIFICATION_ABS
#ifdef VOLTAGE_NOTIFICATION_ABS
			((float)fabs( (double)(voltage_get_input() - voltage_last) )) > VOLTAGE_NOTIFICATION_HISTERESIS_VOLTAGE
#endif // VOLTAGE_NOTIFICATION_ABS
				){
			char notification[ 25 ];
			sprintf( notification, "VOLTAGE %d\n", (int)(voltage_get_input() * 1000.f) );
			xgate_set_notification( XGATE_VOLTAGE );
			xgate_send_notification( notification );

			_D(("D: voltage %.6f [V]\n", voltage_get_input()));
#ifdef VOLTAGE_NOTIFICATION_PERIODICALLY
			voltage_notification = HAL_GetTick();
#endif // VOLTAGE_NOTIFICATION_PERIODICALLY
#ifdef VOLTAGE_NOTIFICATION_ABS
			voltage_last = voltage_get_input();
#endif // VOLTAGE_NOTIFICATION_ABS
		}
#endif // WITH_VOLTAGE

#ifdef WITH_SELFHOLD
		if( HAL_GetTick() > (off_delay + (OFF_DELAY_SEC * 1000)) ){

#ifdef WITH_ODOMETER
			if( SEFLHOLD_IS_ENABLE() )
				odometer_push_eeprom();
#endif // WITH_ODOMETER

			_D(("D: Bye Bye! ...\n"));

			SEFLHOLD_DISABLE();
			off_delay = HAL_GetTick();
		}
#endif // WITH_SELFHOLD
		//_D(("-> %ld\n", HAL_GetTick()));
		//Delay(500000);
	}
}
