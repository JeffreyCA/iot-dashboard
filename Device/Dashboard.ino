#include "Arduino.h";

#include "AZ3166WiFi.h";
#include "RGB_LED.h";
#include "arduinojson.h";
#include "http_client.h";

#include "config.h";

enum RATE_CHANGE { DECREASE, SAME, INCREASE };

static unsigned char AZURE_BMP[] = {
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   128, 192, 192, 224, 240, 56,  12,  192,
    240, 224, 192, 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   128, 224, 224, 224, 224,
    128, 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   192, 224, 248, 252, 254, 255,
    255, 63,  15,  3,   64,  248, 254, 255, 255, 255, 255, 255, 252, 248, 224,
    128, 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    128, 240, 252, 127, 31,  3,   3,   15,  127, 254, 240, 192, 0,   0,   0,
    0,   8,   28,  28,  28,  28,  156, 220, 252, 124, 60,  28,  0,   0,   0,
    252, 252, 248, 0,   0,   0,   0,   0,   0,   252, 252, 248, 0,   0,   0,
    0,   252, 252, 252, 112, 56,  28,  28,  28,  0,   128, 224, 240, 248, 28,
    28,  12,  12,  28,  248, 248, 240, 128, 0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   128, 224, 248,
    252, 255, 255, 255, 255, 63,  15,  3,   0,   0,   0,   0,   0,   0,   3,
    7,   15,  31,  63,  127, 255, 255, 255, 255, 255, 252, 240, 224, 128, 0,
    0,   0,   0,   0,   0,   64,  112, 124, 127, 31,  7,   7,   7,   7,   7,
    7,   7,   7,   31,  127, 126, 120, 64,  0,   96,  112, 120, 126, 127, 103,
    99,  97,  96,  96,  96,  0,   0,   0,   63,  127, 127, 240, 224, 224, 96,
    96,  56,  127, 127, 127, 0,   0,   0,   0,   127, 127, 127, 0,   0,   0,
    0,   0,   0,   15,  63,  127, 123, 243, 227, 227, 227, 227, 99,  99,  35,
    3,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   1,   1,   1,   1,   1,   1,   1,   0,   8,   8,
    8,   8,   8,   12,  12,  12,  12,  12,  14,  14,  14,  14,  14,  15,  15,
    15,  15,  15,  15,  15,  15,  15,  15,  12,  0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0};

static const char SSL_CA_PEM[] =
    "-----BEGIN CERTIFICATE-----\n"
    "MIIDdzCCAl+gAwIBAgIEAgAAuTANBgkqhkiG9w0BAQUFADBaMQswCQYDVQQGEwJJ\n"
    "RTESMBAGA1UEChMJQmFsdGltb3JlMRMwEQYDVQQLEwpDeWJlclRydXN0MSIwIAYD\n"
    "VQQDExlCYWx0aW1vcmUgQ3liZXJUcnVzdCBSb290MB4XDTAwMDUxMjE4NDYwMFoX\n"
    "DTI1MDUxMjIzNTkwMFowWjELMAkGA1UEBhMCSUUxEjAQBgNVBAoTCUJhbHRpbW9y\n"
    "ZTETMBEGA1UECxMKQ3liZXJUcnVzdDEiMCAGA1UEAxMZQmFsdGltb3JlIEN5YmVy\n"
    "VHJ1c3QgUm9vdDCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAKMEuyKr\n"
    "mD1X6CZymrV51Cni4eiVgLGw41uOKymaZN+hXe2wCQVt2yguzmKiYv60iNoS6zjr\n"
    "IZ3AQSsBUnuId9Mcj8e6uYi1agnnc+gRQKfRzMpijS3ljwumUNKoUMMo6vWrJYeK\n"
    "mpYcqWe4PwzV9/lSEy/CG9VwcPCPwBLKBsua4dnKM3p31vjsufFoREJIE9LAwqSu\n"
    "XmD+tqYF/LTdB1kC1FkYmGP1pWPgkAx9XbIGevOF6uvUA65ehD5f/xXtabz5OTZy\n"
    "dc93Uk3zyZAsuT3lySNTPx8kmCFcB5kpvcY67Oduhjprl3RjM71oGDHweI12v/ye\n"
    "jl0qhqdNkNwnGjkCAwEAAaNFMEMwHQYDVR0OBBYEFOWdWTCCR1jMrPoIVDaGezq1\n"
    "BE3wMBIGA1UdEwEB/wQIMAYBAf8CAQMwDgYDVR0PAQH/BAQDAgEGMA0GCSqGSIb3\n"
    "DQEBBQUAA4IBAQCFDF2O5G9RaEIFoN27TyclhAO992T9Ldcw46QQF+vaKSm2eT92\n"
    "9hkTI7gQCvlYpNRhcL0EYWoSihfVCr3FvDB81ukMJY2GQE/szKN+OMY3EU/t3Wgx\n"
    "jkzSswF07r51XgdIGn9w/xZchMB5hbgF/X++ZRGjD8ACtPhSNzkE1akxehi/oCr0\n"
    "Epn3o0WC4zxe9Z2etciefC7IpJ5OCBRLbf1wbWsaY71k5h+3zvDyny67G7fyUIhz\n"
    "ksLi4xaNmjICq44Y3ekQEe5+NauQrz4wlHrQMz2nZQ/1/I6eYs9HRCwBXbsdtTLS\n"
    "R9I4LtD+gdwyah617jzV/OeBHRnDJELqYzmp\n"
    "-----END CERTIFICATE-----\n";

static const char *FOREX_URL =
    "https://free.currconv.com/api/v7/"
    "convert?q=USD_CAD&compact=ultra&apiKey=" API_KEY;
static const char *RATE_KEY = "USD_CAD";

static bool has_wifi = false;
static bool http_error = false;

static unsigned long previous_ms = 0;
static int led_countdown = 0;
static int update_rate_countdown = 0;

static float rate = 0.0;

static bool is_led_on = false;
static bool is_refreshing = false;

static int last_button_b_state = HIGH;
static RATE_CHANGE rate_change = SAME;

RGB_LED rgbLed;

static void init_wifi() {
  Screen.print(0, "Connecting...");

  if (WiFi.begin() == WL_CONNECTED) {
    IPAddress ip = WiFi.localIP();
    has_wifi = true;
  } else {
    has_wifi = false;
    Screen.print(0, "No Wi-Fi\r\n ");
  }
}

static void update_rate() {
  HTTPClient *httpClient = new HTTPClient(SSL_CA_PEM, HTTP_GET, FOREX_URL);
  const Http_Response *result = httpClient->send();

  if (result == NULL) {
    http_error = true;
    Serial.print("Error Code: ");
    Serial.println(httpClient->get_error());
    update_rate_countdown = FAIL_REFRESH_INTERVAL;
  } else {
    const char *response = result->body;
    StaticJsonDocument<32> doc;
    DeserializationError err = deserializeJson(doc, response);

    if (err) {
      http_error = true;
      Serial.print(F("deserializeJson() failed with code "));
      Serial.println(err.c_str());
    } else {
        http_error = false;
      float new_rate = doc[RATE_KEY];

      if (new_rate < rate) {
        rate_change = DECREASE;
        is_led_on = true;
      } else if (new_rate == rate) {
        rate_change = SAME;
        is_led_on = false;
      } else {
        rate_change = INCREASE;
        is_led_on = true;
      }

      rate = new_rate;
    }

    Serial.print("Rate: ");
    Serial.println(rate, 4);

    led_countdown = LED_DURATION_SEC;
    update_rate_countdown = REFRESH_INTERVAL;
  }

  delete httpClient;
}

static void show_rate() {
  char rate_buffer[24];
  sprintf(rate_buffer, "USD = %.4f CAD", rate);
  Screen.print(1, rate_buffer);
}

static void show_countdown() {
  char tick_buffer[8];
  sprintf(tick_buffer, "%d", update_rate_countdown);
  Screen.print(3, tick_buffer);
}

void setup() {
  Screen.init();
  Screen.draw(0, 0, 128, 8, AZURE_BMP);
  delay(SPLASH_DURATION_MS);
  Screen.clean();

  has_wifi = false;
  init_wifi();

  pinMode(USER_BUTTON_A, INPUT);
  pinMode(USER_BUTTON_B, INPUT);

  last_button_b_state = digitalRead(USER_BUTTON_B);

  if (has_wifi) {
    Screen.clean();
    Screen.print(0, "USD/CAD Rate");
    update_rate();
    show_rate();
  }
}

void loop() {
  if (!has_wifi) {
    Serial.println("No Wi-Fi");
    return;
  }

  unsigned long current_ms = millis();

  if (last_button_b_state == HIGH && digitalRead(USER_BUTTON_B) == LOW) {
    last_button_b_state = LOW;

    Serial.println("Force refresh...");
    Screen.print(3, "Refreshing...");
    rgbLed.turnOff();

    update_rate();
  } else if (last_button_b_state == LOW && digitalRead(USER_BUTTON_B) == HIGH) {
    last_button_b_state = HIGH;
  }

  if (is_led_on) {
    switch (rate_change) {
      case DECREASE:
        rgbLed.setColor(255, 0, 0);
        break;
      case INCREASE:
        rgbLed.setColor(0, 255, 0);
        break;
      default:
        break;
    }
  } else {
    rgbLed.turnOff();
  }

  if (current_ms - previous_ms >= UPDATE_INTERVAL_MS) {
    previous_ms = current_ms;

    update_rate_countdown--;
    led_countdown--;

    if (update_rate_countdown < 0) {
      Screen.print(3, "Refreshing...");
      rgbLed.turnOff();
      update_rate();

      if (http_error) {
        Screen.print(1, "Error");
      } else {
        show_rate();
      }
    }

    if (led_countdown < 0) {
      is_led_on = false;
    }

    show_countdown();
  }
}
