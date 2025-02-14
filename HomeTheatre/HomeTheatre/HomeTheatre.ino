#include <DHT.h>        // Libreria sensore DHT
#include <Stepper.h>    // Libreria stepper motor
#include <IRremote.hpp> //Libreria IRremote

#define DHTPIN 2      // Pin DHT11
#define DHTTYPE DHT11

#define FAN_PWM 5     // Pin per velocità della ventola
#define FAN_DIR1 4    // Pin direzione 1
#define FAN_DIR2 3    // Pin direzione 2
bool ventolaAccesa = false;

#define PHOTO_PIN A0          // Pin fotocellula
#define STEPS_PER_REV 2048    // Passi per un giro completo del motore
#define STEP 128              // Step per ogni movimento della tenda
#define MAX_POS 1024          // Massima posizione della tenda
#define MIN_POS 0             // Minima posizione della tenda
int posizioneTenda = 0;       // Posizione attuale della tenda

Stepper motore(STEPS_PER_REV, 11, 9, 10, 8); // Inizializza motore con ULN2003
DHT dht(DHTPIN, DHTTYPE);  // Inizializza DHT

// Pin IR e codici telecomando
#define IR_RECEIVE_PIN 12
#define IR_VOL_PLUS   0xB946FF00  
#define IR_VOL_MINUS  0xEA15FF00  
#define IR_PLAY_PAUSE 0xBF40FF00  
#define IR_FUNCT      0xB847FF00  
bool modalitaAutomatica = true;

void setup() {
  Serial.begin(115200);
  dht.begin();  // Avvia DHT

  // Configurazione pin della ventola
  pinMode(FAN_PWM, OUTPUT);
  pinMode(FAN_DIR1, OUTPUT);
  pinMode(FAN_DIR2, OUTPUT);

  // Configurazione pin fotocellula
  pinMode(PHOTO_PIN, INPUT);
  
  // Spegne la ventola all'avvio
  digitalWrite(FAN_DIR1, LOW);
  digitalWrite(FAN_DIR2, LOW);
  analogWrite(FAN_PWM, 0);

  motore.setSpeed(10); //velocita motore

  //Configurazione pin IR 
  IrReceiver.begin(IR_RECEIVE_PIN, DISABLE_LED_FEEDBACK);
}

void loop() {
    delay(2000);

    // Lettura del telecomando per cambio modalità
    if (IrReceiver.decode()) {
      unsigned long codiceIR = IrReceiver.decodedIRData.decodedRawData;
      Serial.print("Ricevuto codice IR: ");
      Serial.println(codiceIR, HEX);

      if (codiceIR == IR_FUNCT) {
        modalitaAutomatica = !modalitaAutomatica;
        Serial.print("Modalità cambiata: ");
        Serial.println(modalitaAutomatica ? "AUTOMATICA" : "MANUALE");
      } else if (!modalitaAutomatica) {
        // Comandi validi solo in modalità manuale
        switch (codiceIR) {
          case IR_PLAY_PAUSE:
            ventolaAccesa = !ventolaAccesa;
            if (ventolaAccesa) {
              digitalWrite(FAN_DIR1, HIGH);
              digitalWrite(FAN_DIR2, LOW);
              analogWrite(FAN_PWM, 255);
              Serial.println("Ventola ACCESA");
            } else {
              digitalWrite(FAN_DIR1, LOW);
              digitalWrite(FAN_DIR2, LOW);
              analogWrite(FAN_PWM, 0);
              Serial.println("Ventola SPENTA");
            }
            break;

          case IR_VOL_PLUS:
            if (posizioneTenda > MIN_POS) {
              posizioneTenda -= STEP;
              Serial.println("Motore: Rotazione ANTIORARIA");
              motore.step(-STEP);
            } else {
              Serial.println("Tenda già completamente alzata");
            }
            break;

          case IR_VOL_MINUS:
            if (posizioneTenda < MAX_POS) {
              posizioneTenda += STEP;
              Serial.println("Motore: Rotazione ORARIA");
              motore.step(STEP);
            } else {
              Serial.println("Tenda già completamente abbassata");
            }
            break;

          default:
            Serial.println("Comando IR non riconosciuto");
            break;
        }
      }

      IrReceiver.resume();
    }

    // Gestione automatica
    if (modalitaAutomatica) {
      // Lettura sensori
      float humidity = dht.readHumidity();
      float temperature = dht.readTemperature();
      int light = analogRead(PHOTO_PIN); 

      if (isnan(humidity) || isnan(temperature)) { 
        Serial.println("Errore nella lettura del DHT11!");
      }

      Serial.print("Umidità: ");
      Serial.print(humidity);
      Serial.print(" %\t");

      Serial.print("Temperatura: ");
      Serial.print(temperature);
      Serial.println(" °C");

      Serial.print("Luce: ");
      Serial.println(light);

      // Controllo luce per tenda
      while(light > 400 && posizioneTenda < MAX_POS) {
        posizioneTenda += STEP;
        Serial.println("Luce alta: abbassamento tenda");
        motore.step(-STEP);
        delay(500);
        light = analogRead(PHOTO_PIN);
      }

      while (light < 300 && posizioneTenda > MIN_POS) {
        posizioneTenda -= STEP;
        Serial.println("Luce bassa: alzamento tenda");
        motore.step(STEP);
        delay(500);
        light = analogRead(PHOTO_PIN);
      }

      // Controllo ventola
      if (!ventolaAccesa && (temperature > 24 || humidity > 50)) {
        digitalWrite(FAN_DIR1, HIGH);
        digitalWrite(FAN_DIR2, LOW);
        analogWrite(FAN_PWM, 255);
        Serial.println("Ventola ACCESA");
        ventolaAccesa = true;
      } else if (ventolaAccesa && (temperature <= 23 && humidity <= 46)) {
        digitalWrite(FAN_DIR1, LOW);
        digitalWrite(FAN_DIR2, LOW);
        analogWrite(FAN_PWM, 0);
        Serial.println("Ventola SPENTA");
        ventolaAccesa = false;
      }
    } 
}
