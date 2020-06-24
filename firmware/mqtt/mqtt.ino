#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

/**************************** Variaveis globais *******************************/

#define DEVICE_ID     "dev-02"
#define DEVICE_TOKEN  "....."

const char* mqtt_server = "mqtt.proiot.network";

byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
IPAddress ip(192, 168, 0, 100);
IPAddress server(192, 168, 0, 1);


/************************* Instanciação dos objetos  **************************/

EthernetClient ethClient;
PubSubClient client(ethClient);

/********************************** Sketch ************************************/

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    String clientId = "proiot-dev-";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str(), DEVICE_TOKEN, "")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");

      delay(5000);
    }
  }
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, 0);

  Serial.begin(57600);

  client.setServer(server, 1883);

  Ethernet.begin(mac, ip);

  delay(1500);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  Serial.print("Publish message: ");
  Serial.println("ok");

  digitalWrite(LED_BUILTIN, 1);
  client.publish("device/5ef20c1f47f0e40019a54876", "ok");
  digitalWrite(LED_BUILTIN, 0);

  delay(2000);
}