/*============================================================================\
| ##############  FabLamp  ###############                                    |
| FabLab Chêne20 (Renens CH)                                                  |
| https://github.com/FabLabChene20/FabLamp                                    |
| Arduino IDE v1.6.7                                                          |
| Author: Fabien Dubosson <fabien.dubosson@gmail.com>                         |
| License: MIT (http://opensource.org/licenses/MIT)                           |
\============================================================================*/

/*============================================================================\
| Définition des paramètres de configuration                                  |
\============================================================================*/

// Configuration des Pins
#define LEDR 0 // PWM 0  Pin physique 5
#define LEDG 1 // PWM 1  Pin physique 6
#define LEDB 4 // PWM 2  Pin physique 3
#define POT  3 // Input  Pin Physique 2

// Valeur en dessous de laquelle la lampe est en changement de mode
const int CHANGE_LIMIT = 100;  // 0 - 1023

// Intervales entre deux changements de mode
const int CHANGE_TIMER = 3000;  // en millisecondes

// Nombre de clignotement pour un changement de mode
const int CHANGE_BLINK = 3;

/*============================================================================\
| Définition des variables                                                    |
\============================================================================*/

// Un entier pour récupérer la valeur du potentiomètre (0 - 1023)
int val = 0;

// Un tableau de 3 entiers pour récupérer les valeurs RGB en retour de fonctions
// (byte = 0 - 255)
byte rgb_array[3];

// Le mode dans lequel on se trouve:
// 0 = Blanc
// 1 = Rouge
// 2 = Vert
// 3 = Bleu
// 4 = Couleur
// 5 = Arc-en-ciel
int mode = 0;

// Le nombre de mode (on ajoute un pour l'utiliser comme modulo)
int max_mode = 6;

// La dernière couleure utilisée en mode Arc-en-ciel
int last_color = 0;

// Trois long entiers pour stocker les temps pour faire un timer
long last_time = 0;
long curr_time = 0;
long temp_time = 0;

/*=============================================================================\
| Fonctions pour travailler avec le HSV                                        |
| (Hue: Couleur, Saturation: Saturation, Value: Valeur)                        |
| Cela permet de parcourir les couleurs de l'arc en ciel et de les transformer |
| en RGB pour la LED.                                                          |
| Code fourni par: https://github.com/ratkins/RGBConverter                     |
\=============================================================================*/

/**
 * Return the maximum value among 3 doubles.
 */
double threeway_max(double a, double b, double c) {
  return max(a, max(b, c));
}

/**
 * Return the minimum value among 3 doubles.
 */
double threeway_min(double a, double b, double c) {
  return min(a, min(b, c));
}

/**
 * Converts an HSV color value to RGB. Conversion formula
 * adapted from http://en.wikipedia.org/wiki/HSV_color_space.
 * Assumes h, s, and v are contained in the set [0, 1] and
 * returns r, g, and b in the set [0, 255].
 *
 * @param   Number  h       The hue
 * @param   Number  s       The saturation
 * @param   Number  v       The value
 * @return  Array           The RGB representation
 */
void hsvToRgb(double h, double s, double v, byte rgb[]) {
  double r, g, b;

  int i = int(h * 6);
  double f = h * 6 - i;
  double p = v * (1 - s);
  double q = v * (1 - f * s);
  double t = v * (1 - (1 - f) * s);

  switch(i % 6){
  case 0: r = v, g = t, b = p; break;
  case 1: r = q, g = v, b = p; break;
  case 2: r = p, g = v, b = t; break;
  case 3: r = p, g = q, b = v; break;
  case 4: r = t, g = p, b = v; break;
  case 5: r = v, g = p, b = q; break;
  }

  rgb[0] = r * 255;
  rgb[1] = g * 255;
  rgb[2] = b * 255;
}

/*============================================================================\
| Fonctions utiles                                                            |
\============================================================================*/

/**
 * Allume les leds en fonction du mode et de la valeur du potentiomètre.
 */
void set_mode(int mode, int val) {
  // En fonction du mode:
  switch (mode) {
  case 0: // Blanc
    analogWrite(LEDR, map(val, CHANGE_LIMIT, 1024, 0, 100));
    analogWrite(LEDG, map(val, CHANGE_LIMIT, 1024, 0, 255));
    analogWrite(LEDB, map(val, CHANGE_LIMIT, 1024, 0, 255));
    break;
  case 1: // Rouge
    analogWrite(LEDR, map(val, CHANGE_LIMIT, 1024, 0, 255));
    analogWrite(LEDG, 0);
    analogWrite(LEDB, 0);
    break;
  case 2: // Vert
    analogWrite(LEDR, 0);
    analogWrite(LEDG, map(val, CHANGE_LIMIT, 1024, 0, 255));
    analogWrite(LEDB, 0);
    break;
  case 3: // Bleu
    analogWrite(LEDR, 0);
    analogWrite(LEDG, 0);
    analogWrite(LEDB, map(val, CHANGE_LIMIT, 1024, 0, 255));
    break;
  case 4: // Couleur
    hsvToRgb(val / 1024.0, 1, 1, rgb_array);
    analogWrite(LEDR, rgb_array[0]);
    analogWrite(LEDG, rgb_array[1]);
    analogWrite(LEDB, rgb_array[2]);
    break;
  case 5: // Arc-en-ciel
    hsvToRgb(last_color / 1024.0, 1, 1, rgb_array);
    last_color += map(val, CHANGE_LIMIT, 1024, 1, 20);
    last_color %= 1024;
    analogWrite(LEDR, rgb_array[0]);
    analogWrite(LEDG, rgb_array[1]);
    analogWrite(LEDB, rgb_array[2]);
    break;
  }
}

/**
 * Indique un changement de mode
 */
void indicate_change() {
  // On éteint toutes les LED quelques millisecondes
  analogWrite(LEDR, 0);
  analogWrite(LEDG, 0);
  analogWrite(LEDB, 0);
  delay(300);
  // On fait clignoter la LED rouge 3 fois
  for(int i=0; i<3; i++){
    analogWrite(LEDR, map(val, CHANGE_LIMIT, 1024, 0, 255));
    analogWrite(LEDG, 0);
    analogWrite(LEDB, 0);
    delay(50);
    analogWrite(LEDR, 0);
    analogWrite(LEDG, 0);
    analogWrite(LEDB, 0);
    delay(50);
  }
  // On laisse éteint pendant quelques millisecondes
  delay(300);
}

/*============================================================================\
| Code de la lampe                                                            |
\============================================================================*/

void setup() {
  // On définit les modes des Pins
  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);
  pinMode(LEDB, OUTPUT);
  pinMode(POT,  INPUT);

  // On initialize la valeur du timer
  last_time = millis();
}

void loop() {
  // On lit la valeur du potentiomètre
  val = analogRead(POT);

  // On récupère le temps actuel
  curr_time = millis();

  // Si l'on est plus bas que CHANGE_LIMIT, on est en changement de mode
  if(val < CHANGE_LIMIT) {
    // Premièrement si l'on a dépassé le temps du timer
    if(curr_time - last_time > CHANGE_TIMER) {
      // On indique le changement
      indicate_change();
      // On réinitialise le timer (après l'indication, pas que le temps pris par
      // l'indication infule sur le timer)
      last_time = curr_time;
      // On passe au mode suivant
      ++mode %= max_mode;
    }

    // Ensuite on indique le mode ou l'on est. Pour les couleures simples:
    // blanc, rouge, vert et bleu, on allume simplement la couleur au maximum
    if(mode < 4){
      // On allume la LED en question au maximum
      set_mode(mode, 1024);
      // On fait une petite pause
      delay(60);
    }
    // Pour le mode Couleur, on fait clignoter Rouge-Vert-Bleu en alternance
    else if(mode == 4) {
      // Le clignotement se fait en alternance toutes les 333 millisecondes.
      // Plutôt que d'utilser un timer, on se base sur les millisecondes du
      // temps actuel
      temp_time = curr_time % 1000;
      if(temp_time < 333) {set_mode(1, 1024);}
      else if(temp_time >= 666) {set_mode(3, 1024);}
      else {set_mode(2, 1024);}
      // On fait une petite pause
      delay(60);
    }
    // Pour le mode Arc-en-ciel, on fait changer les couleurs de l'arc-en-ciel
    // très vite
    else if(mode == 5) {
      set_mode(mode, last_color);
      last_color += 10;
      last_color %= 1024;
      delay(20);
    }
  }
  else {
    // Si l'ont est pas en changement de mode:
    // On réinitialize le time de changement de mode
    last_time = curr_time;
    // On active le mode en question selon la valeur du potentiomètre
    set_mode(mode, val);
    // On fait une petite pause
    delay(60);
  }
}
