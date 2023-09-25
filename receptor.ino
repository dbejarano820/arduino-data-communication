#include <SoftwareSerial.h>
#include <LiquidCrystal.h>

SoftwareSerial mySerial(10, 11); // RX, TX
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

void setup() {
  Serial.begin(9600);  // Configura la velocidad de transmisión inicial (modificar según las pruebas)
  mySerial.begin(9600);
  lcd.begin(16, 2); // inicializa el LCD
  lcd.print("Iniciando prueba...");
}

void loop() {
  if (mySerial.available()) {
    lcd.clear();
    lcd.print("Prueba en curso...");
    
    char receivedData[20]; // Ajustar tamaño según los datos recibidos
    char crc = mySerial.read(); // Leer el CRC enviado
    
    // Leer los datos aquí
    
    // Calcular el CRC de los datos recibidos y comparar con el CRC enviado
    // Calcular el BER
    // Mostrar los resultados en el LCD
    
    lcd.clear();
    lcd.print("Prueba Finalizada!");
    lcd.setCursor(0,1);
    lcd.print("BER: XX.xx %"); // Mostrar el BER calculado
  }
}
