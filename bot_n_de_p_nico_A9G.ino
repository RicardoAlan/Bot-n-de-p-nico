#include <NMEAGPS.h> //Paquetería para analizar los carácteres GPS
#include <GPSport.h> //Paquetería para definir los puertos del GPS
#include <SoftwareSerial.h> //Paquetería para poner pines digitales como Rx y/o Tx

#if !defined( NMEAGPS_PARSE_RMC )
  #error You must uncomment NMEAGPS_PARSE_RMC in NMEAGPS_cfg.h!
#endif
#if !defined( GPS_FIX_TIME )
  #error You must uncomment GPS_FIX_TIME in GPSfix_cfg.h!
#endif
#if !defined( GPS_FIX_LOCATION )
  #error You must uncomment GPS_FIX_LOCATION in GPSfix_cfg.h!
#endif
#if !defined( GPS_FIX_SPEED )
  #error You must uncomment GPS_FIX_SPEED in GPSfix_cfg.h!
#endif
#if !defined( GPS_FIX_SATELLITES )
  #error You must uncomment GPS_FIX_SATELLITES in GPSfix_cfg.h!
#endif
#ifdef NMEAGPS_INTERRUPT_PROCESSING
  #error You must *NOT* define NMEAGPS_INTERRUPT_PROCESSING in NMEAGPS_cfg.h!
#endif

//------------------------------------------------------------

static NMEAGPS  gps; // This parses the GPS characters

// The serial connection to the GSM device
SoftwareSerial sim(8, 9); // TX, RX

int LED = 2;
int Push = 3;

int PushActual = 0;     // estado actual del botón
int PushViejo = 0; // estado anterior del botón
unsigned long T1 = 0;    // El tiempo en que el botón se presionó 
unsigned long T2 = 0;      // El tiempo en que se dejó de presionar
unsigned long DT = 0;        // El tiempo total en que el botón fue presionado

unsigned long R = 0;     //Tiempo en el que se repetira el tiempo de envio de mensaje
unsigned long TV = 0;     //Tiempo anterior

int LEDestado = 0;

//----------------------------------------------------------------
//  Print the 32-bit integer degrees *as if* they were high-precision floats

static void printL( Print & outs, int32_t degE7 );
static void printL( Print & outs, int32_t degE7 )
{
  // Extract and print negative sign
  if (degE7 < 0) {
    degE7 = -degE7;
    outs.print( '-' );
  }

  // Whole degrees
  int32_t deg = degE7 / 10000000L;
  outs.print( deg );
  outs.print( '.' );

  // Get fractional degrees
  degE7 -= deg*10000000L;

  // Print leading zeroes, if needed
  int32_t factor = 1000000L;
  while ((degE7 < factor) && (factor > 1L)){
    outs.print( '0' );
    factor /= 10L;
  }
  
  // Print fractional degrees
  outs.print( degE7 );
}

static void doSomeWork( const gps_fix & fix );
static void doSomeWork( const gps_fix & fix )
{


  unsigned long TA = millis();     //Tiempo actual
  unsigned long TT = TA - TV;  //Resta del tiempo actual y el viejo.
  
    if (TT >= R && LEDestado == 1){
      sim.print("AT+CMGF=1\r");    //El envío del SMS en modo texto
      delay(100);
      sim.println("AT+CMGS =\"55\""); //Número de teléfono de destino
      delay(100);
      sim.print("https://maps.google.com/maps?q=");//Colocamos la url de google maps
      printL( sim, fix.latitudeL() );//Obtemos los datos de latitud del módulo gps y se lo enviamos al módulo gsm
      sim.print("+");
      printL(sim, fix.longitudeL());//Obtemos los datos de longitud del módulo gps y se lo enviamos al módulo gsm
      delay(100);
      sim.println((char)26);//El código ASCII del ctrl + z es 26
      delay(100);
      sim.println();
      delay(1000); 
      DEBUG_PORT.println("Mensaje enviado");
      TV = TA;
      R = 180000;
  }

} 

//------------------------------------

static void GPSloop();
static void GPSloop()
{
  while (gps.available( gpsPort ))
    doSomeWork( gps.read() );

} // GPSloop
  
//--------------------------

void setup()
{
  pinMode(LED,OUTPUT);
  pinMode(Push,INPUT);
  
  sim.begin(115200); //inicio comunicación con el GSM
  DEBUG_PORT.begin(9600);
  while (!DEBUG_PORT)
    ;


  DEBUG_PORT.println( F("Looking for GPS device on " GPS_PORT_NAME) );

  #ifdef NMEAGPS_NO_MERGING
    DEBUG_PORT.println( F("Only displaying data from xxRMC sentences.\n  Other sentences may be parsed, but their data will not be displayed.") );
  #endif

  DEBUG_PORT.flush();
  
  sim.print("AT+GPS=1\r");
  
  gpsPort.begin(9600);
}


void loop()
{
  PushActual = digitalRead(Push); //Estado del push buttom
  
  tiempo(); //Función tiempo
  
  if (3041 < DT && DT < 5194){ //Si el botón se presiona entre 4 y 6 segundos
    
    if (LEDestado == 0){ //Si el Led estaba apagado
      digitalWrite(LED,HIGH); //Ahora lo prende 
      LEDestado = 1; //Me cambia el estado del LED a prendido
      R = 0;
    }    
  }
  else if (6231 < DT && DT < 10856){ //Si el botón se presionó entre 8 y 11 segundos
    if (LEDestado == 1){ //Si el led estaba prendido
      digitalWrite(LED,LOW); //Ahora lo apaga
      LEDestado = 0; //Cambia el estado del Led a apagado
    }
  }
  PushViejo = PushActual; //Cambia el estado del botón 
  
  GPSloop();
}

void tiempo(){
    if (PushActual != PushViejo){ //Si el estado del Push cambia
    if (PushActual == HIGH){ //Si esta presionado el Push
      T1 = millis(); //Me toma el tiempo en que fue presionado el Push
    }
    else{
      T2 = millis(); //Me toma el tiempo en que fue liberado el Push
      DT = T2-T1; //La diferencia entre T1 y T2 me dara el tiempo en que el Push estuvo presionado
      DEBUG_PORT.println(DT);
      }
   }
}
