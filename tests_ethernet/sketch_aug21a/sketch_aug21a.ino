#include <Ethernet.h>
#include <SPI.h>

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
byte ip[] = { 192,168,0,30 };
//char server[] = {"www.google.com"};
//byte server[] = {216,58,213,164}; // Google
//int port = 80; 
byte server[] = {82,64,32,56}; 
int port = 1790;


EthernetClient client;

void setup()
{
  Serial.begin(115200);
  delay(1000);
  
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    Ethernet.begin(mac, ip);
  }

  Serial.println("connecting...");

  if (client.connect(server, port)) {
    Serial.println("connected");
    client.println("GET /search?q=arduino HTTP/1.0");
    client.println();
  } else {
    Serial.println("connection failed");
  }
}

void loop()
{
  if (client.available()) {
    char c = client.read();
    Serial.print(c);
  }

  if (!client.connected()) {
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();
    while(1){}
  }
}
