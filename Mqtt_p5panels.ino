#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>  // Để hỗ trợ kết nối TLS
#include <PubSubClient.h>
#include <Fonts/FreeSansBold12pt7b.h> 

//----------------------------------------Defines the connected PIN between RGB LED Matrix P5 and ESP32.
#define R1_PIN 25
#define G1_PIN 26
#define B1_PIN 27
#define R2_PIN 14
#define G2_PIN 12
#define B2_PIN 13

#define A_PIN 23
#define B_PIN 19
#define C_PIN 5
#define D_PIN 17
#define E_PIN -1  

#define LAT_PIN 4
#define OE_PIN 15
#define CLK_PIN 16

#define PANEL_RES_X 64  
#define PANEL_RES_Y 32  
#define PANEL_CHAIN 1   

const char *ssid = "Hala madrid";
const char *password = "hoibomicho";

const char *mqtt_broker = "5e37a9a8868d4cd88d811e3348dcca82.s1.eu.hivemq.cloud";
const char *topic = "gia/dau";
const char *mqtt_user = "doancayxang";
const char *mqtt_password = "6LVpeZUZzu5Wfb@";
const int mqtt_port = 8883;

String Message = "Hi Im ";

WiFiClientSecure espClient;  // Sử dụng WiFiClientSecure cho TLS
PubSubClient client(espClient);

MatrixPanel_I2S_DMA *dma_display = nullptr;

void setup() {
  Serial.begin(115200);
  espClient.setInsecure();  // Dùng nếu bạn không có chứng chỉ CA
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");

  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);

  HUB75_I2S_CFG mxconfig(PANEL_RES_X, PANEL_RES_Y, PANEL_CHAIN);
  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->begin();
  dma_display->setBrightness8(90);
  dma_display->setTextColor(dma_display->color444(0,15,15));

  dma_display->setCursor(0, 16);
  dma_display->print(":)");
  
  reconnect();
  client.subscribe(topic);
  client.publish("dsp_ronin", "dsp++");
}

void reconnect() {
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
    if (client.connect("ESP32_Client", mqtt_user, mqtt_password)) {
      Serial.println("Connected to MQTT server");
    } else {
      Serial.print("Failed MQTT connection, state: ");
      Serial.println(client.state());
      delay(2000);
    }
  }
}

void callback(char *topic, byte *payload, unsigned int length) {
  payload[length] = 0;
  Message = String((char *) payload);
  delay(100);
  Serial.println(Message);
}

void Scrolling_text(int scroll_speed, String scroll_text) {
  static uint32_t lastUpdate = 0;
  static int16_t x = PANEL_RES_X;
  int16_t x1, y1;
  uint16_t w, h;

  dma_display->getTextBounds(scroll_text, x, 16, &x1, &y1, &w, &h);

  if (millis() - lastUpdate > scroll_speed) {
    dma_display->fillScreen(0);
    dma_display->setCursor(x, 16);
    dma_display->print(scroll_text);

    x--;
    if (x < -w) {
      x = PANEL_RES_X;
    }
    lastUpdate = millis();
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  Scrolling_text(50, Message);
  delay(100);
}
