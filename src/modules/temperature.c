#include "os_compat.h"
#include "config.h"
#include "sensors.h"
#include "tm_stm32_delay.h"
#include "xgate.h"

#ifdef WITH_DS18B20
#include "tm_stm32_ds18b20.h"
#include "tm_stm32_onewire.h"
#endif // WITH_DS18B20

#include <stdio.h>

#ifdef WITH_DS18B20
#define DS18B20_SCAN
static float ds18b20_temps[EXPECTING_SENSORS];
static ROM roms[EXPECTING_SENSORS] = {
#ifndef DS18B20_SCAN
	{ .rom = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 } },
#endif // DS18B20_SCAN
};

u8 owire_devices_count =
#ifndef DS18B20_SCAN
		EXPECTING_SENSORS
#else
		0
#endif // DS18B20_SCAN
		;
static TM_OneWire_t OneWire1;
static u32 ds18b20_read_time = 0;

static int DS18D20_SearchSensors( ROM roms[], int size ){
	owire_devices_count = 0;
	uint8_t devices = TM_OneWire_First(&OneWire1);
	while (devices) {
		/* Increase counter */
		owire_devices_count++;

		/* Get full ROM value, 8 bytes, give location of first byte where to save */
		TM_OneWire_GetFullROM(&OneWire1, roms[owire_devices_count - 1].rom);

		/* Get next device */
		devices = TM_OneWire_Next(&OneWire1);
	}
	return owire_devices_count;
}

static void temperature_notification( int i, float val_temp ){
	char notification[ 80 ];
	sprintf( notification, "TEMPERATURE %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x %f\n",
							roms[i].rom[0], roms[i].rom[1], roms[i].rom[2], roms[i].rom[3],
							roms[i].rom[4], roms[i].rom[5], roms[i].rom[6], roms[i].rom[7],
							val_temp);
	xgate_set_notification( XGATE_TEMPERATURE );
	xgate_send_notification( notification );
}

float temperature_get( int id ){
	return ds18b20_temps[id];
}

static void DS18D20_ReadTemp(void){
	int i;
	/* Start temperature conversion on all devices on one bus */
	TM_DS18B20_StartAll(&OneWire1);

	/* Wait until all are done on one onewire port */
	while (!TM_DS18B20_AllDone(&OneWire1));

	/* Read temperature from each device separatelly */
	for (i = 0; i < owire_devices_count; i++) {
		/* Read temperature from ROM address and store it to temps variable */
		if (TM_DS18B20_Read(&OneWire1, roms[i].rom, &ds18b20_temps[i])) {
			/* Print temperature */
			_D(("[DS18B20] ROM>%d: Temp %f\n", i, ds18b20_temps[i]));
			temperature_notification( i, ds18b20_temps[i] );
		} else {
			/* Reading error */
			_D(("[DS18B20] Reading error ROM>%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n",
									roms[i].rom[0], roms[i].rom[1], roms[i].rom[2], roms[i].rom[3],
									roms[i].rom[4], roms[i].rom[5], roms[i].rom[6], roms[i].rom[7]));
		}
	}
}

#endif // WITH_DS18B20

void temperature_init(void){
#ifdef WITH_DS18B20
	TM_OneWire_Init( &OneWire1, GPIOC, GPIO_Pin_13 );

#ifdef DS18B20_SCAN
	owire_devices_count = DS18D20_SearchSensors( roms, sizeof(roms)/sizeof(ROM) );
#endif // DS18B20_SCAN
	int i;
	/* Go through all connected devices and set resolution to 12bits */
	for (i = 0; i < owire_devices_count; i++) {
		/* Set resolution to 12bits */
		TM_DS18B20_SetResolution(&OneWire1, roms[i].rom, TM_DS18B20_Resolution_12bits);
	}

	for (i = 0; i < owire_devices_count; i++) {

		_D(("[DS18B20] ROM> %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x ... %s\n",
						roms[i].rom[0], roms[i].rom[1], roms[i].rom[2], roms[i].rom[3],
						roms[i].rom[4], roms[i].rom[5], roms[i].rom[6], roms[i].rom[7],
						TM_DS18B20_Is( roms[i].rom ) ? "online": "offline"));
	}
#endif // WITH_DS18B20
	_D(("I: TEMPERATURE module loaded ... OK\n"));
}

void temperature_poll(void){
#ifdef WITH_DS18B20
	if( HAL_GetTick() > (ds18b20_read_time + (1 * 1000)) ){
		DS18D20_ReadTemp();
		ds18b20_read_time = HAL_GetTick();
	}
#endif // WITH_DS18B20
}
