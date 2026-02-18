#ifndef _UDP_NTP_H
#define _UDP_NTP_H

void sendUdpData(uint8_t udpChannel,IPAddress host,uint16_t hostPort,char* data);
int getUDPdate(uint32_t* hms,uint32_t* amj,byte* js);
void sendNTPpacket(char* address);



#endif // _UDP_NTP_H