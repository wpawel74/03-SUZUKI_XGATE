/* Include core modules */
#include "stm32fxxx_hal.h"
/* Include my libraries here */
#include "defines.h"
#include "os_compat.h"

#include "esp8266.h"

typedef enum {
	WIFI_AP_INITIALISATION = 0x00,		// initialization ESP8266
	WIFI_AP_SET_MODE,					// set WIFI mode
	WIFI_AP_CONFIGURATION,				// WIFI configuration
	WIFI_AP_SERVER_ENABLE,				// enable server mode
	WIFI_AP_READY						//
} WIFI_AP_State_t;

/**-------------------------------------------------
 *             Server configuration
 *--------------------------------------------------*/
#define AP_SERVER_PORT					80
#define SERVER_MAX_SEND_BYTES			50

/**-------------------------------------------------
 *               usedfull macros
 *--------------------------------------------------*/
#define WAIT_WHILE(ttw)					AP_delay = HAL_GetTick() + ttw;
#define NEXT_STEP_IF_ESPOK(next_step)	{ if(result != ESP_OK) { WAIT_WHILE( 5000 ); } else { AP_state = next_step; } }

/* ESP8266 working structure */
static ESP8266_t						ESP8266;
static WIFI_AP_State_t					AP_state = WIFI_AP_INITIALISATION;
static uint32_t							AP_delay = 0;
static uint32_t							AP_breath = 0;

/**--------------------------------------------------
 *            wrapper for write method
 *---------------------------------------------------*/
typedef enum {
	ESP8266_CONN_CLOSE = 0x00,
	ESP8266_CONN_OPEN = 0x01,
	ESP8266_CONN_PENDING = 0x02
} AP_Conn_Status_t;

typedef struct AP_HLC {
	ESP8266_t				*ESP8266;							// hook to esp8266
	ESP8266_Connection_t	*Connection;						// hook to current opened connection
	AP_Conn_Status_t		ConnStatus;							// wrapper status
	char					Data[ SERVER_MAX_SEND_BYTES ];		// output buffer
} AP_HLC_t;

AP_HLC_t ESP8266_HLC = {
	.ESP8266 = &ESP8266,
	.Connection = 0,
	.ConnStatus = ESP8266_CONN_CLOSE
};

static void AP_SendData( AP_HLC_t *ESP8266_HLC, const char *Data, int Size ){

	if( !ESP8266_HLC->Connection || Size > sizeof(ESP8266_HLC->Data) )
		return;

	if( ESP8266_HLC->ConnStatus != ESP8266_CONN_OPEN )
		return;

	memcpy( ESP8266_HLC->Data, Data, Size );
	ESP8266_RequestSendBytes(ESP8266_HLC->ESP8266, ESP8266_HLC->Connection, Size);
	ESP8266_HLC->ConnStatus = ESP8266_CONN_PENDING;
	return;
}

/**--------------------------------------------------
 *                   Reactor
 *---------------------------------------------------*/

void AP_init(void) {

	/* Display message */
	_D(("I: ESP8266 AT commands parser loaded ... OK\n"));

}

void AP_poll(void) {
	ESP8266_Result_t	result;

	if( AP_delay > HAL_GetTick() )
		return;

	switch( AP_state ){
	case WIFI_AP_INITIALISATION:
		// initialize ESP strucrure
		memset( &ESP8266, 0, sizeof(ESP8266) );

		// clear write meta data ...
		ESP8266_HLC.ESP8266 = &ESP8266;
		ESP8266_HLC.Connection = 0;
		ESP8266_HLC.ConnStatus = ESP8266_CONN_CLOSE;

		/* Init ESP module */
		result = ESP8266_Init(&ESP8266, 9600);
		NEXT_STEP_IF_ESPOK(WIFI_AP_SET_MODE);
		_D(("I: ESP8266 initialization ... %s\n", result != ESP_OK ? "FAIL": "OK"));
		break;

	case WIFI_AP_SET_MODE:
		/* Set mode to STA+AP */
		result = ESP8266_SetMode(&ESP8266, ESP8266_Mode_STA_AP);
		NEXT_STEP_IF_ESPOK(WIFI_AP_CONFIGURATION);
		_D(("I: ESP8266 set mode: AP ... %s\n", result != ESP_OK ? "FAIL": "OK"));
		break;

	case WIFI_AP_CONFIGURATION: {
		/* AP configuration */
		ESP8266_APConfig_t ap_conf = {
				.SSID = "SUZUKI_XGATE",	/*!< Network public name for ESP AP mode */
				.Pass = "",             /*!< Network password for ESP AP mode */
				.Ecn = ESP8266_Ecn_OPEN,/*!< Security of Wi-Fi spot. This parameter can be a value of \ref ESP8266_Ecn_t enumeration */
				.Channel = 1,           /*!< Channel Wi-Fi is operating at */
				.MaxConnections = 1,    /*!< Max number of stations that are allowed to connect to ESP AP, between 1 and 4 */
				.Hidden = 0             /*!< Set to 1 if network is hidden (not broadcast) or zero if noz */
		};
		result = ESP8266_SetAP(&ESP8266, &ap_conf);
		NEXT_STEP_IF_ESPOK(WIFI_AP_SERVER_ENABLE);
		_D(("I: WIFI configuration SSID %s Pass %s ... %s\n", ap_conf.SSID, ap_conf.Pass, result != ESP_OK ? "FAIL": "OK"));
		} break;

	case WIFI_AP_SERVER_ENABLE:
		/* Enable server on port 80 */
		result = ESP8266_ServerEnable(&ESP8266, AP_SERVER_PORT);
		NEXT_STEP_IF_ESPOK(WIFI_AP_READY);
		_D(("I: ESP8266 enable server on port %d ... %s\n", AP_SERVER_PORT, result != ESP_OK ? "FAIL": "OK"));
		break;

	case WIFI_AP_READY:
		/* Update ESP module */
		ESP8266_Update(&ESP8266);

		if( HAL_GetTick() > AP_breath ){
			AP_SendData(&ESP8266_HLC, "0123456789", 10);
			AP_breath = HAL_GetTick() + 20;
		}
		break;
	}
}

/************************************/
/*       ESP8266 TIME UPDATE        */
/************************************/

/* 1ms handler */
void TM_DELAY_1msHandler() {
	/* Update ESP8266 library time for 1 ms */
	ESP8266_TimeUpdate(&ESP8266, 1);
}

/************************************/
/*         ESP8266 CALLBACKS        */
/************************************/

/* Called when ready string detected */
void ESP8266_Callback_DeviceReady(ESP8266_t* ESP8266) {
	printf("I: Device is ready\n");
	if( AP_state != WIFI_AP_INITIALISATION )
		/**
		 * it's mean that esp8266 was unexpectedly reseted
		 */
		AP_state = WIFI_AP_INITIALISATION;
}

/* Called when watchdog reset on ESP8266 is detected */
void ESP8266_Callback_WatchdogReset(ESP8266_t* ESP8266) {
	printf("E: Watchdog reset detected!\n");
	AP_state = WIFI_AP_INITIALISATION;
}

/************************************/
/*         SERVER CALLBACKS         */
/************************************/

void ESP8266_Callback_ServerConnectionActive(ESP8266_t* ESP8266, ESP8266_Connection_t* Connection) {
	Connection->UserParameters = &ESP8266_HLC;
	((AP_HLC_t *) Connection->UserParameters)->Connection = Connection;
	((AP_HLC_t *) Connection->UserParameters)->ConnStatus = ESP8266_CONN_OPEN;
	printf("I: ServerConnectionActive connection: %d\n", Connection->Number);
}

/* Called when "x,CLOSED" is detected */
void ESP8266_Callback_ServerConnectionClosed(ESP8266_t* ESP8266, ESP8266_Connection_t* Connection) {
	printf("I: ServerConnectionClosed connection: %d\n", Connection->Number);
	((AP_HLC_t *) Connection->UserParameters)->ConnStatus = ESP8266_CONN_CLOSE;
}

/* Called when "+IPD..." is detected */
void ESP8266_Callback_ServerConnectionDataReceived(ESP8266_t* ESP8266, ESP8266_Connection_t* Connection, char* Buffer) {
	printf("I: ServerConnectionReceived connection: %d\n", Connection->Number);
}

/* Called when user should fill data buffer to be sent with connection */
uint16_t ESP8266_Callback_ServerConnectionSendData(ESP8266_t* ESP8266, ESP8266_Connection_t* Connection, char* Buffer, uint16_t max_buffer_size) {
	AP_HLC_t *ESP8266_HLC = (AP_HLC_t *) Connection->UserParameters;

	printf("I: ServerConnectionSendData connection: %d\n", Connection->Number);

	/**
	 * for old esp8266 firmware we need to push the same bytes like in
	 * max_buffer_size otherwise communication will be hang up
	 */
	memcpy( Buffer, ESP8266_HLC->Data, max_buffer_size );
	return max_buffer_size;
}

/* Called when data are send successfully */
void ESP8266_Callback_ServerConnectionDataSent(ESP8266_t* ESP8266, ESP8266_Connection_t* Connection) {
	AP_HLC_t *ESP8266_HLC = (AP_HLC_t *) Connection->UserParameters;
	ESP8266_HLC->ConnStatus = ESP8266_CONN_OPEN;

	printf("I: ServerConnectionDataSent connection: %d\n", Connection->Number);
}

/* Called when error returned trying to sent data */
void ESP8266_Callback_ClientConnectionDataSentError(ESP8266_t* ESP8266, ESP8266_Connection_t* Connection) {
	AP_HLC_t *ESP8266_HLC = (AP_HLC_t *) Connection->UserParameters;
	ESP8266_HLC->ConnStatus = ESP8266_CONN_CLOSE;
	Connection->Active = 0;

	printf("I: ServerConnectionDataSentError connection: %d\n", Connection->Number);
}
