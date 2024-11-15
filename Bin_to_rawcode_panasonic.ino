#include <FS.h>
#include <IRsend.h>
#include "AcCommand.h"
#include <PubSubClient.h>
#include "webConfigWifiAndMQTT.h"
#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ArduinoOTA.h>
#include <IRutils.h>
#include <IRrecv.h>


// Constants
const uint16_t kIrled = 4;                 // IR LED pin
const uint16_t kRecvPin = 14;              // IR Receiver pin
const uint32_t kBraudRate = 115200;        // Serial communication baud rate
const uint16_t kCaptureBufferSize = 1024;  // Buffer size for IR capture

// IR timing settings
const uint8_t kTimeout = 15;
const uint16_t kMinUnknownSize = 12;
const uint8_t kTolerancePercentage = kTolerance;

// Device settings and state variables
unsigned long lastReconnectAttempt = 0;
uint8_t temp = 28;
uint8_t mode = 1;
uint8_t fan_mode = 0;
uint8_t swing_mode = 0;
char deviceName[20] = "ESP8266";
char wm_device_name_identifier[12] = "device_name";
bool power = false;
bool shouldSaveConfig = false;
bool checkWiFiSaveInFile = false;

// IR signal storage
uint16_t short_rawSignal[SHORT_CODE_LENGTH] = {
  3586, 1790, 422, 462, 422, 1334, 422, 462, 422, 468, 422, 462, 424, 462, 422,
  462, 422, 468, 422, 462, 422, 462, 422, 462, 422, 468, 422, 462, 422, 1334,
  422, 462, 424, 468, 422, 462, 422, 462, 422, 462, 422, 468, 422, 462, 424,
  1334, 422, 1334, 422, 1340, 422, 462, 422, 464, 422, 1334, 422, 468, 422,
  462, 424, 462, 422, 462, 422, 468, 422, 464, 422, 462, 422, 462, 422, 468,
  422, 462, 422, 462, 422, 464, 422, 468, 422, 464, 422, 462, 422, 464, 422,
  468, 422, 464, 422, 462, 422, 464, 422, 470, 422, 464, 422, 462, 422, 464,
  422, 468, 422, 464, 422, 464, 422, 464, 422, 470, 422, 488, 396, 1334, 422,
  1360, 396, 470, 422, 464, 422, 464, 422, 488, 396, 490, 480
};

// WiFi settings
struct WifiSettings {
  char apName[20] = "ESP8266-config";
  char apPassword[20];
  char SSID[20] = "IoT";
  char password[20] = "thoakimdo";

  char old_SSID[20];
  char old_password[20];
  char wm_old_wifi_ssid_identifier[9] = "old_ssid";
  char wm_old_wifi_password_identifier[18] = "old_wifi_password";
  char wm_wifi_ssid_identifier[5] = "ssid";
  char wm_wifi_password_identifier[14] = "wifi_password";
} wifisettings;

// MQTT settings
struct MQTTSettings {
  char client_id[20];
  char hostname[20];
  char port[6];
  char topicSet[40];
  char topicGet[40];
  char user[20];
  char password[20];
  char wm_mqtt_client_id_identifier[15] = "mqtt_client_id";
  char wm_mqtt_hostname_identifier[14] = "mqtt_hostname";
  char wm_mqtt_port_identifier[10] = "mqtt_port";
  char wm_mqtt_user_identifier[10] = "mqtt_user";
  char wm_mqtt_password_identifier[14] = "mqtt_password";
  char wm_mqtt_topicGet_identifier[14] = "mqtt_topicGet";
  char wm_mqtt_topicSet_identifier[14] = "mqtt_topicSet";
} mqttsettings;

// IR communication objects
IRrecv irrecv(kRecvPin, kCaptureBufferSize, kTimeout, true);
decode_results results;
IRsend irsend(kIrled);

// Network and web server objects
WiFiClient espClient;
PubSubClient mqtt_client(espClient);
WiFiManager wifimanager;
ESP8266WebServer webServer(80);
ESP8266HTTPUpdateServer httpUpdater;

void readSettingFromConfig() {
  // clean FS for testing
  // SPIFFS.format();

  // Read config from SPIFFS
  //Serial.println("Mounting FS....");

  if (SPIFFS.begin()) {
    //Serial.println("File System Mounted");
    if (SPIFFS.exists("/config.json")) {
      //Serial.println("Reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        //Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        if (size > 0) {
          // Whenever we read a line, we'll append it to this string.
          std::unique_ptr<char[]> buf(new char[size + 1]);
          configFile.readBytes(buf.get(), size);
          buf.get()[size] = 0;

          StaticJsonDocument<1024> doc;
          DeserializationError error = deserializeJson(doc, buf.get());
          if (error) {
            //Serial.println(F("Failed to load JSON config"));
          } else {
            //Serial.println("\nParsed JSON succesfully");

            // Read information from JSON and copy in structure
            strncpy(mqttsettings.client_id, doc[mqttsettings.wm_mqtt_client_id_identifier], sizeof(mqttsettings.client_id));
            strncpy(mqttsettings.hostname, doc[mqttsettings.wm_mqtt_hostname_identifier], sizeof(mqttsettings.hostname));
            strncpy(mqttsettings.port, doc[mqttsettings.wm_mqtt_port_identifier], sizeof(mqttsettings.port));
            strncpy(mqttsettings.user, doc[mqttsettings.wm_mqtt_user_identifier], sizeof(mqttsettings.user));
            strncpy(mqttsettings.password, doc[mqttsettings.wm_mqtt_password_identifier], sizeof(mqttsettings.password));
            strncpy(mqttsettings.topicGet, doc[mqttsettings.wm_mqtt_topicGet_identifier], sizeof(mqttsettings.topicGet));
            strncpy(mqttsettings.topicSet, doc[mqttsettings.wm_mqtt_topicSet_identifier], sizeof(mqttsettings.topicSet));

            strncpy(wifisettings.SSID, doc[wifisettings.wm_wifi_ssid_identifier], sizeof(wifisettings.SSID));
            strncpy(wifisettings.password, doc[wifisettings.wm_wifi_password_identifier], sizeof(wifisettings.password));
            strncpy(wifisettings.old_SSID, doc[wifisettings.wm_old_wifi_ssid_identifier], sizeof(wifisettings.old_SSID));
            strncpy(wifisettings.old_password, doc[wifisettings.wm_old_wifi_password_identifier], sizeof(wifisettings.old_password));
            strncpy(deviceName, doc[wm_device_name_identifier], sizeof(deviceName));
            

            //Serial.print("The length of SSID in WM_SETTINGS:");
            //Serial.println(strlen(wifisettings.SSID));
            //Serial.print("The length of password in WM_SETTINGS:");
            //Serial.println(strlen(wifisettings.password));
            if (strlen(wifisettings.SSID) != 0 && strlen(wifisettings.password) != 0) {
              checkWiFiSaveInFile = true;
            }
          }
        } else {
          //Serial.println("Config file is empty");
        }
        configFile.close();
      } else {
        //Serial.println("Failed to open config file");
      }
    } else {
      //Serial.println("Config file does not exist");
    }
  } else {
    //Serial.println("Failed to mout file system");
  }
}

void convertToRawSignal(uint16_t *encodedSignal) {
  uint8_t binaryData[19];
  encodePanasonicIR(binaryData, temp, mode, power, fan_mode, swing_mode);
  binaryToRawSignal(binaryData, encodedSignal);
}

void processMQTTMessage(char *topic, String receivedMessage) {
  bool capture = false;
  String subTopic = "";  // To store the main sub-topic of the MQTT message
  int topicLength = strlen(topic);

  // Extract sub-topic after the first occurrence of '$'
  for (int i = 0; i < topicLength; ++i) {
    if (topic[i] == '$') {
      capture = true;
      continue;
    }
    if (capture) {
      subTopic += topic[i];
    }
  }

  // Handle the extracted sub-topic
  if (subTopic == "temperature") {
    temp = receivedMessage.toInt();
  } else if (subTopic == "mode") {
    mode = receivedMessage.toInt();
  } else if (subTopic == "fan") {
    fan_mode = receivedMessage.toInt();
  } else if (subTopic == "swing") {
    swing_mode = receivedMessage.toInt();
  }

  // Set power based on mode (assuming mode 5 indicates power off)
  power = (mode == 5) ? false : true;

  // Display received values
  //Serial.println();
  //Serial.print("Received Temperature from MQTT: ");
  //Serial.println(temp);
  //Serial.print("Received Mode from MQTT: ");
  //Serial.println(mode);
  //Serial.print("Received Fan Mode from MQTT: ");
  //Serial.println(fan_mode);
  //Serial.print("Received Swing Mode from MQTT: ");
  //Serial.println(swing_mode);
  //Serial.print("Power Status from MQTT: ");
  //Serial.println(power);

  // Encode and send IR signal
  uint16_t encodedSignal[MAX_CODE_LENGTH];
  convertToRawSignal(encodedSignal);
  irsend.sendRaw(short_rawSignal, SHORT_CODE_LENGTH, 38);
  irsend.sendRaw(encodedSignal, MAX_CODE_LENGTH, 38);
}

void mqttCallBack(char *topic, byte *payload, unsigned int length) {
  // Hàm gọi lại khi nhận được tin nhắn từ MQTT
  String reveiceMessage = "";
  //Serial.print("Message received on topic: ");
  //Serial.print(topic);
  //Serial.print(" with message: ");

  // Chuyển đổi payload thành chuỗi để sử dụng
  int i = 0;
  for (i; i < length; ++i) {
    reveiceMessage += (char)payload[i];
  }

  //Serial.println();
  //Serial.print(reveiceMessage);
  //Serial.println();
  //Serial.println("-----------------------------");
  processMQTTMessage(topic, reveiceMessage);
}

void initializeMqttClient() {
  //Serial.println("Local IP: ");
  //Serial.println(WiFi.localIP());

  mqtt_client.setServer(mqttsettings.hostname, atoi(mqttsettings.port));
  mqtt_client.setCallback(mqttCallBack);
}

void connectToMQTTBroker() {
  // Hàm kết nối đến MQTT Broker
  if (!mqtt_client.connected()) {
    if (millis() - lastReconnectAttempt > 5000) {
      lastReconnectAttempt = millis();

      // Tạo client_id với định dạng đúng
      String client_id = String(mqttsettings.client_id) + "-" + String(WiFi.macAddress());
      //Serial.printf("Connecting to MQTT broker as %s......... \n", client_id.c_str());

      // Kết nối đến MQTT với client_id, user, password
      if (mqtt_client.connect(mqttsettings.client_id, mqttsettings.user, mqttsettings.password)) {
        //Serial.println("Connected to MQTT broker");

        // Đăng ký các topic cần thiết

        mqtt_client.subscribe(strcat(mqttsettings.topicGet, "/$mode"));
        mqtt_client.subscribe(strcat(mqttsettings.topicGet, "/$temperature"));
        mqtt_client.subscribe(strcat(mqttsettings.topicGet, "/$fan"));
        mqtt_client.subscribe(strcat(mqttsettings.topicGet, "/$swing"));

        lastReconnectAttempt = 0;
      } else {
        //Serial.println("Connection failed");
        //Serial.println(mqtt_client.state());
        //Serial.println(" try again in 5 seconds");
      }
    }
  } else {
    mqtt_client.loop();  // Xử lý kết nối MQTT
  }
}

void connectToWiFi() {
  WiFi.begin(wifisettings.SSID, wifisettings.password);
  Serial.print("Connecting to WiFi........");

  // Time start connecting
  unsigned long startAttemptTime = millis();

  // Try to connect WiFi in 20s
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 20000) {
    delay(500);
    //Serial.print(".");
  }

  if (WiFi.status() != WL_CONNECTED) {
    //Serial.println("\nCould not connect to new WiFi, try with old WiFi......!");

    WiFi.begin(wifisettings.old_SSID, wifisettings.old_password);
    startAttemptTime = millis();

    // Try to connect with old WIFI
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 20000) {
      delay(500);
      //Serial.print(".");
    }

    // Check connection again
    if (WiFi.status() == WL_CONNECTED) {
      //Serial.println("Connected to WiFi");
    } else {
      //Serial.println("Could not connect to WiFi");
    }
  } else {
    Serial.println("Connected to WiFi");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }
  createAccessPoint();
}

void setSaveConfigCallBack() {
  //Serial.println("Should save config");
  shouldSaveConfig = true;
}
void initializeWiFiManager() {
  WiFiManagerParameter custom_mqtt_client("client_id", "MQTT Client ID", mqttsettings.client_id, 20);
  WiFiManagerParameter custom_mqtt_server("server", "MQTT Server", mqttsettings.hostname, 40);
  WiFiManagerParameter custom_mqtt_port("port", "MQTT Port", mqttsettings.port, 6);
  WiFiManagerParameter custom_mqtt_user("user", "MQTT User", mqttsettings.user, 20);
  WiFiManagerParameter custom_mqtt_pass("pass", "MQTT Password", mqttsettings.password, 20);
  WiFiManagerParameter custom_mqtt_topic_get("topicGet", "Topic Get", mqttsettings.topicGet, 40);
  WiFiManagerParameter custom_mqtt_topic_set("topicSet", "Topic Set", mqttsettings.topicSet, 40);

  // Rest wifi setiing for testing
  // wifimanager.resetSettings();

  wifimanager.setSaveConfigCallback(setSaveConfigCallBack);

  // add all your parameters here
  wifimanager.addParameter(&custom_mqtt_client);
  wifimanager.addParameter(&custom_mqtt_server);
  wifimanager.addParameter(&custom_mqtt_port);
  wifimanager.addParameter(&custom_mqtt_user);
  wifimanager.addParameter(&custom_mqtt_pass);
  wifimanager.addParameter(&custom_mqtt_topic_get);
  wifimanager.addParameter(&custom_mqtt_topic_set);

  // Start auto create an Access Point in the first time programmed
  createAccessPoint();

  strcpy(mqttsettings.client_id, custom_mqtt_client.getValue());
  strcpy(mqttsettings.hostname, custom_mqtt_server.getValue());
  strcpy(mqttsettings.port, custom_mqtt_port.getValue());
  strcpy(mqttsettings.user, custom_mqtt_user.getValue());
  strcpy(mqttsettings.password, custom_mqtt_pass.getValue());
  strcpy(mqttsettings.topicGet, custom_mqtt_topic_get.getValue());
  strcpy(mqttsettings.topicSet, custom_mqtt_topic_set.getValue());
}

void createAccessPoint() {
  // set timeout until configuration
  wifimanager.setConfigPortalTimeout(360);

  // set minium quality of signal
  // wifimanager.setMinimumSignalQuality();

  if (!wifimanager.autoConnect(wifisettings.apName)) {
    //Serial.println("failed to connect and hit time out");
    delay(3000);

    // reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  } else {
    strcpy(wifisettings.SSID, wifimanager.getWiFiSSID().c_str());
    strcpy(wifisettings.password, wifimanager.getWiFiPass().c_str());
  }
  //Serial.println("WiFi connected!");
}
void handleRoot() {
  String s = FPSTR(MainPage);
  s.replace("%devicename%", String(deviceName));
  s.replace("%macdevice%", WiFi.macAddress());
  s.replace("%ipdevice%", WiFi.localIP().toString());
  webServer.send(200, "text/html", s);
}

void changeWiFiParameters() {
  String new_ssid = webServer.arg("ssid");
  String new_password = webServer.arg("password");

  //Serial.print("SSID get from Web: ");
  //Serial.println(new_ssid);
  //Serial.print("Password get from web: ");
  //Serial.println(new_password);

  strcpy(wifisettings.old_SSID, wifisettings.SSID);
  strcpy(wifisettings.old_password, wifisettings.password);
  new_ssid.toCharArray(wifisettings.SSID, sizeof(wifisettings.SSID));
  new_password.toCharArray(wifisettings.password, sizeof(wifisettings.password));

  //Serial.print("SSID copy to wifisettings: ");
  //Serial.println(wifisettings.SSID);
  //Serial.print("Password copy to wifisetting: ");
  //Serial.println(wifisettings.password);
  //Serial.print("SSID copy vào old_SSID: ");
  //Serial.println(wifisettings.old_SSID);
  //Serial.print("Pass copy vào old_password: ");
  //Serial.println(wifisettings.old_password);

  saveConfig();

  String response = FPSTR(wifiSuccessfulConnectionWeb);
  webServer.send(200, "text/html", response);

  delay(2000);
  ESP.restart();
}

void changeMQTTParameters() {
  String new_mqtt_hostname = webServer.arg("server");
  String new_mqtt_client_id = webServer.arg("clientID");
  String new_mqtt_port = webServer.arg("port");
  String new_mqtt_user = webServer.arg("user");
  String new_mqtt_password = webServer.arg("pass");
  String new_mqtt_topic_get = webServer.arg("topicget");
  String new_mqtt_topic_set = webServer.arg("topicset");

  new_mqtt_client_id.toCharArray(mqttsettings.client_id, sizeof(mqttsettings.client_id));
  new_mqtt_hostname.toCharArray(mqttsettings.hostname, sizeof(mqttsettings.hostname));
  new_mqtt_port.toCharArray(mqttsettings.port, sizeof(mqttsettings.port));
  new_mqtt_user.toCharArray(mqttsettings.user, sizeof(mqttsettings.user));
  new_mqtt_password.toCharArray(mqttsettings.password, sizeof(mqttsettings.password));
  new_mqtt_topic_get.toCharArray(mqttsettings.topicGet, sizeof(mqttsettings.topicGet));
  new_mqtt_topic_set.toCharArray(mqttsettings.topicSet, sizeof(mqttsettings.topicSet));

  // Set flag for configuration and store if need
  saveConfig();

  String response = FPSTR(mqttSuccessfulConnectionWeb);
  response.replace("%devicename%", deviceName);
  response += "<div style='display: flex; align-items: center; justify-content: center; "
              "margin-top: 20px; background-color: #e0e0e0; padding: 10px 20px; "
              "border-radius: 5px; box-shadow: 0px 4px 8px rgba(0, 0, 0, 0.2);'>";

  response += "<a style='text-decoration: none; color: darkmagenta; font-size: 1.2em; font-weight: bold;' ";
  response += "href='http://" + WiFi.localIP().toString() + "'>Go to Device Page</a></div>";

  webServer.send(200, "text/html", response);
  delay(2000);

  ESP.restart();
}

void changeDeviceName(){
  String new_device_name = webServer.arg("deviceName");
  new_device_name.toCharArray(deviceName, sizeof(deviceName));
  saveConfig();
  
  String response = FPSTR(mqttSuccessfulConnectionWeb);
  response.replace("%devicename%", deviceName);
    response += "<div style='display: flex; align-items: center; justify-content: center; "
              "margin-top: 20px; background-color: #e0e0e0; padding: 10px 20px; "
              "border-radius: 5px; box-shadow: 0px 4px 8px rgba(0, 0, 0, 0.2);'>";

  response += "<a style='text-decoration: none; color: darkmagenta; font-size: 1.2em; font-weight: bold;' ";
  response += "href='http://" + WiFi.localIP().toString() + "'>Go to Device Page</a></div>";
  webServer.send(200, "text/html", response);
  delay(2000);

  ESP.restart();
}

void saveConfig() {
  //Serial.println("\n---------------------------------------------------");
  //Serial.println("Saving config");

  StaticJsonDocument<1024> doc;
  doc[mqttsettings.wm_mqtt_client_id_identifier] = mqttsettings.client_id;
  doc[mqttsettings.wm_mqtt_hostname_identifier] = mqttsettings.hostname;
  doc[mqttsettings.wm_mqtt_port_identifier] = mqttsettings.port;
  doc[mqttsettings.wm_mqtt_user_identifier] = mqttsettings.user;
  doc[mqttsettings.wm_mqtt_password_identifier] = mqttsettings.password;
  doc[mqttsettings.wm_mqtt_topicGet_identifier] = mqttsettings.topicGet;
  doc[mqttsettings.wm_mqtt_topicSet_identifier] = mqttsettings.topicSet;

  doc[wifisettings.wm_wifi_ssid_identifier] = wifisettings.SSID;
  doc[wifisettings.wm_wifi_password_identifier] = wifisettings.password;
  doc[wifisettings.wm_old_wifi_password_identifier] = wifisettings.old_password;
  doc[wifisettings.wm_old_wifi_ssid_identifier] = wifisettings.old_SSID;
  doc[wm_device_name_identifier] = deviceName;

  //Serial.print("Doc client id: ");
  //Serial.println(doc[mqttsettings.wm_mqtt_client_id_identifier].as<String>());
  //Serial.print("DOC hostname: ");
  //Serial.println(doc[mqttsettings.wm_mqtt_hostname_identifier].as<String>());
  //Serial.print("DOC mqtt user: ");
  //Serial.println(doc[mqttsettings.wm_mqtt_user_identifier].as<String>());
  //Serial.print("DOC mqtt password: ");
  //Serial.println(doc[mqttsettings.wm_mqtt_password_identifier].as<String>());
  //Serial.print("DOC ssid user: ");
  //Serial.println(doc[wifisettings.wm_wifi_ssid_identifier].as<String>());
  //Serial.print("DOC wifi password: ");
  //Serial.println(doc[wifisettings.wm_wifi_password_identifier].as<String>());
  //Serial.print("DOC old ssid user: ");
  //Serial.println(doc[wifisettings.wm_old_wifi_ssid_identifier].as<String>());
  //Serial.print("DOC old wifi password: ");
  //Serial.println(doc[wifisettings.wm_old_wifi_password_identifier].as<String>());

  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    //Serial.println("Failed to open config file for writing");
    return;
  } else {
    //Serial.println("Success to open config file for writing");
  }

  // Writing JSON data into file
  if (serializeJson(doc, configFile) == 0) {
    //Serial.println("JSON data is not valid");
  } else {
    //Serial.println("JSON data is valid");
  }
  configFile.close();
}

void processBinaryIRSignalAndSendToMQTT(String resultData) {
  if (resultData.length() == MAX_CODE_BIT_LENGTH) {
    const char *temperature = returnTempForMQTT(resultData);
    const char *mode = returnModeForMQTT(resultData);
    const char *swing_mode = returnSwingModeForMQTT(resultData);
    const char *fan_mode = returnFanModeForMQTT(resultData);
    const char *power = returnPowerForMQTT(resultData);
    //Serial.print("\nTemperature: ");
    //Serial.print(temperature);
    //Serial.print("\nMode: ");
    //Serial.print(mode);
    //Serial.print("\nSwing: ");
    //Serial.print(swing_mode);
    //Serial.print("\nFan: ");
    //Serial.print(fan_mode);
    //Serial.print("\nPower: ");
    //Serial.print(power);
    mqtt_client.publish(strcat(mqttsettings.topicSet, "/temperature"), temperature);
    mqtt_client.publish(strcat(mqttsettings.topicSet, "/mode"), mode);
    mqtt_client.publish(strcat(mqttsettings.topicSet, "/swing"), swing_mode);
    mqtt_client.publish(strcat(mqttsettings.topicSet, "/fan"), fan_mode);
    mqtt_client.publish(strcat(mqttsettings.topicSet, "/mode"), power);
  }
}

String getRawValueFromIR(const decode_results *const results) {
  String output = "";
  const uint16_t length = getCorrectedRawLength(results);

  // Loop through the raw data and append each value to output
  for (uint16_t i = 1; i < results->rawlen; i++) {
    uint32_t usecs;
    for (usecs = results->rawbuf[i] * kRawTick; usecs > UINT16_MAX; usecs -= UINT16_MAX) {
      output += uint64ToString(UINT16_MAX);
      output += ", ";
    }
    output += uint64ToString(usecs, 10);
    if (i < results->rawlen - 1)
      output += ", ";  // ',' not needed on the last one
  }

  return output;
}

void executeIRsignal() {
  // Check if the IR code has been received.
  if (irrecv.decode(&results)) {
    String binaryData = "";
    String rawSignal = getRawValueFromIR(&results);
    yield();

    binaryData = turnRawSignalToBinary(rawSignal);
    // //Serial.print("Length of binary: ");
    // //Serial.println(binaryData.length());
    yield();

    processBinaryIRSignalAndSendToMQTT(binaryData);
  }
}

void blinkLed() {
  digitalWrite(LED_BUILTIN, HIGH);  // turn the LED on (HIGH is the voltage level
  delay(1000);                      // wait for a second
  digitalWrite(LED_BUILTIN, LOW);   // turn the LED off by making the voltage LOW
  delay(1000);
}


void setup() {
  Serial.begin(115200);
  //-----------------------SPIFFS-----------------------------
  readSettingFromConfig();
  //-----------------------WIFI-------------------------------
  if (checkWiFiSaveInFile) {
    connectToWiFi();
  } else {
    initializeWiFiManager();
  }if (shouldSaveConfig) {
    saveConfig();  // Lưu cấu hình nếu có thay đổi
  }


  //------------------------MQTT-------------------------------
  initializeMqttClient();
  //------------------------IRSEND--------------------------
  irsend.begin();
  //-----------------------IRreceive------------------------
  Serial.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY);
  while (!Serial)
    delay(50);
#if DECODE_HASH
  irrecv.setUnknownThreshold(kMinUnknownSize);
#endif
  irrecv.setTolerance(kTolerancePercentage);
  irrecv.enableIRIn();

  //-----------------------WEBSERVER---------------------------
  webServer.on("/", handleRoot);
  webServer.on("/changedevicename", HTTP_POST, changeDeviceName);
  webServer.on("/savewifi", HTTP_POST, changeWiFiParameters);
  webServer.on("/savemqtt", HTTP_POST, changeMQTTParameters);

  webServer.begin();
  //Serial.println("HTTP server started");
  //----------------------OTA------------------------------------
  httpUpdater.setup(&webServer, "/update");
}

void loop() {
  connectToMQTTBroker();
  executeIRsignal();
  webServer.handleClient();
}