#include "AZ3166WiFi.h"
#include "arduinojson.h"
#include "http_client.h"

#include "api.h"
#include "config.h"
#include "splash.h"

static const String endpoint =
    (String(FUNCTION_URL) + "?code=" + FUNCTION_KEY +
     "&currencypair=" + CURRENCY_PAIR + "&stock=" + STOCK_TICKER +
     "&location=" + WEATHER_LOCATION);

static const char *endpoint_url = endpoint.c_str();

static long connect_wifi_backoff_ms = WIFI_RETRY_STARTING_INTERVAL_MS;
static bool has_wifi = false;
static bool http_error = false;

static unsigned long previous_ms = 0;
static int update_rate_countdown = 0;
static int update_view_countdown = 0;

static float rate = 0.0;
static float stock_price = 0.0;
static const char *condition = "";
static float temp = 0.0;

static int current_view = 0;
static bool is_refreshing = false;

static int last_button_a_state = HIGH;
static int last_button_b_state = HIGH;

// Establish wifi connection.
static void init_wifi() {
  Screen.clean();
  Screen.print(0, "Connecting...");

  if (WiFi.begin() == WL_CONNECTED) {
    has_wifi = true;
    Screen.clean();
  } else {
    has_wifi = false;
    Screen.print(0, "No Wi-Fi");
  }
}

// Retrieve data from Azure Function.
static void refresh_data() {
  is_refreshing = true;
  HTTPClient *httpClient = new HTTPClient(SSL_CA_PEM, HTTP_GET, endpoint_url);
  const Http_Response *result = httpClient->send();
  Serial.println(endpoint_url);

  if (result == NULL) {
    // Request failed, retry later
    http_error = true;
    Serial.print("Error Code: ");
    Serial.println(httpClient->get_error());
    update_rate_countdown = FAIL_REFRESH_INTERVAL;
  } else {
    // Request succeeded
    const char *response = result->body;
    StaticJsonDocument<256> doc;
    DeserializationError err = deserializeJson(doc, response);

    if (err) {
      // Deserialization failed
      http_error = true;
      Serial.print(F("deserializeJson() failed with code"));
      Serial.println(err.c_str());
    } else {
      // Success
      http_error = false;

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

    // Reset countdown values
    update_rate_countdown = SUCCESS_REFRESH_INTERVAL;
    update_view_countdown = CHANGE_VIEW_INTERVAL;
  }

  is_refreshing = false;
  delete httpClient;
}

// Show exchange rate
static void show_rate() {
  char header_buffer[MAX_LINE_BUFFER_SIZE];
  char subheader_buffer[MAX_LINE_BUFFER_SIZE];

  sprintf(header_buffer, "%s Rate", CURRENCY_PAIR);
  sprintf(subheader_buffer, "%s = %.4f %s", BASE_CURRENCY, rate,
          QUOTE_CURRENCY);

  Screen.print(0, header_buffer);
  Screen.print(1, subheader_buffer);
}

// Show stock price
static void show_stock() {
  char header_buffer[MAX_LINE_BUFFER_SIZE];
  char subheader_buffer[12];

  sprintf(header_buffer, "%s Price", STOCK_TICKER);
  sprintf(subheader_buffer, "$%.2f", stock_price);

  Screen.print(0, header_buffer);
  Screen.print(1, subheader_buffer);
}

// Show weather info
static void show_weather() {
  char subheader_buffer[8];
  sprintf(subheader_buffer, "%.1f C", temp);

  Screen.print(0, WEATHER_LOCATION);
  Screen.print(1, subheader_buffer);
}

// Show current view
static void show_view() {
  if (current_view == 0) {
    show_rate();
  } else if (current_view == 1) {
    show_stock();
  } else if (current_view == 2) {
    show_weather();
  }

  if (http_error) {
    Screen.print(1, "Error");
  }
}

// Refresh data and show current view
static void refresh_and_show_view() {
  refresh_data();
  show_view();
}

// Show countdown timer in lower left corner
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

  pinMode(USER_BUTTON_A, INPUT);
  pinMode(USER_BUTTON_B, INPUT);

  last_button_a_state = digitalRead(USER_BUTTON_A);
  last_button_b_state = digitalRead(USER_BUTTON_B);

  init_wifi();

  while (!has_wifi) {
    // No wifi connection, retry later
    Serial.print("Retrying in ");
    Serial.print(connect_wifi_backoff_ms);
    Serial.println(" milliseconds...");
    delay(connect_wifi_backoff_ms);
    init_wifi();
    connect_wifi_backoff_ms =
        (long)(connect_wifi_backoff_ms * WIFI_RETRY_MULT_FACTOR);
  }

  // Load data for first time
  connect_wifi_backoff_ms = WIFI_RETRY_STARTING_INTERVAL_MS;
  Screen.print(0, "Connected.");
  Screen.print(3, "Loading...");
  refresh_data();

  if (http_error) {
    Screen.print(1, "Error");
  } else {
    show_view();
  }
}

void loop() {
  // Check Internet connection
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Lost wifi connection");
    Screen.clean();
    Screen.print(0, "Rebooting...");
    delay(2000);
    SystemReboot();
    return;
  }

  unsigned long current_ms = millis();

  // Handle button A press (switching views)
  if (last_button_a_state == HIGH && digitalRead(USER_BUTTON_A) == LOW) {
    last_button_a_state = LOW;
    update_view_countdown = CHANGE_VIEW_INTERVAL;
    current_view = (current_view + 1) % 3;
    show_view();
  } else if (last_button_a_state == LOW && digitalRead(USER_BUTTON_A) == HIGH) {
    last_button_a_state = HIGH;
  }

  // Handle button B press (refreshing data)
  if (last_button_b_state == HIGH && digitalRead(USER_BUTTON_B) == LOW) {
    last_button_b_state = LOW;

    if (!is_refreshing) {
      Serial.println("Forced refresh");
      Screen.print(3, "Refreshing...");
      refresh_and_show_view();
    }
  } else if (last_button_b_state == LOW && digitalRead(USER_BUTTON_B) == HIGH) {
    last_button_b_state = HIGH;
  }

  // Handle events per tick (second)
  if (current_ms - previous_ms >= UPDATE_INTERVAL_MS) {
    previous_ms = current_ms;

    update_rate_countdown--;
    update_view_countdown--;

    // Time for data refresh
    if (update_rate_countdown < 0) {
      if (!is_refreshing) {
        Screen.print(3, "Refreshing...");
        refresh_and_show_view();
      }
    }

    // Time for view change
    if (update_view_countdown < 0) {
      update_view_countdown = CHANGE_VIEW_INTERVAL;
      current_view = (current_view + 1) % 3;
      show_view();
    }

    // Update countdown timer
    show_countdown();
  }
}
