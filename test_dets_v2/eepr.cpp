
#include <EEPROM.h>
#include "eepr.h"


Eepr::Eepr()
{
}

uint32_t Eepr::calcCrc32(byte* data,uint16_t len)
{
  uint32_t crc32=0,j,k;
  uint32_t reg;
  uint16_t i;

  for(i=0;i<len;i+=4){
    memcpy(&reg,data+i,4);
    reg=(reg>>1)<<1;
    if(memcmp(&reg,data+i,4)!=0){Serial.print("****bug****");while(1){};}
    for(j=0;j<32;j++){
        k=(crc32^reg)&0x00000001;
        crc32=crc32>>1;
        if(k==1){crc32=crc32^0x82608EDB;} //0x04C11DB7 rigth rotated polynom
        reg=reg>>1;
    }
  }
  return crc32;
}

bool Eepr::checkCrc32(byte* data,uint16_t len,uint32_t crc32)
{
    return calcCrc32(data,len)==crc32;
}

void Eepr::read(byte* data,uint16_t length,uint16_t addr)
{
    length&=0x00ff; // maxi 256
    for(uint8_t i=0;i<length;i++){
        data[i] = EEPROM.read(addr+i);}
}

void Eepr::write(byte* data,uint16_t length,uint16_t addr)
{
    length&=0x00ff; // maxi 256
    for(uint8_t i=0;i<length;i++){
        EEPROM.update(addr+i,data[i]);}
}

void Eepr::store(byte* data,uint16_t length)
{
    byte len[EEPRLENGTHLENGTH];
    memcpy(data+EEPRCRCLENGTH,&length,EEPRLENGTHLENGTH);
    uint32_t crc=calcCrc32(data+EEPRCRCLENGTH,length-EEPRCRCLENGTH);
    memcpy(data,&crc,4);

    write(data,length,0);
}

bool Eepr::load(byte* data,uint16_t length)
{
    if(length<EEPRHEADERLENGTH) {return false;}
    read(data,EEPRHEADERLENGTH,0);                              // lit header
    uint16_t usefullLength;
    memcpy(&usefullLength,data+EEPRCRCLENGTH,EEPRLENGTHLENGTH);   // recup longueur-crc
    if(length<usefullLength+EEPRCRCLENGTH) {return false;}
    read(data+EEPRCRCLENGTH,usefullLength,EEPRCRCLENGTH);       // lit la suite du header
    uint32_t crc32;
    memcpy(&crc32,data,EEPRCRCLENGTH);
    return checkCrc32(data+EEPRCRCLENGTH,usefullLength,crc32);
}
