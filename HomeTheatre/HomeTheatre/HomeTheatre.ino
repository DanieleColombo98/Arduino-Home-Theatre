#include <DHT.h>      //Librerira sensore DHT

#define DHTPIN 2      // Pin DHT11
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);  // Inizializza DHT

void setup() {
    Serial.begin(9600);
    Serial.println("Inizializzazione DHT11");
    dht.begin();  // Avvia DHT
}

void loop() {
    delay(2000);

    //Lettura umidità e temperatura
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
}

