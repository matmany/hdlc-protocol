#include <stdint.h>

const int rxPin = 10;        // Pino GPIO13 para RX (bit 1)
const int rxPinInverse = 11; // Pino GPIO12 para RX barrado (bit 0)
const int txPin = 12;        // Pino GPIO14 para TX de resposta (bit 1)
const int txPinInverse = 13; // Pino GPIO27 para TX barrado de resposta (bit 0)
int cont = 0;

struct Frame
{
  // Campos para pré definições
  uint8_t addressSec1;
  uint8_t dataSec1;
  uint8_t dataSec2;

  // Campos para recebimento
  uint8_t flagInicioFim;
  uint8_t address;
  uint8_t control;
  uint8_t data;
  uint8_t crc;
  uint8_t flagFim; // Novo campo para o último byte de flag de fim

  // Campos para resposta
  uint8_t FlaginicioPrim;
  uint8_t addressPrim;
  uint8_t controlPrim;
  uint8_t dataPrim;
  uint8_t CRCPrim;
  uint8_t FlagfimPrim;
};

void enviarFrame(const Frame &frameResposta);
uint8_t verificaCrc(Frame frame);

const uint8_t commandoDeEnvioS2 = 0b11111111;

bool aguardandoFlagInicio = false;
bool aguardandoFlagFim = false;
bool recebendoDados = false;
bool enviarResp = false;
Frame frameRecebido;
Frame FrameSec1;
Frame frameResposta;

void setup()
{
  Serial.begin(9600);
  pinMode(rxPin, INPUT);
  pinMode(rxPinInverse, INPUT);
  pinMode(txPin, OUTPUT);
  pinMode(txPinInverse, OUTPUT);
}

void loop()
{
  FrameSec1.addressSec1 = 0b01010101;
  FrameSec1.dataSec1 = 0b00000001;
  FrameSec1.dataSec2 = 0b00001111;

  // Recebe dados
  int a = digitalRead(rxPin);
  int b = digitalRead(rxPinInverse);

  if (b == LOW && a == HIGH)
  {
    // Flag de início/fim
    aguardandoFlagInicio = true;
    aguardandoFlagFim = false;
    recebendoDados = false;
  }

  if (aguardandoFlagInicio)
  {

    // Verifica se está aguardando o início
    if (b == HIGH && a == LOW)
    {
      aguardandoFlagInicio = false;
      aguardandoFlagFim = true;
      recebendoDados = true;
    }
  }

  if (recebendoDados)
  {
    // Recebe os bits e armazena no frame
    frameRecebido.flagInicioFim = receberByte();
    frameRecebido.address = receberByte();
    frameRecebido.control = receberByte();
    frameRecebido.data = receberByte();
    frameRecebido.crc = receberByte();
    frameRecebido.flagFim = receberByte();

    // Verifica se o último byte é o byte de fim
    if (frameRecebido.flagFim == frameRecebido.flagInicioFim)
    {

      // Processa os dados recebidos
      if (
          FrameSec1.addressSec1 == frameRecebido.address &&
          verificaCrc(frameRecebido))
      {
        Serial.println("Recebeu dados do ESP32 Primario");
        processarDados();
        enviarResp = true;
      }
      else
      {
        Serial.println("Endereço nao eh Compativel");
        Serial.println(frameRecebido.address, BIN);
        Serial.println(FrameSec1.addressSec1, BIN);
      }

      // Reinicia as variáveis de controle
      aguardandoFlagInicio = false;
      aguardandoFlagFim = false;
      recebendoDados = false;
    }
    else
    {
      Serial.println("Recebendo - Aguardando Conjunto com Frame Compativel");
    }
  }
  if (enviarResp == true)
  {
    delay(5000);
    respondeDados();
    cont = cont + 1;
    if (cont >= 2)
    {
      cont = 0;
      enviarResp = false;
      digitalWrite(txPin, 0);
      digitalWrite(txPinInverse, 0);
    }
  }
  delay(100);
}

uint8_t receberByte()
{
  uint8_t byteRecebido = 0;
  for (int i = 0; i < 8; i++)
  {
    byteRecebido |= (digitalRead(rxPin) << i);
    delay(100); // Ajuste conforme necessário
  }
  return byteRecebido;
}

void processarDados()
{
  // Implemente aqui a lógica para processar os dados recebidos.
  // Neste exemplo, apenas exibimos os dados no monitor serial.
  Serial.print("Flag de Inicio: ");
  Serial.println(frameRecebido.flagInicioFim, BIN);

  Serial.print("Address: ");
  Serial.println(frameRecebido.address, BIN);

  Serial.print("Control: ");
  Serial.println(frameRecebido.control, BIN);

  Serial.print("Data: ");
  Serial.println(frameRecebido.data, BIN);

  Serial.print("CRC: ");
  Serial.println(frameRecebido.crc, BIN);

  Serial.print("Flag de Fim: ");
  Serial.println(frameRecebido.flagFim, BIN);
  Serial.print("");
}

void respondeDados()
{
  frameResposta.FlaginicioPrim = 0b11110000;
  frameResposta.addressPrim = 0b11111110;
  frameResposta.controlPrim = ajustarControl(frameRecebido.control);
  frameResposta.dataPrim = escolheData();
  frameResposta.CRCPrim = calcularXOR(frameResposta.addressPrim, frameResposta.controlPrim, frameResposta.dataPrim);
  frameResposta.FlagfimPrim = 0b11110000;
  processarDadosEnvio();
  enviarFrame(frameResposta);
}

uint8_t escolheData()
{
  if(frameRecebido.data == commandoDeEnvioS2)
    return FrameSec1.dataSec2;
  return FrameSec1.dataSec1;
}

uint8_t ajustarControl(uint8_t controlAtual)
{
  // Incrementa o valor nas posições 5, 6 e 7 do controle
  uint8_t controlNovo = (controlAtual >> 4) + (0b00000001); // Incrementa 2^4 (bit 6)
  controlNovo = (0b01110000 & controlAtual) | (controlNovo);
  controlNovo = controlNovo | 0b00001000;
  controlNovo = controlNovo & 0b01111111;
  return controlNovo;
}

void processarDadosEnvio()
{
  // Implemente aqui a lógica para processar os dados recebidos.
  // Neste exemplo, apenas exibimos os dados no monitor serial.
  Serial.println("Dados para Resposta");
  Serial.println("");
  Serial.print("Flag de Inicio: ");
  Serial.println(frameResposta.FlaginicioPrim, BIN);

  Serial.print("Address Resposta: ");
  Serial.println(frameResposta.addressPrim, BIN);

  Serial.print("Control Resposta: ");
  Serial.println(frameResposta.controlPrim, BIN);

  Serial.print("Data Resposta: ");
  Serial.println(frameResposta.dataPrim, BIN);

  Serial.print("CRC Resposta: ");
  Serial.println(frameResposta.CRCPrim, BIN);

  Serial.print("Flag de Fim Resposta: ");
  Serial.println(frameResposta.FlagfimPrim, BIN);
  Serial.print("");
}

void enviarByte(uint8_t byte)
{
  for (int i = 0; i < 8; i++)
  {
    int c = byte & 0x01;
    int d = !(byte & 0x01);

    digitalWrite(txPin, c);
    digitalWrite(txPinInverse, d); // Envia o complemento do bit

    byte >>= 1; // Desloca para a direita para pegar o próximo bit
    delay(100); // Ajuste conforme necessário
  }
  Serial.println("");
  delay(0);
}

void enviarFrame(const Frame &frameResposta)
{
  // Envia cada campo do frame
  Serial.println("");
  enviarByte(frameResposta.FlaginicioPrim);
  enviarByte(frameResposta.addressPrim);
  enviarByte(frameResposta.controlPrim);
  enviarByte(frameResposta.dataPrim);
  enviarByte(frameResposta.CRCPrim);
  enviarByte(frameResposta.FlagfimPrim);
  Serial.println("");
  Serial.println("Resposta Enviada");
  Serial.println("");
}

uint8_t calcularXOR(uint8_t byte1, uint8_t byte2, uint8_t byte3)
{
  return byte1 ^ byte2 ^ byte3;
}

uint8_t verificaCrc(Frame frame)
{
  uint8_t result = calcularXOR(frame.address, frame.control, frame.data);

  if (result == frame.crc)
    return true;

  return false;
}