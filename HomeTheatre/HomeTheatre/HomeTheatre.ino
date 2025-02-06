#include <DHT.h>      // Libreria sensore DHT

#define DHTPIN 2      // Pin DHT11
#define DHTTYPE DHT11

#define FAN_PWM 5     // Pin per velocità della ventola
#define FAN_DIR1 4    // Pin direzione 1
#define FAN_DIR2 3    // Pin direzione 2

DHT dht(DHTPIN, DHTTYPE);  // Inizializza DHT

void setup() {
    Serial.begin(9600);
    Serial.println("Inizializzazione DHT11");
    dht.begin();  // Avvia DHT

    // Configurazione pin della ventola
    pinMode(FAN_PWM, OUTPUT);
    pinMode(FAN_DIR1, OUTPUT);
    pinMode(FAN_DIR2, OUTPUT);
    
    // Spegne la ventola all'avvio
    digitalWrite(FAN_DIR1, LOW);
    digitalWrite(FAN_DIR2, LOW);
    analogWrite(FAN_PWM, 0);
}

void loop() {
    delay(2000);

    // Lettura umidità e temperatura
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature(); 

    if (isnan(humidity) || isnan(temperature)) { // Controllo se la lettura è valida
        Serial.println("Errore nella lettura del DHT11!");
        return;
    }

    Serial.print("Umidità: ");
    Serial.print(humidity);
    Serial.print(" %\t");

    Serial.print("Temperatura: ");
    Serial.print(temperature);
    Serial.println(" °C");

    // Controllo della ventola
    if (temperature > 24 || humidity > 50) {
        Serial.println("Ventola ACCESA");
        digitalWrite(FAN_DIR1, HIGH);
        digitalWrite(FAN_DIR2, LOW);
        analogWrite(FAN_PWM, 255);
    } else {
        Serial.println("Ventola SPENTA");
        digitalWrite(FAN_DIR1, LOW);
        digitalWrite(FAN_DIR2, LOW);
        analogWrite(FAN_PWM, 0);
    }
}
