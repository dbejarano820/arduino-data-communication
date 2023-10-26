#include <SoftwareSerial.h>
#include <LiquidCrystal.h>
#define MAX_FRAME_SIZE 205
SoftwareSerial mySerial(10, 11);  // RX, TX
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

const uint8_t DEFAULT_FLAG = 0x7E;  // 01111110 en binario

void processFrameA(uint8_t *trama);
void processFrameB(uint8_t *trama);
void acknowledgeFrame(uint8_t NS);

int speed = 300;
uint8_t buffer[MAX_FRAME_SIZE];
uint8_t frameIndex = 0;
bool readingFrame = false;

void setup() {
  Serial.begin(speed);  // Configura la velocidad de transmisión inicial (modificar según las pruebas)
  mySerial.begin(speed);
  lcd.begin(16, 2);  // inicializa el LCD
  lcd.print("Iniciando prueba...");
}

void loop() {
  while (Serial.available() > 0) {
    lcd.clear();
    lcd.print("Prueba en curso!");
    uint8_t byte = Serial.read();
    if (byte == DEFAULT_FLAG) {
      if (readingFrame) {
        // Nueva bandera de entrada, fin de trama
        readingFrame = false;
        //if (frameIndex == 7){processFrameA(buffer[MAX_FRAME_SIZE]);}
        //if (frameIndex == 203){processFrameB(buffer[MAX_FRAME_SIZE]);}
        Serial.print("\nframeIndex =");
        Serial.println(frameIndex);
        // Procesar trama
        // Serial.println(DEFAULT_FLAG);
        // for (int i = 0; i < frameIndex; i++)
        // {
        //   Serial.print(buffer[i]);
        //   Serial.println("");
        // }
        // Serial.println(DEFAULT_FLAG);
        frameIndex = 0;
      } else {
        readingFrame = true;
      }
    } else {
      if (readingFrame && frameIndex < MAX_FRAME_SIZE) {
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

bool isValidFrame(uint8_t frame[MAX_FRAME_SIZE], uint16_t crc_high, uint16_t crc_low) {
  // Valida la trama haciendo la comparacion de CRC
  Serial.print("\ncrc_high =");
  Serial.println(crc_high);
  Serial.print("\ncrc_low =");
  Serial.println(crc_low);
  Serial.print("\nframe[5] =");
  Serial.println(frame[5]);
  Serial.print("\nframe[6] =");
  Serial.println(frame[6]);
  if (frame[5] == crc_high && frame[6] == crc_low) {
    Serial.println("Yes, is Valid");
    return true;
  } else {
    Serial.println("Nope!");
    return false;
  }
}

void processFrameA(uint8_t frame[MAX_FRAME_SIZE]) {
  // Procesa la trama de tipo A (inicio/finalización)
  // Extrae y procesa la velocidad, tamaño de datos, etc.

  uint16_t crc = calculateCRC(frame + 1, 5);
  uint16_t crc_high = crc >> 8;   // byte alto de CRC
  uint16_t crc_low = crc & 0xFF;  // byte bajo de CRC
  bool ValidateFrame = isValidFrame(frame, crc_high, crc_low);

  // Por ahora, solo muestra un mensaje en el LCD
  lcd.clear();
  lcd.print("Trama A recibida");
}


void processFrameB(uint8_t *frame) {
  // Procesa la trama de tipo B (información)

  // Por ahora, solo muestra un mensaje en el LCD
  lcd.clear();
  lcd.print("Trama B recibida");

  uint8_t NS = frame[1] >> 3;  // Extrae el número de secuencia
  acknowledgeFrame(NS);
}

void acknowledgeFrame(uint8_t NS) {
  uint8_t frameACK[3] = { DEFAULT_FLAG, (NS << 3) | 0x05, DEFAULT_FLAG };  // Construye la trama ACK
  // Serial.write(frameACK, sizeof(frameACK));
}


//Calculamos en CRC desde el Receptor
uint16_t calculateCRC(uint8_t *data, size_t longitud) {
  uint16_t crc = 0xFFFF;  // Valor inicial del CRC

  for (size_t i = 0; i < longitud; ++i) {
    crc ^= data[i];  // Realiza la operación XOR con el byte actual

    // Realiza el cálculo para cada bit del byte
    for (int j = 0; j < 8; ++j) {
      if (crc & 0x0001) {
        crc >>= 1;      // Bit shift
        crc ^= 0xA001;  // Polinomio generador para CRC-16, forma reversa de 0x8005 para adecuarse al XOR y otras operaciones de bits
      } else {
        crc >>= 1;
      }
    }
  }

  return crc;
}
