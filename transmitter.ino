#include <SoftwareSerial.h>
#include <LiquidCrystal.h>
#include <stdint.h>

using namespace std;

SoftwareSerial mySerial(10, 11); // RX, TX
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

const uint8_t DEFAULT_FLAG = 0x7E; // 01111110 en binario
const uint8_t ESCAPE_FLAG = 0xAA;  // Para byte stuffing
const uint16_t speed = 300;
const uint16_t payloadSize = 100;
static const size_t frameASize = 9;
static const size_t frameBSize = 205;
bool first_transmission = true;
uint8_t data[payloadSize];
enum : uint8_t
{
  initialHandshake = 0b000,
  infoFrame = 0b001,
  endCurrentComm = 0b010,
  nonInitialHandshake = 0b011,
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
    // Serial.print(data[i]);
    // Serial.print(" : ");
    // Serial.println(crc);
    // Serial.print("\n");
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
  // Serial.println("End");
  // Serial.println(crc);
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

  uint16_t crc = calculateCRC(frameB + 1, 201); // Calcula el CRC de los datos en la trama (sin incluir banderas)
  frameB[202] = crc >> 8;                       // byte alto de CRC
  frameB[203] = crc & 0xFF;                     // byte bajo de CRC
  frameB[204] = DEFAULT_FLAG;
  // Serial.println("\n");
  return frameB;
}

uint8_t *buildSmFrameB(uint8_t NS, uint8_t type, uint8_t *information)
{
  static uint8_t frameBsm[105];

  frameBsm[0] = DEFAULT_FLAG;
  frameBsm[1] = (NS << 3) | type;         // Desplaza NS 3 bits a la izquierda y OR con tipo
  memcpy(frameBsm + 2, information, 100); // Copia 100 bytes de información

  uint16_t crc = calculateCRC(frameBsm + 1, 101); // Calcula el CRC de los datos en la trama (sin incluir banderas)

  frameBsm[102] = crc >> 8;   // byte alto de CRC
  frameBsm[103] = crc & 0xFF; // byte bajo de CRC
  frameBsm[104] = DEFAULT_FLAG;

  return frameBsm;
}

void sendFrame(uint8_t *frame, uint8_t NS, int frameType)
{
  bool isAcked = false;
  while (true)
  {
    if (frameType == -1)
    {
      for (int i = 0; i < frameASize; i++)
      {
        Serial.write(frame[i]);
        delay(10);
      }
      isAcked = acked(NS);
      if (isAcked)
      {
        break;
      }
      else
      {
        delay(1000);
      }
    }
    else if (frameType == 0)
    {
      for (int i = 0; i < frameBSize; i++)
      {
        Serial.write(frame[i]);
        delay(10);
      }
      isAcked = acked(NS);
      if (isAcked)
      {
        break;
      }
      else
      {
        delay(1000);
      }
    }
  }
}

bool acked(uint8_t NS)
{
  uint8_t frame[2];
  uint8_t recNS;
  bool readingFrame = false;
  while (Serial.available() > 0)
  {

    uint8_t byte = Serial.read();
    if (byte == DEFAULT_FLAG)
    {
      Serial.readBytes(frame, 2);

      if (frame[1] == DEFAULT_FLAG)
      {
        return true;
      }
      else
      {
        return false;
      }
    }
  }
}

void setup()
{
  lcd.begin(16, 2);
  Serial.begin(speed); // Configura la velocidad de transmisión inicial (modificar según las pruebas)
  mySerial.begin(speed);
  randomSeed(1);
}

void loop()
{
  // delay(5000); //Para conectar la jugada
  uint8_t *frameA;
  uint8_t *frameB;
  uint8_t NS = random(256);
  for (int i = 0; i < payloadSize; i++)
  {
    int num = random(1, 256);
    if (num == 126)
    {
      num++;
    }
    data[i] = num;
  }
  for (int i = 0; i < numberOfTests; i++)
  {
    int bytesCopied = 0;
    int copySize = 200;

    frameA = buildFrameA(initialHandshake, (uint16_t)speed, (uint16_t)payloadSize);
    sendFrame(frameA, 0, -1);
    while (bytesCopied < payloadSize)
    {
      if (!first_transmission)
      {
        frameA = buildFrameA(nonInitialHandshake, (uint16_t)speed, (uint16_t)payloadSize);
        sendFrame(frameA, 0, -1);
      }
      for (int i = 0; i < payloadSize; i++)
      {
        int num = random(1, 256);
        if (num == 126)
        {
          num++;
        }
        data[i] = num;
      }
      uint8_t NS = random(256);

      if (payloadSize >= 200)
      {
        frameB = buildFrameB(NS, infoFrame, data + bytesCopied);
        sendFrame(frameB, NS, 0);
        bytesCopied += copySize;
      }
      else
      {
        frameB = buildSmFrameB(NS, infoFrame, data);
        sendFrame(frameB, NS, 0);
        bytesCopied += copySize;
      }

      frameA = buildFrameA(endCurrentComm, (uint16_t)speed, (uint16_t)payloadSize);
      sendFrame(frameA, 0, -1);

      first_transmission = false;
    }
    frameA = buildFrameA(finalComm, (uint16_t)speed, (uint16_t)payloadSize);
    sendFrame(frameA, 0, -1);
  }
  while (true)
  {
  }
}