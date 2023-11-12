#include <stdint.h>

// RS485
int a;
int b;
int TX1 = 10;
int TX2 = 11;
int RX1 = 12;
int RX2 = 13;
const int readModePin = 9;
int dataA;
int dataB;

int receivedDataSize;
int receivedData[40];
int receivedSave[40];
int dataIndex = 0;
bool receivingFlag = false;

int dataSize;
int flagSize;
int frameSize;
int addressSize;
int iFrameSize;

int addressPri[] = {1, 1, 1, 1, 1, 1, 1, 1};

int addressSec1[] = {1, 1, 1, 1, 1, 1, 1, 0};
int flag[] = {0, 0, 0, 0, 0, 1, 0, 1};
int addressSec2[] = {1, 1, 1, 1, 1, 1, 0, 1};

int data[] = {1, 1, 1, 1, 1, 1, 1, 1};

int iFrame[] = {0, 0, 0, 0, 1, 0, 0, 0};
const int nsPosition = 1;
const int nrPosition = 5;
const int sequenceSize = 3;
const int poolFinalPosition = 4;

const int seedingDelay = 200;
const int receivingsDelay = 199;


// Criado a variavel frame com o respectivo tamanho
int frame[40];

void setup()
{
  pinMode(RX1, INPUT);  // O RX no pino 12 Entrada
  pinMode(RX2, INPUT);  // O RX no pino 13 Entrada
  pinMode(TX1, OUTPUT); // O RX no pino 10 Entrada
  pinMode(TX2, OUTPUT); // O RX no pino 11 Entrada
  pinMode(readModePin, INPUT);
  Serial.begin(9600); // Inicializa a comunicação serial (opcional, para depuração)
  
  digitalWrite(TX1, 0);
  digitalWrite(TX2, 0);

  dataSize = sizeof(data) / sizeof(data[0]);
  frameSize = sizeof(frame) / sizeof(frame[0]);
  flagSize = sizeof(flag) / sizeof(flag[0]);
  addressSize = sizeof(addressSec2) / sizeof(addressSec2[0]);
  iFrameSize = sizeof(iFrame) / sizeof(iFrame[0]);
  receivedDataSize = sizeof(receivedData) / sizeof(receivedData[0]);

}

void loop()
{
  if(receiveData() == true) {
    sendConfirmation();
  }  
}

bool receiveData()
{
  if(receive() == false)
    return false;

  incrementNRBits();
  
  return true;
}

void makeFrame(const int *flag, const int *addressReceiver, const int *data)
{
  Serial.println("makeFrame");
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

void sendFrame()
{
  Serial.println("sendFrame");
  a=0;
  b=0;
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
      continue;
    }
    Serial.print(receivedData[i]);
  }
  Serial.println();
}

bool receive()
{
  readingData();

  for (int i = 0; i < receivedDataSize; i++) {
      	Serial.print(receivedData[i]);
  }
  Serial.println();
  
  if (verifyAddres(receivedData) == false)
  {
    
    Serial.println("Address fail");
    //zera byte de controle
    for (int i = 0; i < 8; i++)
  	{
    	iFrame[i] = 0;
  	}
    iFrame[4]=1;
    return false;
  }

  if (verifyFlag(receivedData) == false)
  {
    Serial.println("Flag fail");
    return false;
  }

  if (dataNSEqualToLocalNR(receivedData) == false)
  {
    Serial.println("NS NR fail");
    return false;
  }
  
  Serial.println("Confirmed receive");
  
  return true;
}

void sendConfirmation()
{
  Serial.println("sendConfirmation");

  makeFrame(flag, addressPri, data);

  delay(0);
  
  sendFrame();
}

bool verifyAddres(int *receivedData)
{
  // Verificando se o endereço é valido
  bool Test_address = true;
  for (int i = 0; i < 8; i++)
  {
    if (addressSec1[i] != receivedData[i + 8])
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

/**
 * Bits de confirmação
 */
void incrementNRBits()
{
  Serial.println("incrementNRBits");

  for (int i = 0; i < iFrameSize; i++) {
      	Serial.print(iFrame[i]);
  }
  Serial.println();

  incrementBinary(iFrame, nrPosition, nrPosition + 2);

  for (int i = 0; i < iFrameSize; i++) {
      	Serial.print(iFrame[i]);
  }

  Serial.println();
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

bool dataNSEqualToLocalNR(int *receivedData)
{
  int nrLimit = nrPosition + sequenceSize;
  int receivedDataNSPostion = flagSize+addressSize+1;
  
  //Serial.println("Iframe:");
  //for(int i = 0; i < iFrameSize; i++)
  //{
  //  Serial.println(iFrame[i]);
  //}

  Serial.println("Iframe:");
  for (int i = nrPosition; i < nrLimit; i++)
  {
    Serial.print(iFrame[i]);
    Serial.print(receivedData[receivedDataNSPostion]);
    if (iFrame[i] == receivedData[receivedDataNSPostion])
    {
      receivedDataNSPostion++;
      continue;
    }
    else
    {
      return false;
    }
  }
  return true;
}