
#include <WiFiUdp.h>

char timeServer[] = "ntp-p1.obspm.fr\0";  //"ntp-p1.obspm.fr\0";      // 145.238.203.14  NTP.MIDWAY.OVH ntp.unice.fr ntp.uvsq.fr ntp-p1.obspm.fr
const int NTP_PACKET_SIZE = 48;           // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE];      // buffer to hold incoming and outgoing packets

WiFiUDP Udp;
#define LOCALUDPPORT 8880
#define NTPHOSTPORT 123

void convertNTP(unsigned long *dateUnix,int *year,int *month,int *day,byte *js,int *hour,int *minute,int *second)
{
    int feb29;
    unsigned long secDay=24*3600L;
    unsigned long secYear=365*secDay;
    unsigned long m28=secDay*28L,m30=m28+secDay+secDay,m31=m30+secDay;
    unsigned long monthSec[]={0,m31,monthSec[1]+m28,monthSec[2]+m31,monthSec[3]+m30,monthSec[4]+m31,monthSec[5]+m30,monthSec[6]+m31,monthSec[7]+m31,monthSec[8]+m30,monthSec[9]+m31,monthSec[10]+m30,monthSec[11]+m31};
    unsigned long sec1quart=0;
    unsigned long secLastQuart=0;
    unsigned long secLastYear=0;
    unsigned long secLastMonth=0;
    unsigned long secLastDay=0;

    if(*dateUnix<2*secYear){*year=(int)(*dateUnix/secYear)+1970;feb29=1;}
    else
      {sec1quart = 4*secYear+secDay;
      secLastQuart = (*dateUnix-(2*secYear))%sec1quart;                           // sec dans quartile courant (premier 1972 bisextile)
//      Serial.print(secLastQuart);Serial.print(" ");
      *year = (int)((((*dateUnix-(2*secYear))/sec1quart)*4)+1972L);               // 1ère année quartile courant
//      Serial.print(*year);Serial.print(" ");
      if(secLastQuart<=(monthSec[2]+secDay)){feb29=0;monthSec[2]+=secDay;}        // date dans les 29 premiers jours de la 1ère année du dernier quartile
      else{*year += ((secLastQuart-secDay)/secYear);feb29=1;}                     // sinon ...
      }
    secLastYear=((secLastQuart-feb29*secDay)%secYear);
    for(int i=0;i<12;i++){
//      Serial.print("monthSec[i+1]=");Serial.print(i+1);Serial.print("/");Serial.println(monthSec[i+1]);
      if(monthSec[i+1]>secLastYear){*month=i+1;i=12;}
    }
    secLastMonth=secLastYear-monthSec[*month-1];
    *day  = (int)(secLastMonth/secDay)+1;
    secLastDay = secLastMonth-((*day-1)*secDay);
//    Serial.print("secLastDay=");Serial.print(secLastDay);Serial.print("/");Serial.print(*day);Serial.print(" secLastMonth=");Serial.print(secLastMonth);Serial.print("/");Serial.print(*month);Serial.print(" secLastYear=");Serial.print(secLastYear);Serial.print("/");Serial.println(*year);
    *hour = (int)(secLastDay/3600L);
    *minute = (int)(secLastDay-*hour*3600L)/60;
    *second = (int)(secLastDay-*hour*3600L-*minute*60);

    int k=0;if(*month>=3){k=2;}
    *js=( (int)((23*(*month))/9) + *day + 4 + *year + (int)((*year-1)/4) - (int)((*year-1)/100) + (int)((*year-1)/400) - k )%7;

//char buf[4]={0};strncat(buf,days+(*js)*3,3);
//Serial.print(*js);Serial.print(" ");Serial.print(buf);Serial.print(" ");
//Serial.print(*year);Serial.print("/");Serial.print(*month);Serial.print("/");Serial.print(*day);Serial.print(" ");
//Serial.print(*hour);Serial.print(":");Serial.print(*minute);Serial.print(":");Serial.println(*second);
}



void sendNTPpacket(char* address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, NTPHOSTPORT); //NTP requests are to port 123
//IPAddress loc(82,64,32,56);
//Udp.beginPacket(loc,8888);

  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}

int getUDPdate(uint32_t* hms,uint32_t* amj,byte* js)
{
  int returnStatus=0;
  int year=0,month=0,day=0,hour=0,minute=0,second=0;

  Udp.begin(LOCALUDPPORT);

  sendNTPpacket(timeServer); 
  
  delay(1000);                                // wait to see if a reply is available
  if (Udp.parsePacket()) {                    // packet received
    Udp.read(packetBuffer, NTP_PACKET_SIZE);  // get it                 // sec1900- 2208988800UL;
    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    unsigned long secsSince1970 = secsSince1900 -2208988800UL;          //sec1970=1456790399UL;  // 29/02/2016

//Serial.print("packetBuffer 40-43 : ");Serial.print(packetBuffer[40]);Serial.print(" ");Serial.print(packetBuffer[41]);Serial.print(" ");Serial.print(packetBuffer[42]);Serial.print(" ");Serial.println(packetBuffer[43]);
//Serial.print(" sin 1900/1970 : ");Serial.print(secsSince1900,10);Serial.print("/");Serial.println(secsSince1970,10);
    convertNTP(&secsSince1970,&year,&month,&day,js,&hour,&minute,&second);
    *amj=year*10000L+month*100+day;*hms=hour*10000L+minute*100+second;
//Serial.print(*amj);Serial.print(" ");Serial.println(*hms);
    returnStatus=1;
  }
  Udp.stop();
  return returnStatus;
}