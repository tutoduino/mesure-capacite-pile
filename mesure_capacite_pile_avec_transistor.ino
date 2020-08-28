// Mesure de capacité d'une pile rechargeable Ni-MH 1.2 V 
// https://tutoduino.fr/
// Copyleft 2020

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define LARGEUR_ECRAN 128 // Largeur de l'écran OLED en pixels
#define HAUTEUR_ECRAN 64  // Hauteur de l'écran OLED en pixels

// Délcaration pour l'afficheur OLED relié via I2C
#define OLED_RESET     -1 // OLED sans broche RESET
Adafruit_SSD1306 display(LARGEUR_ECRAN, HAUTEUR_ECRAN, &Wire, OLED_RESET);

// Valeur de la resistance de decharge
#define R 10.0

// Seuil bas de la tension d'une pile Ni-MH
#define SEUIL_BAS_TENSION_PILE 0.80

// Détection de seuil bas de la tension de la pile
bool seuilBasBatterieAtteint = false;

// Booleen qui indique l'arrêt de la mesure car la 
// pile est déchargée
//bool fin = false;

// Heure de début de la mesure
unsigned long initTime;

// Quantité d'électricité générée par la pile lors 
// de la dernière seconde
float quantiteElectricite = 0;

// Quantité d'électricité totale générée par la pile 
// depuis le lancement du programme
float quantiteElectriciteTotale = 0;



// Fonction qui doit être appellée toutes les secondes
// La fonction mesure la tension aux bornes de la résistance
// et en déduit le courant qui y circule. 
// Elle mesure la capacité de la pile en additionnant
// le courant de décharges toutes les secondes jusqu'à ce
// que la pile soit totalement déchargée
void mesureQuantiteElectricite() {
  float U0,U1;
  float tensionResistance;
  float courant;

  // Lit la tension aux bornes de la pile et aux bornes 
  // de la résistance
  U0 = analogRead(A0)*5.0/1024.0;
  U1 = analogRead(A1)*5.0/1024.0;
  tensionResistance = U0-U1;
    
  // Verifie que la tension de la pile n'est
  // pas inferieure à son seuil bas afin de ne pas 
  // l'endommager
  if (U0 < SEUIL_BAS_TENSION_PILE) {
    seuilBasBatterieAtteint = true;
  }

  // Calcul du courant qui circule dans la résistance
  courant = (U0-U1)/R;

  // La quantité d'électricité est la quantité de charges électriques
  // déplacées par les électrons. Elle est égale à l'intensité
  // multipliée par le temps (1 seconde ici).
  // Afin d'avoir la quantité d'électricité en mAh, il faut
  // utiliser le facteur 1000/3600
  quantiteElectricite = courant/3.6;

  // On additionne cette quantité d'électricité aux précédentes
  // afin de connaître la quantité totale d'électricité
  // à la fin de la mesure
  quantiteElectriciteTotale = quantiteElectriciteTotale+quantiteElectricite;

  // Affichage sur l'afficheur OLED de la tension de la pile, 
  // du courant débité ainsi que de la quantité d'électricité débitée 
  // par la pile depuis le lancement du programme
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);  
  display.setCursor(0, 0);
  display.println("Tutoduino");

  display.setTextSize(1);
  display.setCursor(0, 20);
  display.print("U=");
  display.print(U0);
  display.println(" V");
  
  display.setCursor(0,30);
  display.print("I=");
  display.print(courant);
  display.println(" A");

  display.setCursor(0,40);
  display.print("Q=");  
  display.print(quantiteElectriciteTotale);
  display.println(" mAh");
  display.display(); 
}

void setup() {

  Serial.begin(9600);

  // Initialisation de l'écran OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Echec initialisation afficheur"));
    for(;;); // Don't proceed, loop forever
  }
  
  // Positionne la référence de tension par défaut
  // la tension de référence sera donc la tension 
  // d'alimentation du micro-contrôleur
  analogReference(DEFAULT);

  // Heure de début de la mesure 
  initTime = millis();

  // La sortie D7 sera utilisé pour contrôler 
  // l'état du transistor
  pinMode(7, OUTPUT);
  delay(100);

  Serial.println("Décharge de la pile terminée");

  // Passe le transistor dans son état saturé
  // afin de démarrer la décharge de la pile
  digitalWrite(7, HIGH);


}

void loop() {
  // Mesure de la quantité d'électricité débitée toutes les 
  // secondes jusqu'à ce que la tension de la pile soit inférieure
  // à son seuil bas afin de ne pas l'endommager.
  if (seuilBasBatterieAtteint == false) {
    mesureQuantiteElectricite();
    delay(1000);
  } else {
    // Une fois que le seule bas de la pile est atteint, 
    // on positionne le transistor en mode bloqué afin
    // d'arrêter la décharge de la pile
    digitalWrite(7, LOW);
    Serial.println("Décharge de la pile terminée");
    Serial.print("Capacité de la pile = ");
    Serial.print(quantiteElectriciteTotale);
    Serial.println(" mAh");
    display.setCursor(0,50);
    display.print("Mesure terminee !");  
    display.display();     
    delay(1000);
  }

}
