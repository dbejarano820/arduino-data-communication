#include <SoftwareSerial.h>
#include <LiquidCrystal.h>

SoftwareSerial mySerial(10, 11); // RX, TX
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

const uint8_t DEFAULT_FLAG = 0x7E; // 01111110 en binario

void processFrameA(uint8_t *trama);
void processFrameB(uint8_t *trama);
void acknowledgeFrame(uint8_t NS);

int speed = 100;

void setup()
{
  Serial.begin(speed); // Configura la velocidad de transmisión inicial (modificar según las pruebas)
  mySerial.begin(speed);
  lcd.begin(16, 2); // inicializa el LCD
  lcd.print("Iniciando prueba...");
}

void loop()
{
  if (mySerial.available())
  {
    lcd.clear();
    lcd.print("Prueba en curso...");
    uint8_t firstByte = mySerial.read();

    if (firstByte == DEFAULT_FLAG)
    {
      uint8_t type = mySerial.read() & 0x07; // Extrae los 3 bits menos significativos para determinar el tipo

      switch (type)
      {
      case 0x00: // Trama de control para inicio
      case 0x03: // Trama de control para inicio (no primera)
      case 0x04: // Trama de control para finalización
        uint8_t frameA[9];
        frameA[0] = DEFAULT_FLAG;
        frameA[1] = type;
        mySerial.readBytes(frameA + 2, 7); // Lee el resto de la trama
        processFrameA(frameA);
        break;

      case 0x01: // Trama de información
        uint8_t frameB[205];
        frameB[0] = DEFAULT_FLAG;
        frameB[1] = type;
        mySerial.readBytes(frameB + 2, 203); // Lee el resto de la trama
        processFrameB(frameB);
        break;

        // Agregar otros tipos si es necesario
      }
    }

    lcd.clear();
    lcd.print("Prueba Finalizada!");
    lcd.setCursor(0, 1);
    lcd.print("BER: XX.xx %"); // Mostrar el BER calculado
  }
}

void processFrameA(uint8_t *frame)
{
  // Procesa la trama de tipo A (inicio/finalización)
  // Extrae y procesa la velocidad, tamaño de datos, etc.

  // Por ahora, solo muestra un mensaje en el LCD
  lcd.clear();
  lcd.print("Trama A recibida");
}

void processFrameB(uint8_t *frame)
{
  // Procesa la trama de tipo B (información)

  // Por ahora, solo muestra un mensaje en el LCD
  lcd.clear();
  lcd.print("Trama B recibida");

  uint8_t NS = frame[1] >> 3; // Extrae el número de secuencia
  acknowledgeFrame(NS);
}

void acknowledgeFrame(uint8_t NS)
{
  uint8_t frameACK[3] = {DEFAULT_FLAG, (NS << 3) | 0x05, DEFAULT_FLAG}; // Construye la trama ACK
  mySerial.write(frameACK, sizeof(frameACK));
}
