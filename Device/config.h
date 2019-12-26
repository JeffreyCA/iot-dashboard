#ifndef CONFIG_H
#define CONFIG_H

// User-configurable values
#define CURRENCY_1 "USD"
#define CURRENCY_2 "CAD"
#define STOCK_TICKER "MSFT"
#define WEATHER_LOCATION "Toronto"

// Other constants
#define CURRENCY_PAIR CURRENCY_1 "/" CURRENCY_2

#define MAX_LINE_BUFFER_SIZE 17
#define MAX_BUFFER_SIZE 65

#define SPLASH_DURATION_MS 1500
#define UPDATE_INTERVAL_MS 1000
#define LED_FLASH_INTERVAL 1
#define FAIL_REFRESH_INTERVAL 10
#define REFRESH_INTERVAL 120

#endif // CONFIG_H
