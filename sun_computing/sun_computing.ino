/*
 * Hypothèses
 * La hauteur du soleil sur l'horizon dans la journée est proportionnelle à une sinusoide dont l'angle 0° est à 6h solaire pour la longitude concernée.
 * La hauteur maximale du soleil sur l'horizon est proportionnelle à une sinusoide dont l'angle 0° est à l'équinoxe.
 * L'angle de la hauteur maximale du soleil sur l'horizon est 90-(latitude+déclinaison apparente) 
 * L'heure solaire pour la longitude concernée de lever/coucher du soleil est 6h/18h à l'équinoxe.

 * T heure solaire (0 à 24*60)
 * L latitude 
 * J n° jour dans l'année civile 
 * d déclinaison - 23°44
 * da déclinaison apparente d*cos((360/365)*(J+10))
 * ASLC L'angle de lever/coucher du soleil relatif à l'équinoxe 
 * ASLC = 90-da-L
 * AHSE angle hauteur solaire instantanée à l'équinoxe
 * AHSE = sin(90*(T-(6*60))/(6*60))*(90-L) 
 * AHS angle hauteur solaire instantanée
 * AHS = AHSE-ASLC
 * OS orientation soleil
 * OS = T/(24*60)*360


 * Az azimuth
 * Az = 180-(ACOS((SIN(H) * SIN(L) - SIN(δ)) / (COS(H) * COS(L))))
 * Az = 180+(ACOS((SIN(H) * SIN(L) - SIN(δ)) / (COS(H) * COS(L))))

 * H = Hauteur du soleil = ASIN(SIN(δ)*SIN(L)+COS(δ)*COS( L)*COS(Ah))
 * Ah = angle horaire = 180* (Heure solaire /12)
 * L = Latitude
 * δ = déclinaison (22.7°)

 */

#define DECL -23,75;                          // déclinaison
#define DECLA DECL*cos((360/365)*(jour+10));  // déclinaison apparente du jour
#define ASLC 90-DECLA-latitude;               // cap lever/coucher du soleil
#define AZIME sin(90*(heure_solaire-(6*60))/(6*60))*(90-latitude);  // azimuth équinoxe
#define AZIM AZIME-ASLC;                      // azimuth instantané


float jour;
float heure_solaire;
float latitude=42,69;
float declinaison;



void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
