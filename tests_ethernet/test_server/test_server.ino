
#include <SPI.h>      //bibliothèqe SPI pour W5100
#include <Ethernet.h> //bibliothèque W5x00 Ethernet
#include <EthernetUdp.h>

extern "C" {
 #include "utility/w5100.h"
}

#define PORTSERVER 1786
#define NOMSERV    "test"

  byte        mac[]={0x90,0xA2,0xDA,0x0F,0xDF,0xAE};   // W5X00 board  mac addr
  IPAddress   localIp={192,168,0,36};                  // W5X00 board fixed IP

  EthernetClient cli;  

  EthernetServer serv(PORTSERVER); 

  IPAddress remote_IP={0,0,0,0};
  byte      remote_MAC[6]={0,0,0,0,0,0};
  uint16_t  remote_Port=0;                   

void setup() {                              // ====================================

  Serial.begin (115200);delay(1000);
  Serial.print("+");delay(100);

  //pinMode(PINLED,OUTPUT);
  

  Serial.print("NOMSERV=");Serial.print(NOMSERV);Serial.print(" PORTSERVER=");Serial.println(PORTSERVER);
  
  if(Ethernet.begin(mac) == 0)
    {Serial.print("Failed with DHCP... forcing Ip ");Serial.println(localIp);
    Ethernet.begin (mac, localIp); //initialisation de la communication Ethernet
    }
    Serial.print("localIP=");Serial.println(Ethernet.localIP());


  serv.begin();Serial.println("serv.begin ");   // serveur périphériques

  delay(100);

  Serial.println("fin setup");
}

/*=================== fin setup ============================ */

void page(EthernetClient* cli)
{
  cli->println("HTTP/1.1 200 OK");
  //cli->println("Location: http://82.64.32.56:1789/");
  //cli->println("Cache-Control: private");
  cli->println("CONTENT-Type: text/html; charset=UTF-8");
  cli->println("Connection: close\n");
  cli->println("<!DOCTYPE HTML ><html>");
  cli->print("<body>");
  cli->print("coucou");
  cli->print("</body></html>");
}

void purge(EthernetClient* cli)
{
  while(cli->available()){char a=cli->read();Serial.print(a);}
  Serial.println();
}

void getremote_IP(EthernetClient* cli,IPAddress* remote_IP,byte* remote_MAC)
{
    byte rip[4];
    W5100.readSnDHAR(cli->getSocketNumber(), remote_MAC);
    W5100.readSnDIPR(cli->getSocketNumber(), rip);
    Serial.println("rip=");
    //*remote_IP=rip[0]+rip[1]*256+rip[2]*65536+rip[3]*16777216;
    //uint32_t ripx=(uint32_t)rip;
    memcpy((char*)remote_IP+4,rip,4);
    for(int i=0;i<8;i++){Serial.print(*((char*)&localIp+i),HEX);Serial.print(" ");}
    Serial.println();
    for(int i=0;i<8;i++){Serial.print(*((char*)remote_IP+i),HEX);Serial.print(" ");}
    Serial.println();
    for(int i=0;i<8;i++){Serial.print(*((char*)rip+i),HEX);Serial.print(" ");}
    Serial.println();
    for(int i=0;i<4;i++){
      Serial.print(rip[i]);Serial.print(" ");}
    Serial.println();
    
}

/* ======================================= loop ===================================== */

void loop()                         
{

     if(cli=serv.available())      // attente d'un client
     {
        getremote_IP(&cli,&remote_IP,remote_MAC);      
        
        if (cli.connected()){
          Serial.print(remote_IP);Serial.println(" connecté");
          
          purge(&cli);
          page(&cli);
          purge(&cli);
          
          cli.stop();
        }
     }     
}
