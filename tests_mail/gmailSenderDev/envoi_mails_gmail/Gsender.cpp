#include <ESP8266WiFi.h>
#include "Gsender.h"
Gsender* Gsender::_instance = 0;
Gsender::Gsender(){}
Gsender* Gsender::Instance()
{
    if (_instance == 0) 
        _instance = new Gsender;
    return _instance;
}

Gsender* Gsender::Subject(const char* subject)
{
  delete [] _subject;
  _subject = new char[strlen(subject)+1];
  strcpy(_subject, subject);
  return _instance;
}
Gsender* Gsender::Subject(const String &subject)
{
  return Subject(subject.c_str());
}

bool Gsender::AwaitSMTPResponse(WiFiClientSecure &client, const String &resp, uint16_t timeOut)
{
  uint32_t ts = millis();
  while (!client.available())
  {
    if(millis() > (ts + timeOut)) {
      _error = "SMTP Response TIMEOUT!";
      return false;
    }
  }
  _serverResponce = client.readStringUntil('\n');
#if defined(GS_SERIAL_LOG_1) || defined(GS_SERIAL_LOG_2) 
  Serial.println(_serverResponce);
#endif
  if (resp && _serverResponce.indexOf(resp) == -1) return false;
  return true;
}

String Gsender::getLastResponce()
{
  return _serverResponce;
}

const char* Gsender::getError()
{
  return _error;
}

bool Gsender::Send(const String &to, const String &message)
{
  WiFiClientSecure client;
#if defined(GS_SERIAL_LOG_2)
  Serial.print("Connecting to :");
  Serial.println(SMTP_SERVER);  
#endif
  if(!client.connect(SMTP_SERVER, SMTP_PORT)) {
    _error = "Could not connect to mail server";
    return false;
  }
  if(!AwaitSMTPResponse(client, "220")) {
    _error = "Connection Error";
    return false;
  }

#if defined(GS_SERIAL_LOG_2)
  Serial.println("HELO friend:");
#endif
  client.println("HELO friend");
  if(!AwaitSMTPResponse(client, "250")){
    _error = "identification error";
    return false;
  }

#if defined(GS_SERIAL_LOG_2)
  Serial.println("AUTH LOGIN:");
#endif
  client.println("AUTH LOGIN");
  AwaitSMTPResponse(client);

#if defined(GS_SERIAL_LOG_2)
  Serial.println("EMAILBASE64_LOGIN:");
#endif
  client.println(EMAILBASE64_LOGIN);
  AwaitSMTPResponse(client);

#if defined(GS_SERIAL_LOG_2)
  Serial.println("EMAILBASE64_PASSWORD:");
#endif
  client.println(EMAILBASE64_PASSWORD);
  if (!AwaitSMTPResponse(client, "235")) {
    _error = "SMTP AUTH error";
    return false;
  }
  
  String mailFrom = "MAIL FROM: <" + String(FROM) + '>';
#if defined(GS_SERIAL_LOG_2)
  Serial.println(mailFrom);
#endif
  client.println(mailFrom);
  AwaitSMTPResponse(client);

  String rcpt = "RCPT TO: <" + to + '>';
#if defined(GS_SERIAL_LOG_2)
  Serial.println(rcpt);
#endif
  client.println(rcpt);
  AwaitSMTPResponse(client);

#if defined(GS_SERIAL_LOG_2)
  Serial.println("DATA:");
#endif
  client.println("DATA");
  if(!AwaitSMTPResponse(client, "354")) {
    _error = "SMTP DATA error";
    return false;
  }
  
  client.println("From: <" + String(FROM) + '>');
  client.println("To: <" + to + '>');
  
  client.print("Subject: ");
  client.println(_subject);
  
  client.println("Mime-Version: 1.0");
  client.println("Content-Type: text/html; charset=\"UTF-8\"");
  client.println("Content-Transfer-Encoding: 7bit");
  client.println();
  String body = "<!DOCTYPE html><html lang=\"en\">" + message + "</html>";
  client.println(body);
  client.println(".");
  if (!AwaitSMTPResponse(client, "250")) {
    _error = "Sending message error";
    return false;
  }
  client.println("QUIT");
  if (!AwaitSMTPResponse(client, "221")) {
    _error = "SMTP QUIT error";
    return false;
  }
  return true;
}


uint8_t WiFiConnect(const char* ssid, const char* password)
{
    static uint16_t attempt = 0;
    Serial.print("Connecting to ");
    
    WiFi.begin(ssid, password);
    Serial.println(ssid);

    uint8_t i = 0;
    while(WiFi.status()!= WL_CONNECTED && i++ < 50)
    {
        delay(200);
        Serial.print(".");
    }
    ++attempt;
    Serial.println("");
    if(i == 51) {
        Serial.print("Connection: TIMEOUT on attempt: ");
        Serial.println(attempt);
        if(attempt % 2 == 0)
            Serial.println("Check if access point available or SSID and Password\r\n");
        return false;
    }
    Serial.println("Connection: ESTABLISHED");
    Serial.print("Got IP address: ");
    Serial.println(WiFi.localIP());
    return true;
}

uint8_t Awaits(const char* ssid, const char* password)
{
    uint16_t reconnect_interval = 10000;             // If not connected wait time to try again
    uint8_t cx_state=0;
    uint32_t ts = millis();
    
    while(!cx_state){
        delay(50);
        if(millis() > (ts + reconnect_interval) && !cx_state){
            cx_state = WiFiConnect(ssid,password);
            ts = millis();
        }
    }
    return cx_state;
}
