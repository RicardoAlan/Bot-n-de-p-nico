#include <TinyGPS++.h> //Paquetería para extraer información del GPS
#include <SoftwareSerial.h> //Paquetería para poner pines digitales como Rx y/o Tx

static const uint32_t GPSBaud = 9600; //velocidad de transmisión del GPS
//static const uint32_t GSMBaud = 115200; // velocidad de transmisión del GSM

// The TinyGPS++ object
TinyGPSPlus gps; //defino gps para usar paqueteria

// The serial connection to the GPS device
SoftwareSerial ss(5, 6); // TX, RX

// The serial connection to the GSM device
SoftwareSerial sim(2, 3); // TX, RX


int LED = 10;
int Push = 11;

int PushActual = 0;     // estado actual del botón
int PushViejo = 0; // estado anterior del botón
unsigned long T1 = 0;    // El tiempo en que el botón se presionó 
unsigned long T2 = 0;      // El tiempo en que se dejó de presionar
unsigned long DT = 0;        // El tiempo total en que el botón fue presionado

unsigned long R = 0;     //Tiempo en el que se repetira el tiempo de envio de mensaje
unsigned long TV = 0;     //Tiempo anterior

int LEDestado = 0;




void setup() {

  pinMode(LED,OUTPUT);
  pinMode(Push,INPUT);
  
  sim.begin(115200); //inicio comunicación con el GSM
  delay(1000);
  Serial.begin(9600); //COMUNICACIÓN CON EL MONITOR SERIAL
  delay(1000);
  ss.begin(GPSBaud); //inicio comunicación con el GPS

}

void loop() {

  while (ss.available() > 0) //Verificamos si el módulo gps está enviando datos
  if (gps.encode(ss.read())) //Y los leemos
  
  PushActual = digitalRead(Push); //Estado del push buttom
  
  tiempo(); //Función tiempo
  
  if (4000 < DT && DT < 6000){ //Si el botón se presiona entre 4 y 6 segundos
    
    if (LEDestado == 0){ //Si el Led estaba apagado
      digitalWrite(LED,HIGH); //Ahora lo prende 
      LEDestado = 1; //Me cambia el estado del LED a prendido
      R = 0;
    }    
  }
  else if (8000 < DT && DT < 11000){ //Si el botón se presionó entre 8 y 11 segundos
    if (LEDestado == 1){ //Si el led estaba prendido
      digitalWrite(LED,LOW); //Ahora lo apaga
      LEDestado = 0; //Cambia el estado del Led a apagado
    }
  }
  PushViejo = PushActual; //Cambia el estado del botón 
  
  unsigned long TA = millis();     //Tiempo actual
  unsigned long TT = TA - TV;  //Resta del tiempo actual y el viejo.
    
  if (TT >= R && LEDestado == 1){
    EnvioSMS();
    TV = TA;
    R = 180000;
  }
}



void tiempo(){
    if (PushActual != PushViejo){ //Si el estado del Push cambia
    if (PushActual == HIGH){ //Si esta presionado el Push
      T1 = millis(); //Me toma el tiempo en que fue presionado el Push
    }
    else{
      T2 = millis(); //Me toma el tiempo en que fue liberado el Push
      DT = T2-T1; //La diferencia entre T1 y T2 me dara el tiempo en que el Push estuvo presionado
      Serial.println(DT);
      }
   }
}





void EnvioSMS(){  
  sim.print("AT+CMGF=1\r");    //El envío del SMS en modo texto
  delay(100);
  sim.println("AT+CMGS =\"5534644316\""); //Número de teléfono de destino
  delay(100);
  sim.print("https://maps.google.com/maps?q=");//Colocamos la url de google maps
  sim.print(gps.location.lat(), 6);//Obtemos los datos de latitud del módulo gps y se lo enviamos al módulo gsm
  sim.print("+");
  sim.print(gps.location.lng(), 6);//Obtemos los datos de longitud del módulo gps y se lo enviamos al módulo gsm
  delay(100);
  sim.println((char)26);//El código ASCII del ctrl + z es 26
  delay(100);
  sim.println();
  delay(1000); 
  Serial.println("Mensaje enviado");
}




void displayInfo()
{
  Serial.print(F("Ubicación: ")); //Imprime la palabra ubicación 
  if (gps.location.isValid()) //Si el GPS me esta mandando info de ubicacion 
  {
    Serial.print(F("https://maps.google.com/maps?q=")); //Imprime el incio del LINK a google maps
    Serial.print(gps.location.lat(), 6); //Me toma la latitud
    Serial.print(F("+"));
    Serial.print(gps.location.lng(), 6); //Me toma la longitud y crea así el link a maps
  }
  else
  {
    Serial.print(F("INVALID")); //Si no hay nada lo marca como inválido 
  }

  Serial.print(F(" GMT Date/Time: ")); //Me toma la fecha en GMT
  if (gps.date.isValid()) //Si el GPS me esta mandando info de la fecha  
  {
    Serial.print(gps.date.day());
    Serial.print(F("/"));
    Serial.print(gps.date.month()); 
    Serial.print(F("/"));
    Serial.print(gps.date.year());
  }
  else
  {
    Serial.print(F("INVALID")); //Si no hay nada lo marca como inválido 
  }

  Serial.print(F(" "));
  if (gps.time.isValid()) //Me toma la hora 
  {
    if (gps.time.hour() < 10) Serial.print(F("0"));
    Serial.print(gps.time.hour());
    Serial.print(F(":"));
    if (gps.time.minute() < 10) Serial.print(F("0"));
    Serial.print(gps.time.minute());
    Serial.print(F(":"));
    if (gps.time.second() < 10) Serial.print(F("0"));
    Serial.print(gps.time.second());
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.println();
}
