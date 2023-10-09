#include <SoftwareSerial.h>
#include <LiquidCrystal.h>
#include <stdint.h>

SoftwareSerial mySerial(10, 11); // RX, TX
uint8_t datos[1];                // Initialized variable to store recieved data
char display[1];
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

uint16_t calculateCRC(uint8_t *datos, size_t longitud);

void setup()
{
  lcd.begin(16, 2);
  Serial.begin(9600); // Configura la velocidad de transmisión inicial (modificar según las pruebas)
  mySerial.begin(9600);
}

void loop()
{
  uint8_t datos[] = {0xAA};                                  // Datos a enviar (simulados)
  uint16_t resultadoCRC = calcularCRC(datos, sizeof(datos)); // Aquí se debe implementar la función para calcular CRC

  Serial.readBytes(datos, 1); // Read the serial data and store in var
  // Serial.print(datos); //Print data on Serial Monitor
  Serial.print(datos[0]);
  delay(1000);
  if (Serial.available())
  {
    // wait a bit for the entire message to arrive
    delay(100);
    // clear the screen
    lcd.clear();
    sprintf(display, " %02x", datos[0]);
    // read all the available characters
    while (Serial.available() > 0)
    {
      // display each character to the LCD
      lcd.write(display);
    }
  }
}

/*
  Calcula el valor CRC de un conjunto de bits (MODBUS)
  Se usa el tipo de dato uint8_t que es equivalente a un unsigned char (8 bytes) con la
  finalidad de usar la operación XOR para el CRC, con el polinomio x^16 + x^15 + x^2 + 1
  Entradas: Arreglo de bytes sobre cuales hacer el cálculo de CRC, longitud de los datos ingresados
  Salidas: Devuelve el valor calculado sobre el CRC
*/
uint16_t calculateCRC(uint8_t *datos, size_t longitud)
{
  uint16_t crc = 0xFFFF; // Valor inicial del CRC

  for (size_t i = 0; i < longitud; ++i)
  {
    crc ^= datos[i]; // Realiza la operación XOR con el byte actual

    // Realiza el cálculo para cada bit del byte
    for (int j = 0; j < 8; ++j)
    {
      if (crc & 0x0001)
      {
        crc >>= 1;     // Bit shift
        crc ^= 0xA001; // Polinomio generador para CRC-16, forma reversa de 0x8005 para adecuarse al XOR y otras operaciones de bits
      }
      else
      {
        crc >>= 1;
      }
    }
  }

  return crc;
}