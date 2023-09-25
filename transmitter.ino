#include <SoftwareSerial.h>

SoftwareSerial mySerial(10, 11); // RX, TX

void setup() {
  Serial.begin(9600); // Configura la velocidad de transmisión inicial (modificar según las pruebas)
  mySerial.begin(9600); 
}

void loop() {
  char data[] = "10101010101010101010"; // Datos a enviar (simulados)
  char crc = calculateCRC(data); // Aquí se debe implementar la función para calcular CRC
  mySerial.write(data);
  mySerial.write(crc);
  delay(1000); // Espera un segundo
}

char calculateCRC(char* data) {
  // Implementar el algoritmo CRC aquí
  return '0'; // Retornar el valor del CRC
}
