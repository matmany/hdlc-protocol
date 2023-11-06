#include <stdint.h>

//RS485
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
int receivedData[32];
int receivedSave[32];
int dataIndex = 0;
bool receivingFlag = false;

int dataSize;
int flagSize;
int frameSize;
int addressSize;

int addressSec1[]={1,1,1,1,1,1,1,0};
int flag[] = {0, 0, 0, 0, 0, 1, 0, 1};
int addressSec2[] = {1, 1, 1, 1, 1, 1, 0, 1};

int data[] = {1, 1, 1, 1, 1, 1, 1, 1};

// Criado a variavel frame com o respectivo tamanho
int frame[32];

void setup() {
  pinMode(RX1, INPUT); // O RX no pino 12 Entrada
  pinMode(RX2, INPUT); // O RX no pino 13 Entrada
  pinMode(TX1, OUTPUT); // O RX no pino 10 Entrada
  pinMode(TX2, OUTPUT); // O RX no pino 11 Entrada
  pinMode(readModePin, INPUT);
  Serial.begin(9600); // Inicializa a comunicação serial (opcional, para depuração)
}

void loop() {

  dataSize = sizeof(data) / sizeof(data[0]);
  frameSize = sizeof(frame) / sizeof(frame[0]);
  flagSize = sizeof(flag) / sizeof(flag[0]);
  addressSize = sizeof(addressSec2) / sizeof(addressSec2[0]);

  receivedDataSize = sizeof(receivedData)/sizeof(receivedData[0]);
  
  int readMode = digitalRead(readModePin);

  if(readMode == 1) {
    readMode();
  }
  else {
    writeMode();
  }
}

void readMode()
{
      //Recebimento do Frame MOD RS485
  for (int i = 0; i < receivedDataSize; i++) {
		delay(999);
		dataA = 0;
		dataB = 0;
		dataA = digitalRead(RX1);
		dataB = digitalRead(RX2);
	
		if (dataA==1 && dataB==0){
			receivedData[i] = 1;
		}
		else if(dataA==0 && dataB==1){
			receivedData[i] = 0;
		}
		else{
			i=-1;
		}
	}
  
  //Verificando se o endereço é valido
  bool Test_address = true;
 
  for (int i = 0; i < 8; i++) {
		if (addressSec1[i] != receivedData[i+8]) {
		Test_address = false;
		break; // Se um elemento for diferente, não há necessidade de verificar os outros
		}
	}
  //Se o endereco nao for valido, informa que não é valido e imprime os dados anteriores
  if (Test_address == false){
		Serial.println("Endereco nao eh do Dispositivo");
		Serial.print("Ainda continua:");
		for (int i = 0; i < receivedDataSize; i++) {
			Serial.print(receivedSave[i]);
		}
		Serial.println();
	}
  //Se for valido, informe e continue....
  else{
		Serial.println("Endereco Valido");
		
		//Comparando as flags iniciais e finais
		bool FlagInOut = true;
		for (int i = 0; i < 8; i++) {
			if (receivedData[i] != receivedData[receivedDataSize-(8-i)]) {
				FlagInOut = false;
				break; // Se um elemento for diferente, não há necessidade de verificar os outros
			}
		}
		
		//Se os frames forem iguais - Armazena o Frame e Printa os dados novos
		if (FlagInOut == true){
			for (int i = 0; i < receivedDataSize; i++) {
				receivedSave[i]=receivedData[i];
				Serial.print(receivedSave[i]);
			}
			Serial.println();
		}
		//Se os frames forem divergentes Printa erro e informa o dado antigo
		else{
			Serial.println("Frame não recebido, Flag divergentes");
			Serial.print("Ainda continua:");
			for (int i = 0; i < receivedDataSize; i++) {
				Serial.println(receivedSave[i]);
			}
		}
	}
}

void writeMode()
{
    makeFrame(flag, addressSec2, data);
    sendFrame();
}

void readBitRS485()
{
      for (int i = 0; i < receivedDataSize; i++) {
		delay(999);
		dataA = 0;
		dataB = 0;
		dataA = digitalRead(RX1);
		dataB = digitalRead(RX2);
	
		if (dataA==1 && dataB==0){
			receivedData[i] = 1;
		}
		else if(dataA==0 && dataB==1){
			receivedData[i] = 0;
		}
		else{
			i=-1;
		}
	}
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

void sendFrame()
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

