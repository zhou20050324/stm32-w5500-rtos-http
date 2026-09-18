/**
 * @file	httpUtil.c
 * @brief	HTTP Server Utilities
 * @version 1.0
 * @date	2014/07/15
 * @par Revision
 *			2014/07/15 - 1.0 Release
 * @author
 * \n\n @par Copyright (C) 1998 - 2014 WIZnet. All rights reserved.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "httpUtil.h"
#include "adc.h"

#ifndef DATA_BUF_SIZE
#define DATA_BUF_SIZE 2048
#endif

uint8_t http_get_cgi_handler(uint8_t *uri_name, uint8_t *buf, uint32_t *file_len)
{
	uint8_t ret = HTTP_OK;
	uint16_t len = 0;

	if (predefined_get_cgi_processor(uri_name, buf, &len))
	{
		;
	}
	else if (strcmp((const char *)uri_name, "example.cgi") == 0)
	{
		// To do
		;
	}
	else
	{
		// CGI file not found
		ret = HTTP_FAILED;
	}

	if (ret)
		*file_len = len;
	return ret;
}

uint8_t http_post_cgi_handler(uint8_t *uri_name, st_http_request *p_http_request, uint8_t *buf, uint32_t *file_len)
{
	uint8_t ret = HTTP_OK;
	uint16_t len = 0;
	uint8_t val = 0;

	if (predefined_set_cgi_processor(uri_name, p_http_request->URI, buf, &len))
	{
		;
	}
	else if (strcmp((const char *)uri_name, "example.cgi") == 0)
	{
		// To do
		val = 1;
		len = sprintf((char *)buf, "%d", val);
	}
	else
	{
		// CGI file not found
		ret = HTTP_FAILED;
	}

	if (ret)
		*file_len = len;
	return ret;
}

uint8_t predefined_get_cgi_processor(uint8_t *uri_name, uint8_t *buf, uint16_t *len)
{
	// 引用 APP_freeRTOS.c 中的 DHT11 全局缓存
	extern float dht_temperature;
	extern float dht_humidity;
	extern uint8_t dht_valid;

	if (strcmp((const char *)uri_name, "data.json") == 0)
	{
		float mc_temp = Get_Internal_Temperature();
		float light_v = Get_Light_Voltage();
		float vref = Get_ADC_Reference_Voltage();

		*len = snprintf((char *)buf, DATA_BUF_SIZE,
						"{\"mcu_temp\":%.2f,\"light_v\":%.3f,\"vref\":%.3f,"
						"\"dht_temp\":%.1f,\"dht_humi\":%.1f}",
						mc_temp, light_v, vref,
						dht_valid ? dht_temperature : 0.0f,
						dht_valid ? dht_humidity : 0.0f);
		return 1; // 表示处理成功
	}
	// 其他 CGI 可以继续添加
	return 0;
}

uint8_t predefined_set_cgi_processor(uint8_t *uri_name, uint8_t *uri, uint8_t *buf, uint16_t *en)
{
	;
}
