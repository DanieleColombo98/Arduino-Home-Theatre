#include <DHT.h>        // Libreria sensore DHT
#include <Stepper.h>    // Libreria stepper motor
#include <IRremote.hpp> //Libreria IRremote

#define DHTPIN 2      // Pin DHT11
#define DHTTYPE DHT11

#define FAN_PWM 5     // Pin per velocità della ventola
#define FAN_DIR1 4    // Pin direzione 1
#define FAN_DIR2 3    // Pin direzione 2
bool ventolaAccesa = false;

#define PHOTO_PIN A0  // Pin fotocellula

#define STEPS_PER_REV 2048   // Passi per un giro completo del motore

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
  Serial.println("Inizializzazione DHT11");
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
            Serial.println("Motore: Rotazione ORARIA");
            motore.step(512);
            break;

          case IR_VOL_MINUS:
            Serial.println("Motore: Rotazione ANTIORARIA");
            motore.step(-512);
            break;

          default:
            Serial.println("Comando IR non riconosciuto");
            break;
        }
      }

      delay(1000);
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

      // Controllo motore per tenda
      if (light < 500) {
        Serial.println("Luce bassa: motore in senso antiorario");
        motore.step(-512);
        delay(1000);
      } else if (light > 900) {
        Serial.println("Luce alta: motore in senso orario");
        motore.step(512);
        delay(1000);
      }
    } 
}
