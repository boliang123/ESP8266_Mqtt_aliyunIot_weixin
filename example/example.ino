#include <dht11.h> //引入DHT11库
#include <Wire.h>
#include <Adafruit_SSD1306.h>
/*阿里云SDK*/
#include <AliyunIoTSDK.h>
#include <WiFiClient.h> // 引用 WiFi 库
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "aliyun_mqtt.h"
#include <ESP8266WiFi.h>

/* 配置WIFI */
// #define WIFI_SSID "TYSB"
// #define WIFI_PASSWD "xjp123456"
#define WIFI_SSID "TYSB"
#define WIFI_PASSWD "xjp123456"

// 阿里云IOT平台信息 修改为你自己的
#define PRODUCT_KEY "xxxxx"                        // 产品KEY
#define DEVICE_NAME "xxx"                              // 设备名字
#define DEVICE_SECRET "xxxxxx" // 设备密钥
#define REGION_ID "cn-shanghai"                          // 连接服务器

// 不需要改
// #define MQTT_SERVER PRODUCT_KEY ".iot-as-mqtt." REGION_ID ".aliyuncs.com"
// #define MQTT_PORT 1883
// #define MQTT_USRNAME DEVICE_NAME "&" PRODUCT_KEY

// 上传数据格式
#define ALINK_BODY_FORMAT "{\"id\":\"ESP8266\",\"version\":\"1.0\",\"method\":\"thing.event.property.post\",\"params\":%s}"

// 对应阿里云的设备连接信息 修改为你自己的
#define CLIENT_ID "xxxxxxxx"
#define MQTT_PASSWD "xxxx"
// 订阅的 MQTT 主题
#define ALINK_TOPIC_PROP_SET "/sys/" PRODUCT_KEY "/" DEVICE_NAME "/thing/service/property/set"
// 发布的topic 上传数据
#define ALINK_TOPIC_PROP_POST "/sys/" PRODUCT_KEY "/" DEVICE_NAME "/thing/event/property/post"



const char *mqttServer = "a1ITu4LFqZf.iot-as-mqtt.cn-shanghai.aliyuncs.com"; // 替换成您的 MQTT 服务器地址
const int mqttPort = 1883;                                                   // MQTT 服务器端口号

unsigned long lastMs = 0;
long s;

static WiFiClient espClient;
PubSubClient mqttClient(espClient);

/********************###定义###********************/
dht11 DHT11;         // 定义传感器类型
#define DHT11PIN1 2  // 定义传感器连接引脚。此处的PIN2在NodeMcu8266开发板上对应的引脚是D4
#define DHT11PIN2 13 // 定义传感器连接引脚。此处的PIN2在NodeMcu8266开发板上对应的引脚是D7

#define JPIN 15       // 定义继电器引脚D7
#define BUZZER_PIN 12 // 定义有源蜂鸣器连接的引脚 D6

Adafruit_SSD1306 display(128, 64, &Wire, -1);

void displayTempHumidity(float temp1, float humidity1, float temp2, float humidity2)
{
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.print("Device1: ");
    display.setCursor(10, 10);
    display.print("Temp1: ");
    display.print(temp1, 1);
    display.println("C");
    display.setCursor(10, 20);
    display.print("Humi1: ");
    display.print(humidity1, 1);
    display.println("%");
    display.setCursor(0, 30);
    display.print("Device2: ");
    display.setCursor(10, 40);
    display.print("Temp2: ");
    display.print(temp2, 1);
    display.println("C");
    display.setCursor(10, 50);
    display.print("Humi2: ");
    display.print(humidity2, 1);
    display.println("%");
    display.display();
}

/********************###程序初始化###********************/
void setup()
{
    pinMode(LED_BUILTIN, OUTPUT); // led

    Serial.begin(115200); // 设置波特率为115200
    Serial.println("\n");
    Serial.println("esp8266读取DHT11传感器数值 ");
    Serial.println("串口会分别打印当前湿度（%），当前摄氏度温度，当前华氏度温度和当前开式温度 ");
    Serial.print("DHT11库文件版本: ");
    Serial.println(DHT11LIB_VERSION);
    Serial.println();

    // 初始化显示屏
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    {
        Serial.println("SSD1306初始化失败");
        while (1)
            ;
    }
    display.clearDisplay();
    display.display();

    // 初始化有源蜂鸣器
    pinMode(BUZZER_PIN, OUTPUT);    // 设置有源蜂鸣器引脚为输出模式
    digitalWrite(BUZZER_PIN, HIGH); // 停止有源蜂鸣器
    // 启动 I2C 通信
    Wire.begin();

    // 初始化WIFI
    wifiInit(WIFI_SSID, WIFI_PASSWD);


    pinMode(JPIN, OUTPUT);


    mqttClient.setCallback(mqtt_callback); // 设置回调函数
    mqtt_check_connect();                  // 连接 MQTT

}

// 连接MQTT
void mqtt_check_connect()
{
    while (!mqttClient.connected())
    {
        while (connect_aliyun_mqtt(mqttClient, PRODUCT_KEY, DEVICE_NAME, DEVICE_SECRET))
        {
            Serial.println("MQTT connect succeed!");

            // 订阅消息
            mqttClient.subscribe(ALINK_TOPIC_PROP_SET);

            Serial.println("subscribe done");
        }
    }
}

// 接收处理微信端发送的数据
void mqtt_callback(char *topic, byte *payload, unsigned int length) // mqtt回调函数
{
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    payload[length] = '\0';
    Serial.print("****** ");
    Serial.println((char *)payload);
    Serial.print("****** ");
    // https://arduinojson.org/v5/assistant/  json数据解析网站

    // const char* jsonString = (char *)payload;
    Serial.println((char *)payload);

    if (strstr(topic, ALINK_TOPIC_PROP_SET))
    {
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, payload, length);
        const char *cmd = doc["cmd"];
        int data = doc["data"];
        Serial.print("cmd is:");
        Serial.println(cmd);
        Serial.print("data is:");
        Serial.println(data);
        // doc.clear();//clear() 函数释放内存
        if (data == 1)
        {
            // 接收到数字1 时，蜂鸣器开
            digitalWrite(BUZZER_PIN, LOW); // 将蜂鸣器引脚设为低电平，使蜂鸣器响起
            // digitalWrite(LED_BUILTIN, LOW);
            delay(500);                     // 等待500毫秒
            digitalWrite(BUZZER_PIN, HIGH); // 将蜂鸣器引脚设为高电平，使蜂鸣器停止响
            // digitalWrite(LED_BUILTIN, HIGH);
            delay(500); // 等待500毫秒
        }
        else
        {
            digitalWrite(BUZZER_PIN, HIGH);
        }
    }
}

// 在连接成功后订阅 MQTT 主题
void mqttConnected()
{
    mqttClient.subscribe(ALINK_TOPIC_PROP_SET);
    Serial.printf("Subscribed to MQTT topic: %s\n", ALINK_TOPIC_PROP_SET);
}

void mqtt_interval_post(float temp1, float humidity1, float temp2, float humidity2)
{
    char param[512];
    char jsonBuf[1024];

    sprintf(param, "{\"CurrentHumidity\":%f,\"CurrentTemperature\":%f,\"CurrentHumidity1\":%f,\"CurrentTemperature1\":%f}", humidity1, temp1, humidity2, temp2);

    Serial.println(jsonBuf);*/
    sprintf(jsonBuf, ALINK_BODY_FORMAT, param);
    Serial.println(jsonBuf);
    mqttClient.publish(ALINK_TOPIC_PROP_POST, jsonBuf); // 这个是上传数据的topic,jsonBuf这个是上传的数据
}

/********************###主函数###********************/
void loop()
{

    Serial.println("\n");
    DHT11.read(DHT11PIN1); // 更新传感器所有信息

    float CurrentHumidity1 = (float)DHT11.humidity;
    Serial.print("当前1湿度 (%): ");
    Serial.println(CurrentHumidity1, 2);

    Serial.print("当前1温度 (℃): ");
    float CurrentTemperature1 = (float)DHT11.temperature;
    Serial.println(CurrentTemperature1, 2);

    DHT11.read(DHT11PIN2); // 更新传感器所有信息

    Serial.print("当前2湿度 (%): ");
    float CurrentHumidity2 = (float)DHT11.humidity;
    Serial.println(CurrentHumidity2, 2);

    Serial.print("当前2温度 (℃): ");
    float CurrentTemperature2 = (float)DHT11.temperature;
    Serial.println(CurrentTemperature2, 2);

    displayTempHumidity(CurrentTemperature1, CurrentHumidity1, CurrentTemperature2, CurrentHumidity2);

    s = millis();
    delay(1000);
    // read_data();
    Serial.println(millis() - s);
    if (millis() - lastMs >= 5000)
    {
        lastMs = millis();
        mqtt_check_connect();
        mqtt_interval_post(CurrentTemperature1, CurrentHumidity1, CurrentTemperature2, CurrentHumidity2);


    }

    delay(2000); // 每两秒打印一次

    display.clearDisplay(); // 清除显示屏内容

    mqttClient.loop();
}

// 初始化 wifi 连接
void wifiInit(const char *ssid, const char *passphrase)
{
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, passphrase);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("WiFi not Connect");
    }
    Serial.println("Connected to AP");
}

// 电源属性修改的回调函数
void powerCallback(JsonVariant p)
{
    int PowerSwitch = p["LightSwitch"];

    // 启动设备
    if (PowerSwitch == 1)
    {
        digitalWrite(BUZZER_PIN, LOW); // 将蜂鸣器引脚设为低电平，使蜂鸣器响起
        digitalWrite(LED_BUILTIN, LOW);
        delay(500);                     // 等待500毫秒
        digitalWrite(BUZZER_PIN, HIGH); // 将蜂鸣器引脚设为高电平，使蜂鸣器停止响
        digitalWrite(LED_BUILTIN, HIGH);
        delay(500); // 等待500毫秒
    }
    Serial.println(PowerSwitch);
}
