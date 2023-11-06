#include <stdint.h>

//RS485
int a;
int b;
int TX1 = 10;
int TX2 = 11;
int RX1 = 12;
int RX2 = 13;

int dataA;
int dataB;

int receivedDataSize;
int receivedData[32];
int receivedSave[32];
int dataIndex = 0;
bool receivingFlag = false;

int addressSec2[]={1,1,1,1,1,1,0,1};

void setup() {
    Serial.begin(9600);
  
  	pinMode(RX1, INPUT);//SAIDA PARA COMUNICAÇÃO
  	pinMode(TX1, OUTPUT);//SAIDA PARA COMUNICAÇÃO
  	pinMode(RX2, INPUT);//SAIDA PARA COMUNICAÇÃO
  	pinMode(TX2, OUTPUT);//SAIDA PARA COMUNICAÇÃO
}

void loop() {
  receivedDataSize = sizeof(receivedData)/sizeof(receivedData[0]);
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
    if (addressSec2[i] != receivedData[i+8]) {
      Test_address = false;
      break; // Se um elemento for diferente, nao ha necessidade de verificar os outros
    }
  }
  //Se o endereco nao for valido, informa que nao eh valido e imprime os dados anteriores
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