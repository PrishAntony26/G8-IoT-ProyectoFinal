/******************* Librerías usadas *************************/
//Libreria para conectarse a una red Wifi con el ESP8266
#include <ESP8266WiFi.h>
//Libreria para que el microcontrolador se comporte como un cliente MQTT
#include <PubSubClient.h>
//Libreria para realizar la calibracion de los sensores de temperatura y humedad
#include "DHTesp.h" 

/******** Credenciales para la conexión MQTT *****************/
const char* ssid = "MOVISTAR-MalcaR.";//Nombre de red 
const char* password = "I8k7l911@MalcaRamirez25.Com";// Contraseña de la red
const char* mqtt_server = "192.168.1.25"; //ip de broker  
const char* mqtt_user = "grupo8";
const char* mqtt_password = "grupo8";

/************* Definimos los pines usados ****************/
//Se define los pines para los diferentes sensores 
const int TRIG = 14;      // trigger en pin D5
const int ECHO = 12;      // echo en pin D6
const int relay = 16;    // relay en PIN D0 
#define DHTpin 5         // declaramos el pin nro 5 - D1 para el sensor de temperatura y humedad
const int ledRojo = 2; //pin D4 LED-RGB
const int ledVerde = 0; //pin D3 LED-RGB
const int ledAzul = 4; //pin D2 LED-RGB
const int led =13;//pin D7 LED

const float VelSon = 34000.0;// Constante velocidad sonido en cm/s

bool validar= true; //Auxiliar para accionar solo una vez el envio de correo
bool validarriego= true;//Auxiliar para accionar como máximo dos veces el riego

/****** Inicializamos los valores de cada una de las salidas de los sensores ********/
float hum = 0;
float temp = 0;
float humedad =0;
float distancia;
String regar;

/******* Vector de caracteres para enviar el mensaje al topico ************/
char datos[100];
char datos2[100];
char datos3[100];
char datos4[100];
char datos5[100];

DHTesp dht; // declaramos una variable tipo DHTest

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

void setup_wifi() {
  //Configuracion del wifi para establecer la conexion   
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  //Wifi en modo estación (STA)
  WiFi.mode(WIFI_STA); 
  //Conectarse a la red wifi
  WiFi.begin(ssid, password);
  //Mientras no nos conectemos a la red wifi 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  //Inicializa el generador de numeros pseudo aleatorios con una semilla 
  //establecida por el numero de ms en el cual arduino comenzo a ejecutar el programa
  randomSeed(micros());
  
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
}

void callback(char* topic, byte* payload, unsigned int length) {
  String  cadena;
  Serial.print("Message arrived [");
  Serial.print(topic); //Topico al cual nos suscribimos
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    cadena += (char)payload[i];
  }
  regar = cadena; //Obtenemos el mensaje de salida del topico "Regar"

}

void reconnect() {
  // Codigo que se ejecutara mientras no se establezca la conexion con el protocolo MQTT.
  while (!client.connected()) { //Verifica si el cliente esta conectado al servidor 
    Serial.print("Attempting MQTT connection...");
    // Crea un cliente con un id aleatorio, el cual se usara para conectar al servidor
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    //Si la conexion se realiza de forma correcta. 
    if (client.connect(clientId.c_str())) { 
      Serial.println("connected");
      // Publica un mensaje en el topico "outTopic"
      client.publish("outTopic", "hello world");
      // Realiza una suscripción a un topico especifico "Riego"
      client.subscribe("Riego");
    } else {
      //Mensaje que se imprime en caso la conexion no se pudo realizar
      Serial.print("failed, rc="); 
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  //Se definen los pines del LED RGB como salida
  pinMode(ledRojo,OUTPUT);
  pinMode(ledVerde,OUTPUT);
  pinMode(ledAzul,OUTPUT);
  //Se define LED como salida
  pinMode(led,OUTPUT);
 
  pinMode(TRIG, OUTPUT);   // trigger como salida(sensorultrasonido)
  pinMode(ECHO, INPUT);    // echo como entrada(sensorultrasonido)
  pinMode(relay, OUTPUT); // Configurar relay como salida o OUTPUT
  dht.setup(DHTpin, DHTesp::DHT22);
  pinMode(A0,INPUT); //El pin A0 establecido como entrada 
  Serial.begin(115200);
  setup_wifi();//Configuracion del wifi para establecer la conexion   
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {

TempAndHumidity  data = dht.getTempAndHumidity(); //Implementacion de la funcion que permitira realizar la calibracion de sensores.
  
  if (!client.connected()) {//Si el cliente no esta conectado al servidor
    reconnect();//Llamada a funcion reconnect
  }
  client.loop();//Funcion que permite que el cliente procese los mensajes entrantes y mantenga la conexion con el servidor. 
  
  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    hum = data.humidity;//Calibracion de los valores del sensor de humedad
    temp = data.temperature;//Calibracion de lo valores del sensor de temperatura
    humedad=map(analogRead(A0),0,1024,100,0);//Se realiza la calibracion de los valores que el sensor de humedad del suelo obtiene.
    //Obtener valores del sensor ultrasonido
    iniciarTrigger();

    unsigned long tiempo = pulseIn(ECHO, HIGH);// La función pulseIn obtiene el tiempo que tarda en cambiar entre estados, en este caso a HIGH
  
    distancia = tiempo * 0.000001 * VelSon / 2.0;  // Obtenemos la distancia en cm, hay que convertir el tiempo en segudos ya que está en microsegundos por eso se multiplica por 0.000001
    
    //Publicaciones para cada uno de los topicos
    sprintf(datos, "%.2f", temp);
    client.publish("Temperature",datos);

    sprintf(datos2, "%.2f", hum);
    client.publish("Humedad",datos2);
    
    sprintf(datos3, "%.2f", humedad);
    client.publish("HumedadSuelo",datos3); 
  }
  
  //Riego opcional, en caso se cumplan con las condiciones de las lecturas de los sensores
  if ((humedad <50 || temp > 20 || hum <80)&& validarriego){
    strcpy(datos4, "inicia riego ");
    digitalWrite(relay, HIGH); // envia señal alta al relay
    validarriego=false;
    digitalWrite(led, HIGH);//
    delay(60000);
    digitalWrite(led, LOW);
    digitalWrite(relay, LOW);  // envia señal baja al relay
  }
  else{
    strcpy(datos4, "no regar");
  }
  
  delay(1000);
  if(distancia<=6){
    //Hacer color rojo
    digitalWrite(ledRojo,0); 
    digitalWrite(ledVerde,0);
    digitalWrite(ledAzul,0);
    validar=true;
  }
  //Condicion para que se envíe el correo 
  if (distancia>6 && validar){
    strcpy(datos5, "Llenar tanque");
    //Hacer color rojo
    digitalWrite(ledRojo,0); 
    digitalWrite(ledVerde,0);
    digitalWrite(ledAzul,255);
    
    validar= false; //validar distancia
    client.publish("Correo",datos5);
  }
  client.publish("Mensaje",datos4);
  
  //Riego obligatorio diario. 
  if(regar == "1")
  { 
    validarriego = true; //riego dos veces por dia (maximo)
    digitalWrite(relay, HIGH); // envia señal alta al relay
  }
  else if(regar == "0"){
    digitalWrite(relay, LOW);  // envia señal baja al relay
  }
    delay(5000); 
  }
  // Método que inicia la secuencia del Trigger para comenzar a medir
  void iniciarTrigger()
  {
    // Ponemos el Triiger en estado bajo y esperamos 2 ms
    digitalWrite(TRIG, LOW);
    delayMicroseconds(2);
    
    // Ponemos el pin Trigger a estado alto y esperamos 10 ms
    digitalWrite(TRIG, HIGH);
    delayMicroseconds(10);
    
    // Comenzamos poniendo el pin Trigger en estado bajo
    digitalWrite(TRIG, LOW);
  }
 
