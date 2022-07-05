// Copyright (c) 2022 Technologiestiftung Berlin & Fabian Morón Zirfas
//
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include "stadtpuls.h"
const char *PARAM_SSID = "ssid";
const char *PARAM_PASSWORD = "password";
const char *PARAM_SENSOR_ID = "sensorid";
const char *PARAM_AUTH_TOKEN = "authtoken";
AsyncWebServer ap_server(80);
TwoWire I2C_OLED = TwoWire(0);
Adafruit_SSD1306 display;

WiFiClientSecure client;
String sensor_name = "Stadtpuls";
String processor_sensor_name = "Stadtpuls";
String processor_ssid = "";
String processor_password = "";
String processor_sensor_id = "";
String processor_auth_token = "";

Stadtpuls::Stadtpuls()
{

  // Constructor
}

void Stadtpuls::begin()
{
  Stadtpuls_Options options;
  begin(options);
}
void Stadtpuls::begin(Stadtpuls_Options options)
{
// See board identity here https://github.com/MattFryer/Board_Identify/blob/3fd711f15cb752a9e18ab837c9b5773eb4d871e0/src/Board_Identify.h
#if defined(WIFI_LoRa_32_V2) || defined(WIFI_Kit_32)

  Serial.println("WiFi LoRa 32 V2 or WiFi Kit 32");
  // || defined( WIFI_LoRa_32 )
#elif defined(ARDUINO_FEATHER_ESP32)
  Serial.println("ARDUINO_FEATHER_ESP32");
#else
  Serial.println("Not a LoRa 32 V2 board. Use the library at your own risk.");
  //  defined( WIFI_Kit_32 ) || defined( WIFI_LoRa_32 )
#warning "This library is not tested with your board proceed at own risk"
#endif

  if (options.debug)
  {
    Serial.println("-----------------------");
    Serial.println("current configruation");
    Serial.println("Options do not reflect data stored in flash memory");
    Serial.println("ssid, password, sensor_id and auth_token will be loaded from preferences");
    Serial.print("options.sensor_name: ");
    Serial.println(options.sensor_name);
    Serial.print("options.ssid: ");
    Serial.println(options.ssid);
    Serial.print("options.password: ");
    Serial.println(options.password);
    Serial.print("options.sensor_id: ");
    Serial.println(options.sensor_id);
    Serial.print("options.auth_token: ");
    Serial.println(options.auth_token);
    Serial.print("options.server: ");
    Serial.println(options.server);
    Serial.print("options.check_certificate: ");
    Serial.println(options.check_certificate);
    Serial.print("options.forget_pin: ");
    Serial.println(options.forget_pin);
    Serial.print("options.use_display: ");
    Serial.println(options.use_display);
    Serial.print("options.show_splash: ");
    Serial.println(options.show_splash);
    Serial.print("options.screen_width: ");
    Serial.println(options.screen_width);
    Serial.print("options.screen_height: ");
    Serial.println(options.screen_height);
    Serial.print("options.screen_reset: ");
    Serial.println(options.screen_reset);
    Serial.print("options.i2c_screen_sda_pin: ");
    Serial.println(options.i2c_screen_sda_pin);
    Serial.print("options.i2c_screen_scl_pin: ");
    Serial.println(options.i2c_screen_scl_pin);
    Serial.print("options.debug: ");
    Serial.println(options.debug);
    Serial.println("-----------------------");
  }
  // options parsing
  if (options.sensor_name.length() > 0)
  {
    sensor_name = options.sensor_name;
  }
  if (options.ssid.length() > 0)
  {
    ssid = options.ssid;
  }
  if (options.password.length() > 0)
  {
    password = options.password;
  }
  if (options.sensor_id.length() > 0)
  {
    sensor_id = options.sensor_id;
  }
  if (options.auth_token.length() > 0)
  {
    auth_token = options.auth_token;
  }
  if (options.forget_pin != STADTPULS_FORGET_PIN)
  {
    forget_pin = options.forget_pin;
  }
  if (options.server.length() > 0)
  {
    server = options.server;
  }
  if (options.check_certificate)
  {
    check_cert = true;
  }
  if (options.debug == true)
  {
    Serial.println("Debug mode enabled");
    PRINT = true;
  }
  else
  {
    PRINT = false;
  }
  if (options.use_display == true)
  {
    use_display = true;
  }

  // end options parsing
  pinMode(forget_pin, INPUT_PULLDOWN);
  digitalWrite(forget_pin, LOW);

  if (use_display == true)
  {

    // display setup
    I2C_OLED.begin(
        options.i2c_screen_sda_pin,
        options.i2c_screen_scl_pin);
    display = Adafruit_SSD1306(
        options.screen_width,
        options.screen_height,
        &I2C_OLED,
        options.screen_reset);
    bool display_status = display.begin(SSD1306_SWITCHCAPVCC, options.screen_address);
    if (!display_status)
    {
      if (PRINT)
      {
        Serial.println("Could not find OLED display");
      }
    }
    else
    {
      screen_active = true;
      grfx.begin(&display, &screen_active);
      if (options.show_splash == true)
      {
        grfx.splash(1000);
      }
    }
  }
  // wifi setup
  if (PRINT)
  {
    WiFi.onEvent(stadtpuls_WiFiEvent);
  }

  Stadtpuls::preferences.begin(Stadtpuls::prefs_credentials_key, false);
  if (ssid == "")
  {
    ssid = Stadtpuls::preferences.getString(prefs_ssid_key, "");
  }
  else
  { //
    Stadtpuls::preferences.putString(prefs_ssid_key, ssid);
  }
  if (password == "")
  {
    password = Stadtpuls::preferences.getString(prefs_password_key, "");
  }
  else
  {
    Stadtpuls::preferences.putString(prefs_password_key, password);
  }
  if (sensor_id == "")
  {
    sensor_id = Stadtpuls::preferences.getString(prefs_sensor_id_key, "");
  }
  else
  {
    Stadtpuls::preferences.putString(prefs_sensor_id_key, sensor_id);
  }
  if (auth_token == "")
  {
    auth_token = Stadtpuls::preferences.getString(prefs_auth_token_key, "");
  }
  else
  {
    Stadtpuls::preferences.putString(prefs_auth_token_key, auth_token);
  }

  // if (ssid == "" || password == "" || sensor_id == "" || auth_token == "")
  // {
  //   password = Stadtpuls::preferences.getString(prefs_password_key, "");
  //   sensor_id = Stadtpuls::preferences.getString(prefs_sensor_id_key, "");
  //   auth_token = Stadtpuls::preferences.getString(prefs_auth_token_key, "");
  // }
  // else
  // {
  //   Stadtpuls::preferences.putString(prefs_ssid_key, ssid);
  //   Stadtpuls::preferences.putString(prefs_password_key, password);
  //   Stadtpuls::preferences.putString(prefs_sensor_id_key, sensor_id);
  //   Stadtpuls::preferences.putString(prefs_auth_token_key, auth_token);
  // }
  Stadtpuls::preferences.end();
  int no_wifi_count = 0;
  WiFi.mode(WIFI_STA);
  if (PRINT)
    Serial.println("Connecting to WiFi");
  if (password == "")
  {
    WiFi.begin(ssid.c_str());
  }
  else
  {
    WiFi.begin(ssid.c_str(), password.c_str());
  }
  while (WiFi.status() != WL_CONNECTED)
  {
    if (PRINT)
      Serial.print('.');
    delay(1000);
    no_wifi_count++;
    if (no_wifi_count == STADTPULS_WIFI_SETUP_RETRIES)
    {
      if (WiFi.status() != WL_CONNECTED)
      {
        wifi_active = false;
        setup_access_point = true;
        if (PRINT)
          Serial.println("WiFi connection failed");
        break;
      }
    }
  }
  if (PRINT)
    Serial.println("");

  if (WiFi.status() == WL_CONNECTED)
  {
    wifi_active = true;
    if (PRINT)
    {
      Serial.println("Connected!");
      Serial.println(WiFi.localIP());
    }
  }
  if (wifi_active == false)
  {
    // Initialize SPIFFS
    if (!SPIFFS.begin(true))
    {
      if (PRINT)
        Serial.println("An Error has occurred while mounting SPIFFS");
      return;
    }
    WiFi.softAP(sensor_name.c_str());
    WiFi.mode(WIFI_AP);
    IPAddress ip = WiFi.softAPIP();
    ap_ip = ip.toString();

    if (PRINT)
    {
      Serial.println("Access Point WiFi: " + sensor_name);
      Serial.println("Access Point IP: http://" + ap_ip);
    }

    ap_server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request)
                 { request->send(SPIFFS, "/style.css", "text/css"); });

    ap_server.on("/", HTTP_GET, [&](AsyncWebServerRequest *request)
                 {
                  if(PRINT){
                    Serial.println("Client connected on path /");
                  }
                   request->send(SPIFFS, "/index.html", String(), false, processor); });
    // Send a POST request to <IP>/post
    // with a form field ssid and password set to <ssid> and <password>
    ap_server.on("/post", HTTP_POST, [&](AsyncWebServerRequest *request)
                 {
      String user_ssid;
      String user_password;
      String user_auth_token;
      String user_sensor_id;
      if (request->hasParam(PARAM_SSID, true))
      {
        user_ssid = request->getParam(PARAM_SSID, true)->value();
      }
      else
      {
        user_ssid = "No ssid sent";
      }
      if (request->hasParam(PARAM_PASSWORD, true))
      {
        user_password = request->getParam(PARAM_PASSWORD, true)->value();
      }
      else
      {
        user_password = "";
      }
      if (request->hasParam(PARAM_SENSOR_ID, true))
      {
        user_sensor_id= request->getParam(PARAM_SENSOR_ID, true)->value();
      }
      else
      {
        user_sensor_id = "";
      }
            if (request->hasParam(PARAM_AUTH_TOKEN, true))
      {
        user_auth_token= request->getParam(PARAM_AUTH_TOKEN, true)->value();
      }
      else
      {
        user_auth_token= "";
      }
      Stadtpuls::preferences.begin(prefs_credentials_key, false);
      Stadtpuls::preferences.putString(prefs_ssid_key, user_ssid);
      processor_ssid = user_ssid;
      Stadtpuls::preferences.putString(prefs_password_key, user_password);
      processor_password = user_password;
      Stadtpuls::preferences.putString(prefs_auth_token_key, user_auth_token);
      processor_auth_token = user_auth_token;
      Stadtpuls::preferences.putString(prefs_sensor_id_key, user_sensor_id);
      processor_sensor_id = user_sensor_id;
      Stadtpuls::preferences.end();
      if(PRINT){
      Serial.println("ssid: " + user_ssid);
      Serial.println("password: " + user_password);
      Serial.println("sensor_id: " + user_sensor_id);
      Serial.println("auth_token: " + user_auth_token);
      Serial.println("Please Restart the device");
      }
      // oled.clearDisplay(0);
      // oled.drawString("Saving credentials", 200);
      request->send(SPIFFS, "/post.html", String(), false, processor); });
    // set password in prefs
    // request->send(200, "text/plain", "SSID: " + user_ssid + " PASSWORD: " + user_password); });

    ap_server.onNotFound(not_found);

    ap_server.begin();
    if (PRINT)
    {
      Serial.println("Access Point Started");
    }
  }
}
void Stadtpuls::listen()
{ // listen for pin events
  if (digitalRead(forget_pin) == HIGH)
  {
    Stadtpuls::preferences.begin(prefs_credentials_key, false);
    Stadtpuls::preferences.clear();
    Stadtpuls::preferences.end();
    Serial.print("Credentials cleared, I will reboot in ");
    Serial.print(STADTOPULS_FORGET_DELAY_MS / 1000);
    Serial.println(" seconds");
    // wait for STADTOPULS_FORGET_DELAY_MS ms
    delay(STADTOPULS_FORGET_DELAY_MS);
    ESP.restart();
  }
}

void Stadtpuls::send(std::vector<double> measurements)
{
  if (wifi_active == true && WiFi.status() == WL_CONNECTED)
  {
    if (measurements.begin() == measurements.end())
    {
      return;
    }
    String m = "";
    for (std::size_t i = 0; i < measurements.size(); ++i)
    {
      if (i != measurements.size() - 1)
      {
        m += clear_pad(measurements[i]) + ", ";
      }
      else
      {
        m += clear_pad(measurements[i]);
      }
    }

    String payload = "{\"measurements\": [" + m + "], \"sensor_name\": \"" + sensor_name + "\"}";
    if (PRINT)
    {
      Serial.print("payload: ");
      Serial.println(payload);
    }
    if (check_cert == true)
    {

      client.setCACert(root_ca);
    }
    else
    {

      client.setInsecure();
    }
    if (!client.connect(server.c_str(), 443))
    {
      if (PRINT)
      {
        Serial.println("Connection failed!");
      }
    }
    else
    {
      if (PRINT)
      {

        Serial.println("Connected to server!");
        Serial.println("\n- - - - - - - - - - - - - - - -");
      }
      // Make a HTTP request:
      client.println("POST " + String("/api/v3/sensors/" + String(sensor_id) + "/records") + " HTTP/1.0");
      client.println("Host: " + String(server));
      client.println("Content-Type: application/json");
      client.println("Content-Length: " + String(payload.length()));
      client.println("Authorization: Bearer " + String(auth_token));
      client.println("Connection: close");
      client.println();
      client.println(payload);

      while (client.connected())
      {
        String line = client.readStringUntil('\n');
        if (line == "\r")
        {
          Serial.println("POST Success!");
          break;
        }
      }
      // if there are incoming bytes available
      // from the server, read them and print them:
      while (client.available())
      {
        char c = client.read();
        if (PRINT)
        {
          Serial.write(c);
        }
      }

      if (PRINT)
      {
        Serial.println("\n- - - - - - - - - - - - - - - -");
      }
      client.stop();
    }
  }
  else
  {
    if (PRINT)
    {
      Serial.println("WiFi not connected");
    }
  }
}

String Stadtpuls::processor(const String &var)
{

  String new_ssid = processor_ssid;
  String new_password = processor_password;
  // preferences.getString(prefs_password_key, "");
  String name = processor_sensor_name;
  String auth_token = processor_auth_token;
  String sensor_id = processor_sensor_id;
  // preferences.getString("name", "");
  Serial.println(var);
  if (var == "SSID")
  {
    return new_ssid;
  }
  if (var == "PASSWORD")
  {
    return new_password;
  }
  if (var == "BOARD_NAME")
  {
    return name;
  }
  if (var == "AUTH_TOKEN")
  {
    return auth_token;
  }
  if (var == "SENSOR_ID")
  {
    return sensor_id;
  }
  return String();
}