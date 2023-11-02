// C++ code
//
#include <stdint.h>

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

// Criado a variavel frame com o respectivo tamanho
int frame[32];

void setup()
{
  Serial.begin(9600);
  pinMode(pinBit1, INPUT);
  pinMode(pinBit2, INPUT);

  pinMode(RX1, INPUT);  // SAIDA PARA COMUNICAÇÃO
  pinMode(TX1, OUTPUT); // SAIDA PARA COMUNICAÇÃO

  pinMode(RX2, INPUT);  // SAIDA PARA COMUNICAÇÃO
  pinMode(TX2, OUTPUT); // SAIDA PARA COMUNICAÇÃO
}

void loop()
{
  dataSize = sizeof(data1) / sizeof(data1[0]);
  frameSize = sizeof(frame) / sizeof(frame[0]);
  flagSize = sizeof(flag1) / sizeof(flag1[0]);
  addressSize = sizeof(addressPri) / sizeof(addressPri[0]);

  key1 = digitalRead(pinBit1);
  key2 = digitalRead(pinBit2);

  // CHAVES OFF
  if (key2 == 0 && key1 == 0)
  {
    Serial.println("Sistema off_line");
    a = 0;
    b = 0;
    digitalWrite(TX1, a);
    digitalWrite(TX2, b);
  }

  // CHAVES ATIVA SEC1
  else if (key2 == 0 && key1 == 1)
  {
    makeFrame(flag1, addressSec1, data1);
    sendDataToS1();
  }

  // CHAVES ATIVA SEC2
  else if (key2 == 1 && key1 == 0)
  {
    makeFrame(flag2, addressSec2, data2);
    sendDataToS2();
  }

  // CHAVES ATIVA SEC1 para SEC2
  else
  {
    s1SendDataToS2();
  }
}

// Receptor1
void sendDataToS1()
{
  // envio do frame MOD RS485
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
    // Serial.print("(");
    // Serial.print(a);
    // Serial.print(".");
    // Serial.print(b);
    // Serial.print(")");
  }
  Serial.println();
}

// Receptor2
void sendDataToS2()
{
  // envio do frame MOD RS485
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
    // Serial.print("(");
    // Serial.print(a);
    // Serial.print(".");
    // Serial.print(b);
    // Serial.print(")");
  }
  Serial.println();
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

void makeFrame(const int *flag, const int *addressReceiver, const int *data)
{
  for (int i = 0; i < flagSize; i++)
  {
    frame[i] = flag[i];
  }

  for (int i = 0; i < addressSize; i++)
  {
    frame[(flagSize + i)] = addressReceiver[i];
  }

  for (int i = 0; i < dataSize; i++)
  {
    frame[(flagSize + addressSize + i)] = data[i];
  }

  for (int i = 0; i < flagSize; i++)
  {
    frame[(flagSize + addressSize + dataSize + i)] = flag[i];
  }
}