#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

//帧头和帧尾
// #define FRAME_HEADER 0xAA
// #define FRAME_FOOTER 0x55

// WiFi配置
const char *ssid = "your wifiname";
const char *password = "your wifipassword";

// MQTT配置
const char *mqtt_broker = "your mqttbroker";
const char *topic = "your topic";
const char *mqtt_username = "yours";
const char *mqtt_password = "yours";
const int mqtt_port = 8883;//1883等，根据你的协议定端口
const char *clientid = "your id";

//定义全局对象
WiFiClientSecure esp_client;
PubSubClient client(esp_client);

// Root CA Certificate
const char *ca_cert = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDrzCCApegAwIBAgIQCDvgVpBCRrGhdWrJWZHHSjANBgkqhkiG9w0BAQUFADBh
MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3
d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBD
QTAeFw0wNjExMTAwMDAwMDBaFw0zMTExMTAwMDAwMDBaMGExCzAJBgNVBAYTAlVT
MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j
b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IENBMIIBIjANBgkqhkiG
9w0BAQEFAAOCAQ8AMIIBCgKCAQEA4jvhEXLeqKTTo1eqUKKPC3eQyaKl7hLOllsB
CSDMAZOnTjC3U/dDxGkAV53ijSLdhwZAAIEJzs4bg7/fzTtxRuLWZscFs3YnFo97
nh6Vfe63SKMI2tavegw5BmV/Sl0fvBf4q77uKNd0f3p4mVmFaG5cIzJLv07A6Fpt
43C/dxC//AH2hdmoRBBYMql1GNXRor5H4idq9Joz+EkIYIvUX7Q6hL+hqkpMfT7P
T19sdl6gSzeRntwi5m3OFBqOasv+zbMUZBfHWymeMr/y7vrTC0LUq7dBMtoM1O/4
gdW7jVg/tRvoSSiicNoxBN33shbyTApOB6jtSj1etX+jkMOvJwIDAQABo2MwYTAO
BgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4EFgQUA95QNVbR
TLtm8KPiGxvDl7I90VUwHwYDVR0jBBgwFoAUA95QNVbRTLtm8KPiGxvDl7I90VUw
DQYJKoZIhvcNAQEFBQADggEBAMucN6pIExIK+t1EnE9SsPTfrgT1eXkIoyQY/Esr
hMAtudXH/vTBH1jLuG2cenTnmCmrEbXjcKChzUyImZOMkXDiqw8cvpOp/2PV5Adg
06O/nVsJ8dWO41P0jmP6P6fbtGbfYmbW0W5BjfIttep3Sp+dWOIrWcBAI+0tKIJF
PnlUkiaY4IBIqDfv8NZ5YBberOgOzW6sRBc4L0na4UU+Krk2U886UAb3LujEV0ls
YSEY1QSteDwsOoBrp+uvFRTp2InBuThs4pFsiv9kuXclVzDAGySj4dzp30d8tbQk
CAUw7C29C79Fv1C5qfPrmAESrciIxpg0X40KPMbp1ZWVbd4=
-----END CERTIFICATE-----
)EOF";

//WIFI连接
void Wifi_connect() 
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi连接成功");
  Serial.print("IP地址: ");
  Serial.println(WiFi.localIP());
}

//MQTT连接
void connectToMQTT() 
{
  while (!client.connected()) 
  {
    Serial.print("正在连接MQTT服务器...");
    String clientId = "ESP32-" + String(WiFi.macAddress());
    
    if (client.connect(clientId.c_str(), mqtt_username, mqtt_password)) 
    {
      Serial.println("\nMQTT连接成功");
      client.subscribe(topic);
    } else
    {
      Serial.print("连接失败, rc=");
      Serial.print(client.state());
      Serial.println(" 5秒后重试...");
      delay(5000);
    }
  }
}

// 串口接收缓冲区
byte serialBuffer[2]; 
int bufferIndex = 0;
uint32_t lastByteTime = 0;

void setup() 
{
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, 16);
  
  Wifi_connect();

  esp_client.setCACert(ca_cert);
  client.setServer(mqtt_broker, mqtt_port);
  client.setKeepAlive(60);
  connectToMQTT();
}

void loop() 
{
  if (!client.connected()) 
  {
    connectToMQTT();
  }
  client.loop();

  while (Serial2.available() > 0) 
  {
    byte incomingByte = Serial2.read();
    if (bufferIndex < 2) {
      serialBuffer[bufferIndex] = incomingByte;
      bufferIndex++;
      lastByteTime = millis();
    }

    // 超时检测（300ms）
    if (bufferIndex > 0 && (millis() - lastByteTime > 300)) 
    {
      bufferIndex = 0;
      Serial.println("接收超时，重置缓冲区");
    }

    if (bufferIndex >= 2) 
    {
      processData();
      bufferIndex = 0;
    }
  }
}

void processData() {
  uint8_t heartRate = serialBuffer[0];
  //暂定
  uint8_t bloodOxygen = serialBuffer[1];
  
  // 数据校验(正常人体无法达到的血氧和心率舍弃，防止外界干扰)
  if (heartRate < 40 || heartRate > 200 || bloodOxygen < 70 || bloodOxygen > 100) 
  {
    Serial.println("无效数据: HR=" + String(heartRate) + " SPO2=" + String(bloodOxygen));
    return;
  }

  DynamicJsonDocument doc(256);
  doc["HR"] = heartRate;
  doc["SP02"] = bloodOxygen;

  char jsonBuffer[256];
  serializeJson(doc, jsonBuffer);

  if (client.publish(topic, jsonBuffer)) 
  {
    Serial.print("已发布: ");
  } else 
  {
    Serial.print("发布失败: ");
  }
  Serial.println(jsonBuffer);
}

void printMemoryStats() 
{
  Serial.printf("Free Heap: %d\n", ESP.getFreeHeap());
}
