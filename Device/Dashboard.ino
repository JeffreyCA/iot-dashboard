#include "Arduino.h";

#include "AZ3166WiFi.h";
#include "RGB_LED.h";
#include "arduinojson.h";
#include "http_client.h";

#include "api.h";
#include "config.h";
#include "splash.h";

static const String endpoint = (String(FUNCTION_URL) + "?code=" + FUNCTION_KEY +
                   "&currencypair=" + CURRENCY_PAIR + "&stock=" + STOCK_TICKER +
                   "&location=" + WEATHER_LOCATION);

static const char *endpoint_url = endpoint.c_str();

static bool has_wifi = false;
static bool http_error = false;

static unsigned long previous_ms = 0;
static int led_countdown = 0;
static int update_rate_countdown = 0;

static float rate = 0.0;
static float stock_price = 0.0;
static const char *condition = "";
static float temp = 0.0;

static bool is_led_on = false;
static bool is_refreshing = false;

static int last_button_a_state = HIGH;
static int last_button_b_state = HIGH;

static int current_view = 0;

RGB_LED rgbLed;

// Establish wifi connection.
static void init_wifi() {
  Screen.clean();
  Screen.print(0, "Connecting to Wifi...", true);

  if (WiFi.begin() == WL_CONNECTED) {
    has_wifi = true;
  } else {
    has_wifi = false;
    Screen.print(0, "No Wi-Fi\r\n ");
  }
}

static void update_info() {
  const char *endpoint_url = endpoint.c_str();
  HTTPClient *httpClient = new HTTPClient(SSL_CA_PEM, HTTP_GET, endpoint_url);
  const Http_Response *result = httpClient->send();

  Serial.println(endpoint_url);

  if (result == NULL) {
    http_error = true;
    Serial.print("Error Code: ");
    Serial.println(httpClient->get_error());
    update_rate_countdown = FAIL_REFRESH_INTERVAL;
  } else {
    const char *response = result->body;
    StaticJsonDocument<256> doc;
    DeserializationError err = deserializeJson(doc, response);

    if (err) {
      http_error = true;
      Serial.print(F("deserializeJson() failed with code"));
      Serial.println(err.c_str());
    } else {
      http_error = false;
      is_led_on = true;

      float new_rate = doc["exchange_rate"];
      float new_stock_price = doc["stock_price"];
      const char *new_condition = doc["condition"];
      float new_temp = doc["temp"];

      rate = new_rate;
      stock_price = new_stock_price;
      condition = new_condition;
      temp = new_temp;
    }

    Serial.print("Rate: ");
    Serial.println(rate, 4);

    Serial.print("Stock price: ");
    Serial.println(stock_price, 2);

    Serial.print("Condition: ");
    Serial.println(condition);

    Serial.print("Temp: ");
    Serial.println(temp, 1);
    Serial.println();

    led_countdown = LED_FLASH_INTERVAL;
    update_rate_countdown = REFRESH_INTERVAL;
  }

  is_refreshing = false;
  delete httpClient;
}

static void show_rate() {
  char header_buffer[MAX_LINE_BUFFER_SIZE];
  char subheader_buffer[MAX_LINE_BUFFER_SIZE];

  sprintf(header_buffer, "%s Rate", CURRENCY_PAIR);
  sprintf(subheader_buffer, "%s = %.4f %s", CURRENCY_1, rate, CURRENCY_2);

  Screen.print(0, header_buffer);
  Screen.print(1, subheader_buffer);
}

static void show_stock() {
  char header_buffer[MAX_LINE_BUFFER_SIZE];
  char subheader_buffer[12];

  sprintf(header_buffer, "%s Price", STOCK_TICKER);
  sprintf(subheader_buffer, "$%.2f", stock_price);

  Screen.print(0, header_buffer);
  Screen.print(1, subheader_buffer);
}

static void show_weather() {
  char subheader_buffer[8];
  sprintf(subheader_buffer, "%.1f C", temp);

  Screen.print(0, WEATHER_LOCATION);
  Screen.print(1, subheader_buffer);
}

static void show_view() {
  if (current_view == 0) {
    show_rate();
  } else if (current_view == 1) {
    show_stock();
  } else if (current_view == 2) {
    show_weather();
  }
}

static void update_and_show_view() {
  update_info();

  if (http_error) {
    Screen.print(1, "Error");
  } else {
    show_view();
  }
}

static void show_countdown() {
  char tick_buffer[8];
  sprintf(tick_buffer, "%d", update_rate_countdown);
  Screen.print(3, tick_buffer);
}

void setup() {
  Screen.init();
  Screen.draw(0, 0, 128, 8, SPLASH_BMP);
  delay(SPLASH_DURATION_MS);
  Screen.clean();

  has_wifi = false;
  init_wifi();

  pinMode(USER_BUTTON_A, INPUT);
  pinMode(USER_BUTTON_B, INPUT);

  last_button_b_state = digitalRead(USER_BUTTON_B);

  if (has_wifi) {
    Screen.clean();
    update_info();

    if (http_error) {
      Screen.print(1, "Error");
    } else {
      show_view();
    }
  }
}

void loop() {
  if (!has_wifi) {
    has_wifi = false;
    init_wifi();
    return;
  }

  unsigned long current_ms = millis();

  if (last_button_a_state == HIGH && digitalRead(USER_BUTTON_A) == LOW) {
    last_button_a_state = LOW;

    current_view = (current_view + 1) % 3;

    Serial.print("Switching to view: ");
    Serial.println(current_view);

    rgbLed.turnOff();
    show_view();
  } else if (last_button_a_state == LOW && digitalRead(USER_BUTTON_A) == HIGH) {
    last_button_a_state = HIGH;
  }

  if (last_button_b_state == HIGH && digitalRead(USER_BUTTON_B) == LOW) {
    last_button_b_state = LOW;

    if (!is_refreshing) {
      is_refreshing = true;
      Serial.println("Force refresh...");
      Screen.print(3, "Refreshing...");
      rgbLed.turnOff();
      update_and_show_view();
    }
  } else if (last_button_b_state == LOW && digitalRead(USER_BUTTON_B) == HIGH) {
    last_button_b_state = HIGH;
  }

  if (is_led_on) {
    rgbLed.setColor(0, 0, 255);
  } else {
    rgbLed.turnOff();
  }

  if (current_ms - previous_ms >= UPDATE_INTERVAL_MS) {
    previous_ms = current_ms;

    update_rate_countdown--;
    led_countdown--;

    if (update_rate_countdown < 0) {
      if (!is_refreshing) {
        is_refreshing = true;
        Screen.print(3, "Refreshing...");
        rgbLed.turnOff();
        update_and_show_view();
      }
    }

    if (led_countdown < 0) {
      is_led_on = false;
    }

    show_countdown();
  }
}
