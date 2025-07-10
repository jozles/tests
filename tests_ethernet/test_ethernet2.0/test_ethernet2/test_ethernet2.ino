#include <Ethernet.h>
//#include <EthernetUdp.h>
#include <SPI.h>

#define LED 13

EthernetClient cli;


//  byte        host[]={82, 64, 32, 56};
  byte        host[]= {192,168,0,36};
  int         port  = 1786;

  byte        mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0x00};   // fake Mac for wiznet board
  byte        localIp[] = {192,168,0,199};                

#define MAXLEN 500  
  char        rxMessage[MAXLEN];
  uint16_t    len;
  int         txStatus,rxStatus;
  uint16_t    cnt=0;


void serialPrintIp(uint8_t* ip)
{
  for(int i=0;i<4;i++){Serial.print(ip[i]);if(i<3){Serial.print(".");}}
  //Serial.print(" ");
}  

int messToServer(EthernetClient* cli,byte* host,int port,char* data)    // connecte au serveur et transfère la data
{
  int             cxStatus=0;
  uint8_t         repeat=0;

  Serial.print("connecting ");    
  for(int i=0;i<4;i++){Serial.print((uint8_t)host[i]);Serial.print(" ");}
  Serial.print(":");Serial.print(port);
  Serial.print("...");
  
  while(!cxStatus && repeat<4){

    repeat++;

    cxStatus=cli->connect(host,port);
    cxStatus=cli->connected();
    Serial.print(repeat);Serial.print("/");
    Serial.print(cxStatus);
    if(!cxStatus){
        switch(cxStatus){
            case -1:Serial.print(" time out ");break;
            case -2:Serial.print(" invalid server ");break;
            case -3:Serial.print(" truncated ");break;
            case -4:Serial.print(" invalid response ");break;
            default:Serial.print(" unknown reason ");break;
        }
    }
    else {
      Serial.println(" ok");
      cli->print(data);
      cli->print("\r\n HTTP/1.1\r\n Connection:close\r\n\r\n");
      return 1;
    }
    delay(100);
  }
  Serial.println(" failed");return 0;
}

int getHttpData(EthernetClient* cli,char* data,uint16_t* len)
{
  #define TIMEOUT 2000
  
  int pt=0;
  char inch;
  unsigned long timerTo=millis();

  if(*len==0){return -1;}

  if(cli->connected()!=0){
      while(millis()<(timerTo+TIMEOUT)){
        if(cli->available()>0){
          timerTo=millis();
          inch=cli->read();Serial.print(inch);
          
            if(pt<*len-1){data[pt]=inch;pt++;}
        }
      }  
      data[pt]='\0';
      *len=pt;
      return 1;               // data ok
  }
  return -2;                  // not connected
}


void setup() {

  Serial.begin(115200);delay(1000);
  Serial.println("start");

  pinMode(LED, OUTPUT);
  for(int i=0;i<3;i++){delay(100);digitalWrite(LED,HIGH);delay(10);digitalWrite(LED,LOW);}
      
  unsigned long t_beg=millis();
  if(Ethernet.begin((uint8_t*)mac) == 0){
    Serial.print("Failed with DHCP... forcing Ip ");
    serialPrintIp(localIp);Serial.println();
    Ethernet.begin((uint8_t*)mac, localIp);
    }
  
  Serial.print("localIP=");Serial.println(Ethernet.localIP());
  delay(1000);
}

void loop() {

    cnt++;
    Serial.print(cnt);Serial.print(" ");
  
    txStatus=messToServer(&cli,host,port,"GET/");
    Serial.println(txStatus);
  
    if(txStatus){
      len=MAXLEN;
      rxStatus=getHttpData(&cli,rxMessage,&len);
      Serial.print(rxStatus);Serial.print(" len=");Serial.println(len);
    }
    cli.stop();
    delay(1000);
 
}
