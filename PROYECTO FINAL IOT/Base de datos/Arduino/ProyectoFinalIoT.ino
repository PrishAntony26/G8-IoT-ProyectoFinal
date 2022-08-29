#include <ESP8266WiFi.h>

#define BUILTIN_LED 4 //Definimos el LED en el pin nro 4 
#include "DHTesp.h" //Libreria para realizar la calibracion de los sensores de temperatura y humedad

//Inicializamos los valores de cada una de las salidas de los sensores
int hum = 0;
int temp = 0;
int humedad =0;
int i =0;

#define DHTpin 5 // declaramos el pin que va al sensor
DHTesp dht; // declaramos una variable tipo DHTest

void setup() {
  // put your setup code here, to run once:
 dht.setup(DHTpin, DHTesp::DHT22);
pinMode(BUILTIN_LED, OUTPUT);//Inicializa el BUILTIN_LED como salida
  Serial.begin(115200);
}

void loop() {
  // put your main code here, to run repeatedly:
TempAndHumidity  data = dht.getTempAndHumidity();

  hum = data.humidity;//Calibracion de los valores del sensor de humedad
  temp = data.temperature;//Calibracion de lo valores del sensor de temperatura
  humedad=map(analogRead(A0),0,1024,100,0);//Se realiza la calibracion de los valores
  //que el sensor de humedad del suelo obtiene.

  Serial.print(i);
  Serial.print(",");
  Serial.print(hum);
  Serial.print(",");
  Serial.print(temp);
  Serial.print(",");
  Serial.print(humedad);
  Serial.println(";");
  i++;
  delay(3000);
}
