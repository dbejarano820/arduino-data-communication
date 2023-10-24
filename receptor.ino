#include <SoftwareSerial.h>
#include <LiquidCrystal.h>
#define MAX_FRAME_SIZE 205
SoftwareSerial mySerial(10, 11); // RX, TX
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

const uint8_t DEFAULT_FLAG = 0x7E; // 01111110 en binario

void processFrameA(uint8_t *trama);
void processFrameB(uint8_t *trama);
void acknowledgeFrame(uint8_t NS);

int speed = 300;
uint8_t buffer[MAX_FRAME_SIZE];
uint8_t frameIndex = 0;
bool readingFrame = false;

void setup()
{
  Serial.begin(speed); // Configura la velocidad de transmisión inicial (modificar según las pruebas)
  mySerial.begin(speed);
  lcd.begin(16, 2); // inicializa el LCD
  lcd.print("Iniciando prueba...");
}

void loop()
{
  while (Serial.available() > 0)
  {
    lcd.clear();
    lcd.print("Prueba en curso...");
    uint8_t byte = Serial.read();
    if (byte == DEFAULT_FLAG)
    {
      if (readingFrame)
      {
        // Nueva bandera de entrada, fin de trama
        readingFrame = false;
        // Procesar trama
        for (int i = 0; i < frameIndex; i++)
        {
          Serial.print(buffer[i]);
          Serial.println("");
        }
        frameIndex = 0;
      }
      else
      {
        readingFrame = true;
      }
    }
    else
    {
      if (readingFrame && frameIndex < MAX_FRAME_SIZE)
      {
        buffer[frameIndex] = byte;
        frameIndex++;
      }
    }

    // if (firstByte == DEFAULT_FLAG)
    // {
    //   Serial.println("Message begin");
    //   uint8_t type = Serial.read();
    //   Serial.print(secondByte);
    //   Serial.print(type);
    // Serial.readBytes(1);// & 0x07; // Extrae los 3 bits menos significativos para determinar el tip

    //      switch (type)
    //      {
    //      case 0x00: // Trama de control para inicio
    //      case 0x03: // Trama de control para inicio (no primera)
    //      case 0x04: // Trama de control para finalización
    //        uint8_t frameA[9];
    //        frameA[0] = DEFAULT_FLAG;
    //        frameA[1] = type;
    //        Serial.readBytes(frameA + 2, 7); // Lee el resto de la trama
    //        processFrameA(frameA);
    //        break;
    //
    //      case 0x01: // Trama de información
    //        uint8_t frameB[205];
    //        frameB[0] = DEFAULT_FLAG;
    //        frameB[1] = type;
    //        Serial.readBytes(frameB + 2, 203); // Lee el resto de la trama
    //        processFrameB(frameB);
    //        break;
    //
    //        // Agregar otros tipos si es necesario
    //      }
  }
  //
  //    lcd.clear();
  //    lcd.print("Prueba Finalizada!");
  //    lcd.setCursor(0, 1);
  //    lcd.print("BER: XX.xx %"); // Mostrar el BER calculado
  //  }
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
  // Serial.write(frameACK, sizeof(frameACK));
}
