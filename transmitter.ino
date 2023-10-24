#include <SoftwareSerial.h>
#include <LiquidCrystal.h>
#include <stdint.h>

using namespace std;

SoftwareSerial mySerial(10, 11); // RX, TX
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

const uint8_t DEFAULT_FLAG = 0x7E; // 01111110 en binario
const uint16_t speed = 300;
const uint16_t payloadSize = 200;
static const size_t frameASize = 9;
static const size_t frameBSize = 205;
bool first_transmission = true;
uint8_t data[payloadSize];
enum : uint8_t
{
  initialHandshake = 0b000,
  infoFrame = 0b001,
  endCurrentComm = 0b010,
  nonnitialHandshake = 0b011,
  finalComm = 0b100,
  ack = 0b101
};
int numberOfTests = 1;
/*
  Calcula el valor CRC de un conjunto de bits (MODBUS)
  Se usa el tipo de dato uint8_t que es equivalente a un unsigned char (8 bytes) con la
  finalidad de usar la operación XOR para el CRC, con el polinomio x^16 + x^15 + x^2 + 1
  Entradas: Arreglo de bytes sobre cuales hacer el cálculo de CRC, longitud de los datos ingresados
  Salidas: Devuelve el valor calculado sobre el CRC
*/

uint16_t calculateCRC(uint8_t *data, size_t longitud)
{
  uint16_t crc = 0xFFFF; // Valor inicial del CRC

  for (size_t i = 0; i < longitud; ++i)
  {
    crc ^= data[i]; // Realiza la operación XOR con el byte actual

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

uint8_t *buildFrameA(uint8_t type, uint16_t speed, uint16_t dataSize)
{
  static uint8_t frameA[9]; // Estático para preservar el valor entre llamadas

  frameA[0] = DEFAULT_FLAG;
  frameA[1] = type;
  frameA[2] = speed >> 8;      // byte alto de velocidad
  frameA[3] = speed & 0xFF;    // byte bajo de velocidad
  frameA[4] = dataSize >> 8;   // byte alto de tamaño
  frameA[5] = dataSize & 0xFF; // byte bajo de tamaño

  uint16_t crc = calculateCRC(frameA + 1, 5); // Calcula el CRC de los datos en la trama (sin incluir banderas)
  frameA[6] = crc >> 8;                       // byte alto de CRC
  frameA[7] = crc & 0xFF;                     // byte bajo de CRC
  frameA[8] = DEFAULT_FLAG;

  return frameA;
}

uint8_t *buildFrameB(uint8_t NS, uint8_t type, uint8_t *information)
{
  static uint8_t frameB[205];

  frameB[0] = DEFAULT_FLAG;
  frameB[1] = (NS << 3) | type;         // Desplaza NS 3 bits a la izquierda y OR con tipo
  memcpy(frameB + 2, information, 200); // Copia 200 bytes de información

  uint16_t crc = calculateCRC(frameB + 1, 202); // Calcula el CRC de los datos en la trama (sin incluir banderas)
  frameB[202] = crc >> 8;                       // byte alto de CRC
  frameB[203] = crc & 0xFF;                     // byte bajo de CRC
  frameB[204] = DEFAULT_FLAG;

  return frameB;
}

// Missing acknowledge logic
void sendFrame(uint8_t *frame, uint8_t NS, int frameType)
{
  delay(5000);
  if (frameType == -1)
  {

    for (int i = 0; i < frameASize; i++)
    {
      Serial.write(*frame);
      delay(50);
      frame++;
    }
  }
  else
  {
    for (int i = 0; i < frameBSize; i++)
    {
      Serial.write(*frame);
      delay(50);
      frame++;
    }
  }
  // Serial.println("\nDone\n");
}

bool acked(uint8_t NS)
{
  Serial.println("Sending...");
  if (Serial.available())
  {

    uint8_t firstByte = Serial.read();
    uint8_t rawData = Serial.read();
    uint8_t lastByte = Serial.read();
    if (firstByte == DEFAULT_FLAG && lastByte == DEFAULT_FLAG)
    {
      uint8_t receivedSN = rawData | 0x07;
      uint8_t type = rawData & 0x07;

      if ((NS != 0 || receivedSN == NS) && type == ack)
      {
        Serial.println("Acked...");
        return true;
      }
      else
      {
        Serial.println("Not Acked...");
        return false;
      }
    }
    Serial.println("Not Acked...");
    return false;
  }
}

void setup()
{
  lcd.begin(16, 2);
  Serial.begin(speed); // Configura la velocidad de transmisión inicial (modificar según las pruebas)
  mySerial.begin(speed);
  randomSeed(analogRead(0));
  for (int i = 0; i < payloadSize; i++)
  {
    data[i] = random(256);
  }
}

void loop()
{
  uint8_t *frameA = nullptr;
  uint8_t *frameB = nullptr;
  for (int i = 0; i < numberOfTests; i++)
  {
    // Start test!
    int bytesCopied = 0;
    int copySize = 200;

    while (bytesCopied < payloadSize)
    {
      uint8_t NS = random(256);
      // Envío primera comunicación
      if (first_transmission)
      {
        // Serial.println("First A transmission");
        frameA = buildFrameA(initialHandshake, (uint16_t)speed, (uint16_t)payloadSize);
        Serial.print(frameA[0], BIN);
        sendFrame(frameA, NS, -1);
        first_transmission = false;
      }
      // Envío comunicación no inicial
      else
      {
        // Serial.println("Non initial A transmission");
        frameA = buildFrameA(nonnitialHandshake, (uint16_t)speed, (uint16_t)payloadSize);
        sendFrame(frameA, NS, -1);
      }

      int remainingBytes = payloadSize - bytesCopied;
      if (remainingBytes < copySize)
      {
        copySize = remainingBytes;
      }

      bytesCopied += copySize;

      // Envío trama B
      // Serial.println("\nB transmission");
      frameB = buildFrameB(NS, infoFrame, data + bytesCopied);
      sendFrame(frameB, NS, 0);
      // Envío trama A (Faltan más comunicaciones en la prueba)
      // Serial.println("\nlast comm for this test A transmission");
      frameA = buildFrameA(endCurrentComm, (uint16_t)speed, (uint16_t)payloadSize);
      sendFrame(frameA, NS, -1);
      // Serial.print("\n");
    }
    // Serial.println("This test is complete");
    first_transmission = true;
  }
  // Envío trama A (Fin total de la comunicación)
  // Serial.print("\n");
  // Serial.println("Last A transmission");
  frameA = buildFrameA(finalComm, (uint16_t)speed, (uint16_t)payloadSize);
  sendFrame(frameA, 0, -1);
  // Serial.println("\nDone testing");
  while (true)
  {
  }
}
