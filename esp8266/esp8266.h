#ifndef ESP8266_H
#define ESP8266_H

#include <stdint.h>

typedef enum
{
    ESP_RESPONSE_OK = 0,
    ESP_RESPONSE_ERROR,
    ESP_RESPONSE_BUSY,
    ESP_RESPONSE_LED_ON,
    ESP_RESPONSE_LED_OFF,
    ESP_RESPONSE_UNKNOWN
} ESP_RESPONSE;

typedef void (*ESP_ResponseHandler_t)(ESP_RESPONSE result);

/*Brief: ESP initialization
 * [in] - none
 * [out] - none
 * */
void ESP_Init(void);

/*Brief: ESP send commands
 * [in] - command - pointer to command
 * [out] - none
 * */
void ESP_SendCommand(const char* const command);

/*Brief: ESP register response handler
 * [in] - callback - callback
 * [out] - none
 * */
void ESP_RegisterResponseHandler(ESP_ResponseHandler_t callback);

#endif /* ESP8266_H */

