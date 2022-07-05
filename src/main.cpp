// Copyright (c) 2022 Technologiestiftung Berlin & Fabian Morón Zirfas
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <Arduino.h>
#include "stadtpuls.h"

#define PRO_BUTTON_PIN 0

Stadtpuls stadtpuls;               // Create the instance of the Stadtpuls lib
Stadtpuls_Options options;         // All options are optional. See struct `Stadtpuls_Options` in stadtpuls.h
unsigned long previous_millis = 0; // keeping track of time
unsigned long interval = 60000;    // 60 seconds
int button_state;                  // the current reading from the input pin
int last_button_state = LOW;       // the previous reading from the input pin
// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long last_debounce_time = 0; // the last time the output pin was toggled
unsigned long debounce_delay = 50;    // the debounce time; increase if the
void setup()
{

  options.debug = true;              // Make stadtpuls output some infos to the serial
  options.sensor_name = "Stadtpuls"; // Will be used as the sensor name and for the access point
  options.check_certificate = false; // If you want to check the certificate of the server, set this to true. The cert of api.stadtpuls.com is hardcoded at the moment.
  // options.server = "stadtpuls-api-v3-staging.onrender.com"; // The server to send data to. We use a simple Nodejs echo server on localhost and ngrok.com to make it easier to debug. Remove this and it will use `api.stadtpuls.com`
  options.use_display = false; // Tell the board to use its display (Available on Heltec LoRa v2 WiFi)
  // options.ssid = "Stadtpuls";                     // If you already know your WiFi you can pass it here
  // options.password = "Stadtpuls";                 // If you know your WiFi's passphrase you can pass it here
  // options.sensor_id = "0";                        // The id of your sensor on stadtpuls.com
  // options.auth_token = "Stadtpuls";               // Your token from stadtpuls.com
  // options.forget_pin = STADTPULS_FORGET_PIN;      // If you want to wire a different pin as forget pin. (currently 33)
  // options.use_display = false; // if you want to use the display (and have one)
  options.show_splash = true; // show the stadtpuls splashscreen
  // options.screen_width = STADTPULS_SCREEN_WIDTH;
  // options.screen_height = STADTPULS_SCREEN_HEIGHT;
  // options.screen_reset = STADTPULS_SCREEN_RESET; // screen reset pin
  // options.screen_address = STADTPULS_SCREEN_ADDRESS; // I2C address
  // options.i2c_screen_sda_pin = STADTPULS_I2C_SCREEN_SDA_PIN;
  // options.i2c_screen_scl_pin = STADTPULS_I2C_SCREEN_SCL_PIN;

  Serial.begin(115200);
  // stadtpuls.begin(); // Setup the stadtpuls instance with only defaults.
  stadtpuls.begin(options); // Setup the stadtpuls instance with your options.
  pinMode(PRO_BUTTON_PIN, INPUT);
}

/**
 * This is a function for imidiate testing
 * Press the pro button on the heltec board and send to stadtpuls.com
 * Based on https://www.arduino.cc/en/Tutorial/BuiltInExamples/Debounce
 *
 */
void send_on_press()
{
  int button_reading = digitalRead(PRO_BUTTON_PIN);
  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (button_reading != last_button_state)
  {
    // reset the debouncing timer
    last_debounce_time = millis();
  }

  if ((millis() - last_debounce_time) > debounce_delay)
  {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (button_reading != button_state)
    {
      button_state = button_reading;

      // only send if the button state is LOW
      if (button_state == LOW)
      {
        stadtpuls.send(std::vector<double>{
            (double)random(-1000, 1000) / 100,
            (double)random(-1000, 1000) / 100,
            (double)random(-1000, 1000) / 100,
        });
      }
    }
  }
  last_button_state = button_reading;
}

void loop()
{
  stadtpuls.listen(); // Listen to changes on the forget pin (currently 33)

  // Stadtpuls.com's api is rate limitied. Still we urge you not to make to many
  // requests. So we use a interval of 60 seconds to send data to stadtpuls.com.
  unsigned long current_millis = millis(); // Keeping track of time.
  if (current_millis - previous_millis >= interval)
  {
    // stadtpuls.send takes a vector of doubles. You can send as many values as you like. Currently the frontend only displays the first one, but you can optain them all through the API.
    stadtpuls.send(std::vector<double>{
        (double)random(-1000, 1000) / 100,
        (double)random(-1000, 1000) / 100,
        (double)random(-1000, 1000) / 100,
    });

    previous_millis = current_millis; // Set the previous_millis to the current_millis so we can keep track of the interval.
  }
  send_on_press();
}
