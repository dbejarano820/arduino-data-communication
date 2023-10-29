#include <SoftwareSerial.h>
#include <LiquidCrystal.h>
#define MAX_FRAME_SIZE 205
SoftwareSerial mySerial(10, 11);  // RX, TX
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);


const uint8_t DEFAULT_FLAG = 0x7E;  // 01111110 en binario
enum : uint8_t {
  initialHandshake = 0b000,
  infoFrame = 0b001,
  endCurrentComm = 0b010,
  nonInitialHandshake = 0b011,
  finalComm = 0b100,
  ack = 0b101
};
static int totalComms = 0;
static int failedComms = 0;
static int BER = 0;
int speed = 300;
uint8_t buffer[MAX_FRAME_SIZE];
uint8_t frameIndex = 0;
bool readingFrame = false;

void processFrameA(uint8_t *trama);
void processFrameB(uint8_t *trama);
void acknowledgeFrame(uint8_t NS);


void setup() {
  Serial.begin(speed);  // Configura la velocidad de transmisión inicial (modificar según las pruebas)
  mySerial.begin(speed);
  lcd.begin(16, 2);  // inicializa el LCD
  lcd.print("Iniciando prueba...");
}

void loop() {
  uint8_t *frameA;
  uint8_t *frameB;
  while (Serial.available() > 0) {
    lcd.clear();
    lcd.print("Prueba en curso!");
    uint8_t byte = Serial.read();
    if (byte == DEFAULT_FLAG) {
      if (readingFrame) {
        // Nueva bandera de entrada, fin de trama
        readingFrame = false;
        if (frameIndex == 7) { processFrameA(buffer, frameIndex); }
        if (frameIndex == 203 || frameIndex == 103) { processFrameB(buffer, frameIndex); }
        // Procesar trama
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
  }
}

bool isValidFrame(uint8_t frame[MAX_FRAME_SIZE], uint16_t crc_high, uint16_t crc_low, int frameIndex) {
  // Valida la trama haciendo la comparacion de CRC
  Serial.print("\ncrc_high =");
  Serial.println(crc_high);
  Serial.print("\ncrc_low =");
  Serial.println(crc_low);
  if (frameIndex == 7) {
    // Serial.print("\nframe[5] =");
    // Serial.println(frame[5]);
    // Serial.print("\nframe[6] =");
    // Serial.println(frame[6]);
    if (frame[5] == crc_high && frame[6] == crc_low) {
      Serial.println("Yes, is Valid");
      return true;
    } else {
      Serial.println("Nope!");
      return false;
    }
  } else if (frameIndex == 103) {
    // Serial.print("\nframe[101] =");
    // Serial.println(frame[101]);
    // Serial.print("\nframe[102] =");
    // Serial.println(frame[102]);
    if (frame[101] == crc_high && frame[102] == crc_low) {
      Serial.println("Yes, is Valid");
      return true;
    } else {
      Serial.println("Nope!");
      return false;
    }
  } else if (frameIndex == 203) {
    // Serial.print("\nframe[201] =");
    // Serial.println(frame[201]);
    // Serial.print("\nframe[202] =");
    // Serial.println(frame[202]);
    if (frame[201] == crc_high && frame[202] == crc_low) {
      Serial.println("Yes, is Valid");
      return true;
    } else {
      Serial.println("Nope!");
      return false;
    }
  }
}

void processFrameA(uint8_t frame[MAX_FRAME_SIZE], int frameIndex) {
  // Procesa la trama de tipo A (inicio/finalización)
  // Extrae y procesa la velocidad, tamaño de datos, etc.
  uint16_t crc = calculateCRC(frame, 5);
  uint16_t crc_high = crc >> 8;   // byte alto de CRC
  uint16_t crc_low = crc & 0xFF;  // byte bajo de CRC
  uint8_t type = frame[0];
  bool ValidateFrame = isValidFrame(frame, crc_high, crc_low, frameIndex);
  totalComms += 1;

  if (!ValidateFrame) {
    failedComms += 1;
  } else {
    acknowledgeFrame(0);
  }
  lcd.clear();
  lcd.print("Trama A recibida");
  if (type == finalComm) {
    BER = (failedComms * 100) / totalComms;
    lcd.clear();
    lcd.print("Prueba Finalizada!");
    delay(1000);
    lcd.setCursor(0, 1);
    lcd.print("BER: ");  // Mostrar el BER calculado
    lcd.print(BER);
    lcd.print("%");
    Serial.print("BER: ");
    Serial.print(BER);
    Serial.print("%\n");
    delay(2000);
  }
}

void processFrameB(uint8_t frame[MAX_FRAME_SIZE], int frameIndex) {
  uint8_t type = frame[0];
  uint16_t crc;
  uint16_t crc_high;  // byte alto de CRC
  uint16_t crc_low;   // byte bajo de CRC
  bool ValidateFrame;
  Serial.println("Frame B");
  if (frameIndex == 103) {
    uint16_t crc = calculateCRC(frame, 101);
    uint16_t crc_high = crc >> 8;   // byte alto de CRC
    uint16_t crc_low = crc & 0xFF;  // byte bajo de CRC
    ValidateFrame = isValidFrame(frame, crc_high, crc_low, frameIndex);
  }
  if (frameIndex == 203) {
    // Procesa la trama de tipo B (información)
    Serial.println("203 index");
    uint16_t crc = calculateCRC(frame, 201);

    uint16_t crc_high = crc >> 8;   // byte alto de CRC
    uint16_t crc_low = crc & 0xFF;  // byte bajo de CRC
    ValidateFrame = isValidFrame(frame, crc_high, crc_low, frameIndex);
  }
  totalComms += 1;

  if (!ValidateFrame) {
    failedComms += 1;
  } else {
    acknowledgeFrame(type);
  }

  // Por ahora, solo muestra un mensaje en el LCD
  lcd.clear();
  lcd.print("Trama B recibida");
}

void acknowledgeFrame(uint8_t NS) {
  uint8_t frameACK[3] = { DEFAULT_FLAG, (NS << 3) | 0x05, DEFAULT_FLAG };  // Construye la trama ACK
  Serial.write(frameACK, sizeof(frameACK));
}

//Calculamos en CRC desde el Receptor
uint16_t calculateCRC(uint8_t data[MAX_FRAME_SIZE], size_t longitud) {
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
