/****************************************************************************
  IE3054 - PROYECTO 1 - INVERNADERO INTELIGENTE (ESP32)
  CÓDIGO ORIGINAL - Ejemplo 2 de AdafruitIO (adafruitio_02_pubsub)
  ADAPTADO POR - Mario Betancourt (Protocolo de David Carranza)
  DESCRIPCIÓN - Este código conecta un ESP32 a una interfaz de AdafruitIO.
*****************************************************************************/

/************************** Configuration ***********************************/
#include "config.h"

/************************ Variables *******************************/
// Delay para evitar alcanzar data rate máximo
#define IO_LOOP_DELAY 10000
unsigned long lastUpdate = 0;

// Feeds de Sensores
AdafruitIO_Feed *humedadFeed      = io.feed("humedad");
AdafruitIO_Feed *temperaturaFeed  = io.feed("temp");
AdafruitIO_Feed *luzFeed  = io.feed("intensidad_luz");

// Feeds de Actuadores
AdafruitIO_Feed *motorDCFeed      = io.feed("motordc");
AdafruitIO_Feed *stepperFeed      = io.feed("motorstepper");
AdafruitIO_Feed *servoFeed        = io.feed("motorservo");

// Setup
void setup() {

  // Iniciar serial 1 a BAUD = 115200
  Serial.begin(115200);

  // Iniciar serial 2 a BAUD = 9600, 8 BITS SIN PARIDAD Y 1 BIT DE PARADA
  Serial2.begin(9600, SERIAL_8N1, 16, 17);

  // Esperar hasta que se abra el monitor serial
  while(! Serial);

  Serial.print("Connecting to Adafruit IO");

  // Conectar a io.adafruit.com
  io.connect();

  // Message Handlers
  motorDCFeed ->  onMessage(handleMotor);
  stepperFeed ->  onMessage(handleStepper);
  servoFeed   ->  onMessage(handleServo);

  // Esperar a una conexión
  while(io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  // Nos conectamos
  Serial.println();
  Serial.println(io.statusText());
}

// ... (Mantén tus includes y definiciones de feeds iguales)

void loop() {
  // Mantener la conexión con Adafruit IO
  io.run();

  // Realizar la lectura y envío cada 10 segundos (definido por IO_LOOP_DELAY)
  if (millis() > (lastUpdate + IO_LOOP_DELAY)) {
    
    // --- 1. SOLICITAR Y ENVIAR HUMEDAD ---
    Serial2.println("Q"); // Comando para humedad en el Maestro
    delay(100); 
    if (Serial2.available()) {
      String response = Serial2.readStringUntil('\n');
      int value = extractNumber(response);
      Serial.print("Enviando Humedad: "); Serial.println(value);
      humedadFeed->save(value);
    }

    // --- 2. SOLICITAR Y ENVIAR LUZ ---
    Serial2.println("L"); // Comando para luz en el Maestro
    delay(100);
    if (Serial2.available()) {
      String response = Serial2.readStringUntil('\n');
      int value = extractNumber(response);
      Serial.print("Enviando Luz: "); Serial.println(value);
      luzFeed->save(value);
    }

    // --- 3. TEMPERATURA (Placeholder) ---
    // Nota: Tu main.c aún no tiene un comando 'T' para temperatura.
    // Si lo agregas, aquí seguirías la misma lógica.

    lastUpdate = millis();
  }
}

/**
 * Función auxiliar para extraer solo los números de una cadena
 * Ejemplo: "Soil Hum: 150" -> devuelve 150
 */
int extractNumber(String str) {
  String result = "";
  for (int i = 0; i < str.length(); i++) {
    if (isDigit(str[i])) {
      result += str[i];
    }
  }
  return result.length() > 0 ? result.toInt() : 0;
}

// ... (Mantén tus handlers de motores y servos igual)

/****************************************************************/
// Message Handlers
/****************************************************************/
void handleMotor(AdafruitIO_Data *data) {
  // Convertimos a entero para una comparación numérica segura
  if (data->toInt() == 1) { 
    Serial.println("MOTOR -> Ventilador encendido.");
    Serial2.println("F1"); // Enviamos F1 + \n para el Maestro
  } else {
    Serial.println("MOTOR -> Ventilador apagado.");
    Serial2.println("F0");
  }
}

void handleStepper(AdafruitIO_Data *data) {
  if (data->toInt() == 1) {
    Serial.println("STEPPER -> Bomba activada.");
    Serial2.println("P1");
  } else {
    Serial.println("STEPPER -> Bomba desactivada.");
    Serial2.println("P0");
  }
}

void handleServo(AdafruitIO_Data *data) {
  if (data->toInt() == 1) {
    Serial.println("SERVO -> Puerta Abierta.");
    Serial2.println("S1");
  } else {
    Serial.println("SERVO -> Puerta Cerrada.");
    Serial2.println("S0");
  }
}

