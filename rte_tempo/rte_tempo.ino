/* **** Routine d'extraction des couleurs Tempo du site de RTE ****
 * ******************* 2023-2024 Roland MATHIS ********************

   Les 2 requêtes HTTP utilisées dans ce programme ont été créées
   par J-M-L du forum francophone Arduino (un très GROS merci !)
   ici : https://forum.arduino.cc/t/api-rte-ecowatt/1017281
   D'où la notice de copyright ci-dessous :

 ============================================
  KEEP THIS INFORMATION IF YOU USE THIS CODE

  This "API RTE" demo code for ESP 32 is placed under the MIT license
  Copyright (c) 2022 J-M-L For the Arduino Forum

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
  ===============================================
*/

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <esp_sntp.h>      // Permet le contrôle de la synchro avec un serveur NTP
#include <HTTPClient.h>
#include <ArduinoJson.h>

// ******* Valeurs nécessaires à l'accès à l'API "Tempo Like Supply Contract" de RTE *******
    // ID Client et ID Secret en base 64, créées sur le site de RTE avec le bouton "Copier en base 64"
    #define identificationRTE   "YTI1Y****************************************************************************************ZjMg=="

    const char * idRTE = "Basic " identificationRTE;

    // Certificat racine (format PEM) de https://digital.iservices.rte-france.com
    const char* root_ca = \
    "-----BEGIN CERTIFICATE-----\n" \
    "MIIDXzCCAkegAwIBAgILBAAAAAABIVhTCKIwDQYJKoZIhvcNAQELBQAwTDEgMB4G\n" \
    "A1UECxMXR2xvYmFsU2lnbiBSb290IENBIC0gUjMxEzARBgNVBAoTCkdsb2JhbFNp\n" \
    "Z24xEzARBgNVBAMTCkdsb2JhbFNpZ24wHhcNMDkwMzE4MTAwMDAwWhcNMjkwMzE4\n" \
    "MTAwMDAwWjBMMSAwHgYDVQQLExdHbG9iYWxTaWduIFJvb3QgQ0EgLSBSMzETMBEG\n" \
    "A1UEChMKR2xvYmFsU2lnbjETMBEGA1UEAxMKR2xvYmFsU2lnbjCCASIwDQYJKoZI\n" \
    "hvcNAQEBBQADggEPADCCAQoCggEBAMwldpB5BngiFvXAg7aEyiie/QV2EcWtiHL8\n" \
    "RgJDx7KKnQRfJMsuS+FggkbhUqsMgUdwbN1k0ev1LKMPgj0MK66X17YUhhB5uzsT\n" \
    "gHeMCOFJ0mpiLx9e+pZo34knlTifBtc+ycsmWQ1z3rDI6SYOgxXG71uL0gRgykmm\n" \
    "KPZpO/bLyCiR5Z2KYVc3rHQU3HTgOu5yLy6c+9C7v/U9AOEGM+iCK65TpjoWc4zd\n" \
    "QQ4gOsC0p6Hpsk+QLjJg6VfLuQSSaGjlOCZgdbKfd/+RFO+uIEn8rUAVSNECMWEZ\n" \
    "XriX7613t2Saer9fwRPvm2L7DWzgVGkWqQPabumDk3F2xmmFghcCAwEAAaNCMEAw\n" \
    "DgYDVR0PAQH/BAQDAgEGMA8GA1UdEwEB/wQFMAMBAf8wHQYDVR0OBBYEFI/wS3+o\n" \
    "LkUkrk1Q+mOai97i3Ru8MA0GCSqGSIb3DQEBCwUAA4IBAQBLQNvAUKr+yAzv95ZU\n" \
    "RUm7lgAJQayzE4aGKAczymvmdLm6AC2upArT9fHxD4q/c2dKg8dEe3jgr25sbwMp\n" \
    "jjM5RcOO5LlXbKr8EpbsU8Yt5CRsuZRj+9xTaGdWPoO4zzUhw8lo/s7awlOqzJCK\n" \
    "6fBdRoyV3XpYKBovHd7NADdBj+1EbddTKJd+82cEHhXXipa0095MJ6RMG3NzdvQX\n" \
    "mcIfeg7jLQitChws/zyrVQ4PkX4268NXSb7hLi18YIvDQVETI53O9zJrlAGomecs\n" \
    "Mx86OyXShkDOOyyGeMlhLxS67ttVb9+E7gUJTb0o2HLO02JQZR7rkpeDMdmztcpH\n" \
    "WD9f\n" \
    "-----END CERTIFICATE-----\n";

bool FirstCycleDone = false;                 // Mis à "true" après la première boucle de loop()
String JourJ, JourJ1, access_token2 ;

// ******** Variables pour gérer l'intervalle de lecture de l'API de RTE
long intervalle = 60000; //900000;                    //  = 15 minutes
unsigned long top_lect_API_RTE;

// ******** Valeurs en relation avec l'horloge internet et la gestion de la date et de l'heure
const char* ntpServer = "fr.pool.ntp.org";   // Adresse du serveur NTP
time_t moment;                               // utilisé pour stocker la date et l'heure au format unix
struct tm loc;                               // Structure qui contient les informations de l'heure et de la date
struct tm loc2;                              // Structure qui contient les informations de l'heure et de la date de dans 2 jours

void setup()
{
  Serial.begin(115200);
  delay(2000);
  Serial.println("+tempo_rte");

  connect_WiFi();
}

String errorDescription(int code, HTTPClient& http)
// Liste des codes d'erreurs spécifique à l'API RTE ou message général en clair
{
  switch (code) 
  {
    case 400: return "Erreur dans la requête";
    case 401: return "L'authentification a échouée";
    case 403: return "L’appelant n’est pas habilité à appeler la ressource";
    case 413: return "La taille de la réponse de la requête dépasse 7Mo";
    case 414: return "L’URI transmise par l’appelant dépasse 2048 caractères";
    case 429: return "Le nombre d’appel maximum dans un certain laps de temps est dépassé";
    case 509: return "L‘ensemble des requêtes des clients atteint la limite maximale";
    default: break;
  }
  return http.errorToString(code);
}

bool getRTEData()
// ****** Deux requêtes vers l'API de RTE, nécessite que le WiFi soit actif ******
{
  int codeReponseHTTP;
  const char* access_token;
  bool requeteOK = true;
  const char* oauthURI =   "https://digital.iservices.rte-france.com/token/oauth/";

  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WiFI non disponible. Requête impossible");
    return false;
  }
  WiFiClientSecure client;     // on passe en connexion sécurisée (par https...)
  HTTPClient http;
  client.setCACert(root_ca);   // permet la connexion sécurisée en vérifiant le certificat racine

// ************** Première des deux requêtes pour obtenir un token **************

    Serial.println("**************** Première requête (obtention du token) *****************");
    Serial.println();

  http.begin(client, oauthURI);

  // Headers nécessaires à la requête
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  http.addHeader("Authorization", idRTE);

  // Send HTTP POST request     
  codeReponseHTTP = http.POST(nullptr,0);
  Serial.print("code erreur http :");Serial.println(codeReponseHTTP);     
  if (codeReponseHTTP == HTTP_CODE_OK)
  {
    String oauthPayload = http.getString();
    Serial.println("------------ Contenu renvoyé par la requête 1 : ------------");
    Serial.println(oauthPayload);
    Serial.println("------------------------------------------------------------\n");
    StaticJsonDocument<192> doc;
    DeserializationError error = deserializeJson(doc, oauthPayload);
    if (error)     // cas où il y a un problème dans le contenu renvoyé par la requête
    {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.c_str());
      access_token = "";
      requeteOK = false;
    }
    else           // cas où le contenu renvoyé par la requête est valide et exploitable
    {
      access_token = doc["access_token"];
      Serial.println("--------- Acces token extrait des données Json : -----------");
      Serial.print("Access token : ");
      Serial.println(access_token);
      Serial.println("------------------------------------------------------------\n");
      access_token2 = access_token;   // Obligation de mettre "access_token" dans une autre variable , sinon elle est perdue lors de la 2ème requête
    }
  } 
  else 
  {
    Serial.print("erreur HTTP POST: ");
    Serial.println(errorDescription(codeReponseHTTP, http));
    requeteOK = false;
  }
  http.end();
  if (!requeteOK) return false;

  // ***** Deuxième des deux requêtes pour obtenir la couleur des jours, nécessitant le token *****

  // REMARQUES : l'adresse pour la requête est sous la forme :
  // https://digital.iservices.rte-france.com/open_api/tempo_like_supply_contract/v1/tempo_like_calendars?start_date=2015-06-08T00:00:00%2B02:00&end_date=2015-06-11T00:00:00%2B02:00
  // avec (dans notre cas) "start_date" la date du jour et "end_date" la date du jour + 2
  // Après les "%2B" (signe +) on a le décalage horaire par rapport au temps UTC. On doit obligatoirement avoir "02:00" en heures d'été et "01:00" en heure d'hiver
  // Les heures de début et de fin peuvent rester à "T00:00:00" pour la requête mais doivent être présentes !
  // Pour les mois et les jours, les "0" au début peuvent être omis dans le cas de nombres inférieurs à 10

  Serial.println("************* Deuxième requête (obtention des couleurs Tempo) **************");
  Serial.println();

  String requete = "https://digital.iservices.rte-france.com/open_api/tempo_like_supply_contract/v1/tempo_like_calendars?start_date=";
  requete += String(loc.tm_year+1900)+"-"+String(loc.tm_mon+1)+"-"+String(loc.tm_mday)+"T00:00:00%2B0"+String(loc.tm_isdst+1)+":00&end_date="+String(loc2.tm_year+1900)+"-"+String(loc2.tm_mon+1)+"-"+String(loc2.tm_mday)+"T00:00:00%2B0"+String(loc2.tm_isdst+1)+":00";
  // Remarque : "loc.tm_isdst" est à 0 en heure d'hiver et à 1 en heure d'été   
  
  Serial.println("---------------- Adresse pour la requête 2 : ---------------");
  Serial.println(requete);
  Serial.println("------------------------------------------------------------\n");
  http.begin(client, requete);

    String signalsAutorization = "Bearer "+ String(access_token2);
      Serial.println("--------------- String signalsAutorization : ---------------");
      Serial.println(signalsAutorization);
      Serial.println("------------------------------------------------------------\n");
    
    // Headers nécessaires à la requête
    http.addHeader("Authorization", signalsAutorization.c_str());
    http.addHeader("Accept", "application/xml");
    // Mettre la ligne précédente en remarque pour avoir le résultat en json plutôt qu'en xml

    // On envoie la requête HTTP GET
    codeReponseHTTP = http.GET();
    if (codeReponseHTTP == HTTP_CODE_OK)
    {
      String recup = http.getString();              // "recup" est une chaîne de caractères au format xml
      Serial.println("------------ Contenu renvoyé par la requête 2 : ------------");
      Serial.println(recup);
      Serial.println("------------------------------------------------------------\n");

      // Récupération des couleurs
      int posi = recup.indexOf("<Couleur>",100);    // Recherche de la première occurence de la chaîne "<Couleur>"
                                                    // à partir du 100ème caractère de "recup"
      if (recup.length() > 200)                     // Si la couleur J+1 est connue le String "recup" fait plus de 200 caractères 
      {
        JourJ1 = (recup.substring(posi+9,posi+13)); // Récupération du substring des 4 caractères contenant couleur du lendemain
                                                    // peut être "BLEU", "BLAN" ou "ROUG"
        posi = recup.indexOf("<Couleur>",230);      // Recherche de la deuxième  occurence de la chaîne "<Couleur>"
                                                    // à partir du 230ème caractère de "recup"
        JourJ = (recup.substring(posi+9,posi+13));  // Récupération du substring des 4 caractères contenant la couleur du jour
                                                    // peut être "BLEU", "BLAN" ou "ROUG"
      }
      else                                          // cas où la couleur de J+1 n'est pas encore connue
      {
        JourJ = (recup.substring(posi+9,posi+13));  // Récupération du substring des 4 caractères contenant la couleur du jour
                                                    // peut être "BLEU", "BLAN" ou "ROUG"
        JourJ1 = "NON_DEFINI";
      }
      Serial.print("Couleur Tempo du jour : "); Serial.println(JourJ);
      Serial.print("Couleur Tempo de demain : "); Serial.println(JourJ1 + "\n");
    } 
    else
    {
      Serial.print("erreur HTTP GET: ");
      Serial.print(codeReponseHTTP);
      Serial.print(" => ");
      Serial.println(errorDescription(codeReponseHTTP, http));
      Serial.println();
      requeteOK = false;
    }
  http.end();
  return requeteOK;
}

void connect_WiFi()
{
  const char* ssid = "pinks";   //"devolo-5d3";
  const char* mdp = "cain ne dormant pas songeait au pied des monts";    //"JNCJTRONJMGZEEQ";
  const int TimeOut_connexion = 40;                 // Time-out connexion après 40 essais
  int timeOut_counter = 0;                          // Compteur de nombre d'essais de connexion au Wifi
  sntp_sync_status_t syncStatus;                    // Renvoie le statut de synchro au serveur SNTP, nécessite la librairie "esp_sntp.h"

  Serial.print("Connection au WiFi :");Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, mdp);

  while (WiFi.status() != WL_CONNECTED) 
    {
     delay(250);
     Serial.write('.');
     ++timeOut_counter;                             // redémarrage de l'ESP si pas de connexion pendant 10 sec
    if (timeOut_counter > TimeOut_connexion) ESP.restart();

    }
  Serial.print("\nConnecté au WiFi avec l'adresse IP ");
  Serial.println(WiFi.localIP());
  delay(1000);

  // Prise en compte automatique du décalage horaire et des dates de début et de fin d'heure d'été par rapport au temps UTC
  // Fonction intégrée à la librairie "time.h"
  configTzTime("CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00", ntpServer);

  // Vérification de la synchronisation effective de l'heure au démarrage  
  timeOut_counter = 0;
  Serial.println("\nSynchronisation de l'heure en cours ");
   while (syncStatus != SNTP_SYNC_STATUS_COMPLETED)
   {
    syncStatus = sntp_get_sync_status();    
    delay(250); 
     Serial.write('.');
    ++timeOut_counter;                              // redémarrage de l'ESP si pas de connexion pendant 10 sec
    if (timeOut_counter > TimeOut_connexion) ESP.restart();
   }
  Serial.println("\nSynchronisation de l'heure OK !\n");
  delay(1000);
}

void loop()
{
  // ****** Accès périodique à l'API de RTE ******
  if (millis() - top_lect_API_RTE >=  intervalle || FirstCycleDone == false)  // condition vraie tous les "intervalle" ou au premier loop()
    {
      top_lect_API_RTE=millis();
      // Définition de la date et de l'heure actuelles
      moment = time(NULL);            // Mise de la date et de l'heure au format unix dans la variable "moment" de type time_m
      loc = *(localtime(&moment));    // Mise dans la structure loc des éléments de date et de l'heure
      Serial.printf("Date et heure actuelles : %02d/%02d/%04d %02d:%02d:%02d\n", loc.tm_mday, loc.tm_mon+1, loc.tm_year+1900, loc.tm_hour, loc.tm_min, loc.tm_sec);
      Serial.printf("Décalage par rapport à l'heure UTC actuelle : +%01d heure(s)\n", loc.tm_isdst + 1);
      // Définition de la date et de l'heure de dans 2 jours (172800 = 2 jours en secondes)
      moment = moment+172800;         // Mise de la date et de heure de dans 2 jours au format unix dans la variable "moment" de type time_m
      loc2 = *(localtime(&moment));   // Mise dans la structure loc2 des éléments de date et de l'heure de dans 2 jours
      Serial.printf("Date et heure de dans 2 jours : %02d/%02d/%04d %02d:%02d:%02d\n", loc2.tm_mday, loc2.tm_mon+1, loc2.tm_year+1900, loc2.tm_hour, loc2.tm_min, loc2.tm_sec);       
      Serial.printf("Décalage par rapport à l'heure UTC de dans 2 jours : +%01d heure(s)\n", loc2.tm_isdst + 1);
      Serial.println();
      // REMARQUE : le décalage horaire et l'heure d'été sont automatiquemment pris en compte
      //            grâce à la commande "configTzTime" après la connexion au WiFi

      getRTEData();
    }
  FirstCycleDone = true; 
}