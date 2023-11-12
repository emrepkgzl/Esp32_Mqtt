#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_BMP085.h>
#include <DS323x.h>
#include <Preferences.h>

DS323x rtc;         // Kütüphanemiz başlatılıyor
Adafruit_BMP085 bmp;
Preferences pref;

// Update these with values suitable for your network.

const char* ssid = "YourSsid";
const char* password = "YourPassword";
const char* mqtt_server = "YourMqttServer";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
float value;
float oldValue = 0;
int new_values[3];
int old_values[3] = {0, 0, 0};

int ssetTurnOnHour;
int ssetTurnOnMinute;
int ssetTurnOffHour;
int ssetTurnOffMinute;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

int aaa;

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  //12.11.2023.10.33.45

  if((payload[0] - 48) == 1)
  {
    int ssetDay = (payload[2] - 48) * 10 + (payload[3] - 48);
    int ssetMonth = (payload[5] - 48) * 10 + (payload[6] - 48);
    int ssetYear = (payload[8] - 48) * 1000 + (payload[9] - 48) * 100 + (payload[10] - 48) * 10 + (payload[11] - 48);
    int ssetHour = (payload[13] - 48) * 10 + (payload[14] - 48);
    int ssetMinute = (payload[16] - 48) * 10 + (payload[17] - 48);
    int ssetSecond = (payload[19] - 48) * 10 + (payload[20] - 48);
  
    rtc.now(DateTime(ssetYear, ssetMonth, ssetYear, ssetHour, ssetMinute, ssetSecond));//DONT FORGET TO COMMENT AFTER UPLOAD ONCE
    //rtc.now(DateTime(2023, 12, 11, 1, 11, 15));//DONT FORGET TO COMMENT AFTER UPLOAD ONCE

    /*Serial.print("Hour: ");
    Serial.println(ssetHour);
    Serial.print("Minute: ");
    Serial.println(ssetMinute);
    Serial.print("Second: ");
    Serial.println(ssetSecond);*/
  }
  else
  {
    ssetTurnOnHour = (payload[2] - 48) * 10 + (payload[3] - 48);
    ssetTurnOnMinute = (payload[5] - 48) * 10 + (payload[6] - 48);

    ssetTurnOffHour = (payload[8] - 48) * 10 + (payload[9] - 48);
    ssetTurnOffMinute = (payload[11] - 48) * 10 + (payload[12] - 48);

    pref.putInt("TURN_ON_HOUR", ssetTurnOnHour);
    pref.putInt("TURN_ON_MIN", ssetTurnOnMinute);
    pref.putInt("TURN_OFF_HOUR", ssetTurnOffHour);
    pref.putInt("TURN_OFF_MIN", ssetTurnOffMinute);

    snprintf (msg, MSG_BUFFER_SIZE, "TURN_ON_HOUR is :%d", ssetTurnOnHour);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("YOUR_TOPIC/TURN_ON_HOUR", msg);

    delay(2000);

    snprintf (msg, MSG_BUFFER_SIZE, "TURN_ON_MIN is :%d", ssetTurnOnMinute);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("YOUR_TOPIC/TURN_ON_MIN", msg);

    delay(2000);

    snprintf (msg, MSG_BUFFER_SIZE, "TURN_OFF_HOUR is :%d", ssetTurnOffHour);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("YOUR_TOPIC/TURN_OFF_HOUR", msg);

    delay(2000);

    snprintf (msg, MSG_BUFFER_SIZE, "TURN_OFF_MIN is :%d", ssetTurnOffMinute);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("YOUR_TOPIC/TURN_OFF_MIN", msg);


  Serial.print("TURN_ON_HOUR: ");
  Serial.println(ssetTurnOnHour);
  Serial.print("TURN_ON_MIN: ");
  Serial.println(ssetTurnOnMinute);
  Serial.print("TURN_OFF_HOUR: ");
  Serial.println(ssetTurnOffHour);
  Serial.print("TURN_OFF_MIN: ");
  Serial.println(ssetTurnOffMinute);
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("YOUR_TOPIC/TEMP", "MQTT Server is Connected");
      // ... and resubscribe
      client.subscribe("YOUR_TOPIC/SETTINGS");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  pref.begin("RoomTempSystem", false);

  ssetTurnOnHour = pref.getInt("TURN_ON_HOUR", 6);
  ssetTurnOnMinute = pref.getInt("TURN_ON_MIN", 20);
  ssetTurnOffHour = pref.getInt("TURN_OFF_HOUR", 7);
  ssetTurnOffMinute = pref.getInt("TURN_OFF_MIN", 30);

  Serial.print("TURN_ON_HOUR: ");
  Serial.println(ssetTurnOnHour);
  Serial.print("TURN_ON_MIN: ");
  Serial.println(ssetTurnOnMinute);
  Serial.print("TURN_OFF_HOUR: ");
  Serial.println(ssetTurnOffHour);
  Serial.print("TURN_OFF_MIN: ");
  Serial.println(ssetTurnOffMinute);

  if(!bmp.begin()) 
  {
    Serial.println("Could not find a valid BMP085 sensor, check wiring!");
    while (1) {}
  }

  Wire.begin();       // Wire kütüphanesini başlatıyoruz
  delay(1000);        // Bağlantı kurulana kadar bekleyelim

  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);

  rtc.attach(Wire);   // Kütüphaneyi I2C'ye bağlayalım
}

int counter = 0;
bool counter2 = false;

void loop() {

  if (!client.connected()) {
    if(counter < 3)
    {
      counter++;
      reconnect();
    }
    
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 2000) 
  {
    if(!counter2)
    {
      counter2 = true;;
      
      snprintf (msg, MSG_BUFFER_SIZE, "TURN_ON_HOUR is :%d", ssetTurnOnHour);
      Serial.print("Publish message: ");
      Serial.println(msg);
      client.publish("YOUR_TOPIC/TURN_ON_HOUR", msg);
    
      delay(2000);
    
      snprintf (msg, MSG_BUFFER_SIZE, "TURN_ON_MIN is :%d", ssetTurnOnMinute);
      Serial.print("Publish message: ");
      Serial.println(msg);
      client.publish("YOUR_TOPIC/TURN_ON_MIN", msg);
    
      delay(2000);
    
      snprintf (msg, MSG_BUFFER_SIZE, "TURN_OFF_HOUR is :%d", ssetTurnOffHour);
      Serial.print("Publish message: ");
      Serial.println(msg);
      client.publish("YOUR_TOPIC/TURN_OFF_HOUR", msg);
    
      delay(2000);
    
      snprintf (msg, MSG_BUFFER_SIZE, "TURN_ON_MIN is :%d", ssetTurnOffMinute);
      Serial.print("Publish message: ");
      Serial.println(msg);
      client.publish("YOUR_TOPIC/TURN_OFF_MIN", msg);
      
    }
    
    DateTime noww = rtc.now(); 

    new_values[0] = noww.hour();
    new_values[1] = noww.minute();
    new_values[2] = noww.second();

    if((new_values[0] != old_values[0]) || (new_values[1] != old_values[1]))
    {
      old_values[0] = new_values[0];
      snprintf (msg, MSG_BUFFER_SIZE, "Hour is :%d", new_values[0]);
      Serial.print("Publish message: ");
      Serial.println(msg);
      client.publish("YOUR_TOPIC/HOUR", msg);

      delay(2000);

      old_values[1] = new_values[1];
      snprintf (msg, MSG_BUFFER_SIZE, "Minute is :%d", new_values[1]);
      Serial.print("Publish message: ");
      Serial.println(msg);
      client.publish("YOUR_TOPIC/MIN", msg);
    }

    lastMsg = now;
    value = bmp.readTemperature();
    if(value != oldValue)
    {
      oldValue = value;
      snprintf (msg, MSG_BUFFER_SIZE, "Temperature is :%.2f", value);
      Serial.print("Publish message: ");
      Serial.println(msg);
      client.publish("YOUR_TOPIC/TEMP", msg);
    }

    if((noww.hour() == ssetTurnOffHour) && (noww.minute() >= ssetTurnOffMinute))
    {
      digitalWrite(2, LOW);
    }
    else if((noww.hour() == ssetTurnOnHour) && (noww.minute() >= ssetTurnOnMinute))
    {
      digitalWrite(2, HIGH);
    }
  }
}
