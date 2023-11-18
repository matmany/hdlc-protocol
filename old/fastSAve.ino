// C++ code
//
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
int iFrameSize;
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
int addressOff[] = {0,0,0,0,0,0,0,0};

// Bits de Controle
int iFrame[] = {0, 0, 0, 0, 0, 0, 0, 0};
const int nsPosition = 1;
const int nrPosition = 5;
const int sequenceSize = 3;
const int poolFinalPosition = 4;

// Criado a variavel frame com o respectivo tamanho
int frame[40];
int backUpFrame[40];

const int seedingDelay = 200;
const int receivingsDelay = 199;

int receivedDataSize;
int receivedData[40];
int receivedSave[40];

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
  iFrameSize = sizeof(iFrame) / sizeof(iFrame[0]);
  receivedDataSize = sizeof(receivedData) / sizeof(receivedData[0]);
  ackPosition = flagSize + addressSize;
}

void loop()
{

  key1 = digitalRead(pinBit1);
  key2 = digitalRead(pinBit2);

  // CHAVES OFF
  if (key2 == 0 && key1 == 0)
  {
    systemOffLineStatus(flag1, data1);
  }

  // CHAVES ATIVA SEC1
  else if (key2 == 0 && key1 == 1)
  {
    if (canSend)
    {
      sendingMoment(flag1, addressSec1, data1);
    }
    else
      receiveConfirmation();
  }

  // CHAVES ATIVA SEC2
  else if (key2 == 1 && key1 == 0)
  {
    if (canSend)
    {
      sendingMoment(flag2, addressSec2, data2);
    }
    else
      receiveConfirmation();
  }

  // CHAVES ATIVA SEC1 para SEC2
  else
  {
    s1SendDataToS2();
  }
}

void s1SendDataToS2()
{
  for (int i = 0; i < 8; i++)
  {
    iFrame[i] = 0;
  }
  Serial.println("Ainda sem Informacao");
  a = 0;
  b = 0;
  digitalWrite(TX1, a);
  digitalWrite(TX2, b);
  delay(1000);

  canSend = true;
}

void primarySendFrame()
{
  a = 0;
  b = 0;
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
    delay(seedingDelay);
  }

  a = 0;
  b = 0;
  digitalWrite(TX1, a);
  digitalWrite(TX2, b);
  Serial.println();
}

void makeFrame(const int *flag, const int *addressReceiver, const int *data)
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

  for (int i = 0; i < iFrameSize; i++)
  {
    frame[frameLastPosition] = iFrame[i];
    frameLastPosition++;
  }

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

void systemOffLineStatus(
  int *flag,
  int *data
)
{
  for (int i = 0; i < 8; i++)
  {
    iFrame[i] = 0;
  }
  Serial.println("Sistema off_line");

  sendingReset(flag, data);
  canSend = true;
  
  a = 0;
  b = 0;
  digitalWrite(TX1, a);
  digitalWrite(TX2, b);
  delay(90);
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
  Serial.println("purge");
  for (int i = 0; i < frameSize; i++)
  {
    backUpFrame[i] = 0;
  }
}

void sendingMoment(
    const int *flag,
    const int *addressReceiver,
    const int *data)
{
  Serial.println("Enviando...");
  makeFrame(flag, addressReceiver, data);

  storeFrame();

  primarySendFrame();

  // startTimer()

  incrementNSBits();

  canSend = false;
}

void readingData()
{
  int dataA;
  int dataB;
  Serial.print("void reading data");
  Serial.println();

  for (int i = 0; i < receivedDataSize; i++)
  {
    delay(receivingsDelay);
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
    Serial.print(receivedData[i]);
  }
  Serial.println();
}

void receiveConfirmation()
{

  Serial.println("receiveConfirmation");

  readingData();

  Serial.println("Recebido:");

  for (int i = 0; i < receivedDataSize; i++)
  {
    Serial.print(receivedData[i]);
  }

  if (verifyAddres(receivedData) == false)
  {
    Serial.println("Address fail");
    return;
  }

  if (verifyFlag(receivedData) == false)
  {
    Serial.println("Flag fail");
    return;
  }

  if (nSEqualToNR(receivedData) == false)
  {
    Serial.println("NS NR fail");
    return;
  }

  purgeFrame();

  incrementNRBits();

  Serial.println("CanSend");
  canSend = true;

  delay(90);
}

void incrementBinary(int binary[], int start, int size)
{
  int carry = 1; // Inicialmente, um carry de 1 para adicionar
  for (int i = size; i >= start; i--)
  {
    if (iFrame[i] == 0)
    {
      iFrame[i] = 1;
      carry = 0; // Não há carry após a adição
      break;
    }
    else
    {
      iFrame[i] = 0;
    }
  }
}

/**
 * Bits de contagem de envios do primário
 */
void incrementNSBits()
{
  incrementBinary(iFrame, nsPosition, nsPosition + 2);
}

/**
 * Bits de confirmação
 */
void incrementNRBits()
{
  incrementBinary(iFrame, nrPosition, nrPosition + 2);
}

bool verifyAddres(int *receivedData)
{
  // Verificando se o endereço é valido
  bool Test_address = true;
  for (int i = 0; i < 8; i++)
  {
    if (addressPri[i] != receivedData[i + 8])
    {
      Test_address = false;
      break; // Se um elemento for diferente, nao ha necessidade de verificar os outros
    }
  }

  return Test_address;
}

bool verifyFlag(int *receivedData)
{
  // Comparando as flags iniciais e finais
  bool FlagInOut = true;
  for (int i = 0; i < 8; i++)
  {
    if (receivedData[i] != receivedData[frameSize - (8 - i)])
    {
      FlagInOut = false;
      break; // Se um elemento for diferente, não há necessidade de verificar os outros
    }
  }

  return FlagInOut;
}

bool nSEqualToNR(int *receivedData)
{
  int nsLimit = nsPosition + sequenceSize;
  // int receivedDataNRPostion = 21;
  int receivedDataNRPostion = flagSize + addressSize + iFrameSize - 3;
  for (int i = nsPosition; i < nsLimit; i++)
  {
    if (iFrame[i] == receivedData[receivedDataNRPostion])
    {
      receivedDataNRPostion++;
      continue;
    }
    else
    {
      return false;
    }
  }
  return true;
}

void sendingReset(
  const int *flag,
  const int *data
) {
  makeFrame(flag, addressOff, data);
  primarySendFrame();
}