// C++ code
//
#include <stdint.h>

bool sn = false;
bool canSend = true;

const int pinBit1 = 2;
const int pinBit2 = 3;
int key1 = 0;
int key2 = 0;

// RS485
int a;
int b;
int TX1 = 12;
int TX2 = 13;
int RX1 = 10;
int RX2 = 11;

// Variavel para armazenar tamanho dos vetores
int dataSize;
int flagSize;
int frameSize;
int addressSize;
int ackPosition;

// Dados para emissão
int data1[] = {1, 1, 1, 1, 1, 1, 1, 1};
int data2[] = {1, 1, 1, 0, 0, 0, 1, 1};
int data3[] = {1, 1, 1, 1, 1, 1, 1, 1};
int data4[8];

// Flags padrões
int flag1[] = {0, 0, 0, 0, 0, 1, 0, 1};
int flag2[] = {0, 0, 0, 0, 0, 0, 1, 0};
int flag3[] = {0, 0, 0, 0, 0, 0, 1, 1};

// Endereços padrões
int addressPri[] = {1, 1, 1, 1, 1, 1, 1, 1};
int addressSec1[] = {1, 1, 1, 1, 1, 1, 1, 0};
int addressSec2[] = {1, 1, 1, 1, 1, 1, 0, 1};


// Bits de Controle
int iFrame[] = {0, 1,0,1, 0, 0,0,0};
const int nsPosition = 1;
const int nrPosition = 5;
const int sequenceSize = 3;
const int poolFinalPosition = 4;

// Criado a variavel frame com o respectivo tamanho
int frame[33];
int backUpFrame[33];

void setup()
{
  Serial.begin(9600);
  pinMode(pinBit1, INPUT);
  pinMode(pinBit2, INPUT);

  pinMode(RX1, INPUT);  // SAIDA PARA COMUNICAÇÃO
  pinMode(TX1, OUTPUT); // SAIDA PARA COMUNICAÇÃO

  pinMode(RX2, INPUT);  // SAIDA PARA COMUNICAÇÃO
  pinMode(TX2, OUTPUT); // SAIDA PARA COMUNICAÇÃO

  dataSize = sizeof(data1) / sizeof(data1[0]);
  frameSize = sizeof(frame) / sizeof(frame[0]);
  flagSize = sizeof(flag1) / sizeof(flag1[0]);
  addressSize = sizeof(addressPri) / sizeof(addressPri[0]);
  ackPosition = flagSize + addressSize;

  incrementBinary(iFrame, nsPosition, nsPosition+2);
  incrementBinary(iFrame, nrPosition, nrPosition+2);
}

void loop()
{

  key1 = digitalRead(pinBit1);
  key2 = digitalRead(pinBit2);

  // CHAVES OFF
  if (key2 == 0 && key1 == 0)
  {
    systemOffLineStatus();
  }

  // CHAVES ATIVA SEC1
  else if (key2 == 0 && key1 == 1)
  {
    if (canSend)
      int snBinary = static_cast<int>(sn);
    sendingMoment(flag1, addressSec1, data1, sn);
    else receiveConfirmation();
  }

  // CHAVES ATIVA SEC2
  else if (key2 == 1 && key1 == 0)
  {
    makeFrame(flag2, addressSec2, data2, sn);
    primarySendFrame();
  }

  // CHAVES ATIVA SEC1 para SEC2
  else
  {
    s1SendDataToS2();
  }
}

void s1SendDataToS2()
{
  Serial.println("Ainda sem Informação");
  a = 0;
  b = 0;
  digitalWrite(TX1, a);
  digitalWrite(TX2, b);
  delay(1000);
}

void primarySendFrame()
{
  for (int i = 0; i < frameSize; i++)
  {
    Serial.print(frame[i]);
    if (frame[i] == 1)
    {
      a = 1;
      b = 0;
    }
    else if (frame[i] == 0)
    {
      a = 0;
      b = 1;
    }
    else
    {
      a = 0;
      b = 0;
    }
    digitalWrite(TX1, a);
    digitalWrite(TX2, b);
    delay(1000);
  }

  Serial.println();
}

void makeFrame(const int *flag, const int *addressReceiver, const int *data, int snBinary)
{
  int frameLastPosition = 0;
  for (int i = 0; i < flagSize; i++)
  {
    frame[frameLastPosition] = flag[i];
    frameLastPosition++;
  }

  for (int i = 0; i < addressSize; i++)
  {
    frame[frameLastPosition] = addressReceiver[i];
    frameLastPosition++;
  }

  frame[(frameLastPosition)] = snBinary;
  frameLastPosition++;

  for (int i = 0; i < dataSize; i++)
  {
    frame[frameLastPosition] = data[i];
    frameLastPosition++;
  }

  for (int i = 0; i < flagSize; i++)
  {
    frame[frameLastPosition] = flag[i];
    frameLastPosition++;
  }
}

void systemOffLineStatus()
{
  Serial.println("Sistema off_line");
  a = 0;
  b = 0;
  digitalWrite(TX1, a);
  digitalWrite(TX2, b);
}

void storeFrame()
{
  for (int i = 0; i < frameSize; i++)
  {
    backUpFrame[i] = frame[i]; // Copie cada elemento individualmente
  }
}

void purgeFrame()
{
  for (int i = 0; i < frameSize; i++)
  {
    backUpFrame[i] = 0;
  }
}

void sendingMoment(
    const int *flag,
    const int *addressReceiver,
    const int *data,
    int snBinary)
{
  makeFrame(flag, addressReceiver, data, snBinary);

  storeFrame();

  primarySendFrame();

  // startTimer()

  sn = sn + 1;

  canSend = false;
}

void receiveConfirmation()
{
  int *receivedData = readMode();
  int ack = receivedData[ackPosition];

  if (dataIsValid(receivedData) && ack == sn)
  {
    // StopTimer();

    purgeFrame();

    canSend = true;
  }
}

int *readMode()
{
  int dataA;
  int dataB;
  int receivedData[32];
  int receivedDataSize = sizeof(receivedData) / sizeof(receivedData[0]);

  for (int i = 0; i < receivedDataSize; i++)
  {
    delay(999);
    dataA = 0;
    dataB = 0;
    dataA = digitalRead(RX1);
    dataB = digitalRead(RX2);

    if (dataA == 1 && dataB == 0)
    {
      receivedData[i] = 1;
    }
    else if (dataA == 0 && dataB == 1)
    {
      receivedData[i] = 0;
    }
    else
    {
      i = -1;
    }
  }

  return receivedData;
}

bool dataIsValid(int *frame)
{
  return true;
}

void incrementBinary(int binary[], int start, int size)
{
  int carry = 1; // Inicialmente, um carry de 1 para adicionar
  for (int i = size; i >= start; i--)
  {
    if (binary[i] == 0)
    {
      binary[i] = 1;
      carry = 0; // Não há carry após a adição
      break;
    }
    else
    {
      binary[i] = 0;
    }
  }
}