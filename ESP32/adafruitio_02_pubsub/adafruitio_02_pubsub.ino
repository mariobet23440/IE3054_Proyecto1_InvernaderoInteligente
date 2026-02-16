// Adafruit IO Publish & Subscribe Example

/************************** Configuration ***********************************/

// edit the config.h tab and enter your Adafruit IO credentials
// and any additional configuration needed for WiFi, cellular,
// or ethernet clients.
#include "config.h"

/************************ Example Starts Here *******************************/

// this int will hold the current count for our sketch
int count = 0;

#define IO_LOOP_DELAY 10000
unsigned long lastUpdate = 0;

// set up the 'counter' feed
AdafruitIO_Feed *counter          = io.feed("counter");
AdafruitIO_Feed *humedadFeed      = io.feed("humedad");
AdafruitIO_Feed *temperaturaFeed  = io.feed("temp");
AdafruitIO_Feed *motorDCFeed      = io.feed("motordc");
AdafruitIO_Feed *comandoFeed      = io.feed("comando");

void setup() {

  // start the serial connection
  Serial.begin(115200);

  // 1. Arrancamos el Serial 2 (ajusta los baudios según lo que necesites)
  Serial2.begin(9600, SERIAL_8N1, 16, 17);

  // wait for serial monitor to open
  while(! Serial);

  Serial.print("Connecting to Adafruit IO");

  // connect to io.adafruit.com
  io.connect();

  // set up a message handler for the count feed.
  // the handleMessage function (defined below)
  // will be called whenever a message is
  // received from adafruit io.
  counter->onMessage(handleMessage);

  // 1. Vinculamos el feed "motorDCFeed" con una función de manejo
  motorDCFeed->onMessage(handleMotor);

  comandoFeed->onMessage(handleComando);

  // wait for a connection
  while(io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  // we are connected
  Serial.println();
  Serial.println(io.statusText());
  counter->get();

}

void loop() {
  io.run();

  if (millis() > (lastUpdate + IO_LOOP_DELAY)) {
    
    // CAMBIO AQUÍ: Vamos a enviar un número fijo para probar
    int miNumero = 23440; 
    
    Serial.print("Enviando a Adafruit IO -> ");
    Serial.println(miNumero);
    
    // Enviamos el número al feed
    counter->save(miNumero);

    temperaturaFeed -> save(miNumero);
    humedadFeed -> save(miNumero);

    lastUpdate = millis();
  }
}

// this function is called whenever a 'counter' message
// is received from Adafruit IO. it was attached to
// the counter feed in the setup() function above.
void handleMessage(AdafruitIO_Data *data) {

  Serial.print("received <- ");
  Serial.println(data->value());

}

void handleMotor(AdafruitIO_Data *data) {
  
  Serial.print("Botón recibido -> ");
  Serial.println(data->value()); // Imprime "1" o "0"

  // Convertimos el dato a entero para usarlo fácilmente
  int estado = data->toInt();

  if (estado == 1) {
    Serial.println("¡Encendiendo Motor!");
    // digitalWrite(PIN_MOTOR, HIGH); // Aquí podrías activar un pin real
  } else {
    Serial.println("Apagando Motor...");
    // digitalWrite(PIN_MOTOR, LOW);
  }
}

void handleComando(AdafruitIO_Data *data) {
  // Extraemos el valor como texto
  String valor = data->value(); 

  Serial.print("Comando recibido: ");
  Serial.println(valor);

  // Comparamos con comillas DOBLES porque es un String
  if (valor == "A") {
    Serial.println("Acción para A: Enviando al Serial 2");
    Serial2.print('A'); 
  } 
  else if (valor == "B") {
    Serial.println("Acción para B: Enviando al Serial 2");
    Serial2.print('B');
  }
}
