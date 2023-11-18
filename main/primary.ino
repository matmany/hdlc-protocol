#include <stdint.h>

const int txPin = 10;        // Pino GPIO14 para TX (bit 1)
const int txPinInverse = 11; // Pino GPIO27 para TX barrado (bit 0)
const int rxPin = 12;        // Pino GPIO13 para RX de resposta (bit 1)
const int rxPinInverse = 13; // Pino GPIO12 para RX barrado de resposta (bit 0)
int cont = 0;
int timeout = 0;

const int pinBit1 = 2;
const int pinBit2 = 3;
const int pinBit3 = 4;

int key1 = 0;
int key2 = 0;
int key3 = 0;

struct Frame
{
    uint8_t addressPrim;
    uint8_t dataS1S2;

    uint8_t flagInicioFim;
    uint8_t address;
    uint8_t control;
    uint8_t data;
    uint8_t crc;
    uint8_t flagFim; // Novo campo para o último byte de flag de fim

    uint8_t flagInicioRec;
    uint8_t addressRec;
    uint8_t controlRec;
    uint8_t dataRec;
    uint8_t crcRec;
    uint8_t flagFimRec;
};

void enviarFrame(const Frame &frame);
uint8_t verificaCrc(Frame frame);

const uint8_t dataCOMS1S2 = 0b11111111;


bool aguardandoFlagInicio = false;
bool aguardandoFlagFim = false;
bool recebendoDados = false;
bool enviardados = true;
bool sendToS1 = true;
Frame frameRecebido;
Frame FramePrim;
Frame frame;

void setup()
{
    Serial.begin(9600);

    pinMode(pinBit1, INPUT);
    pinMode(pinBit2, INPUT);
    pinMode(pinBit3, INPUT);

    pinMode(txPin, OUTPUT);
    pinMode(txPinInverse, OUTPUT);
    pinMode(rxPin, INPUT);
    pinMode(rxPinInverse, INPUT);

    FramePrim.addressPrim = 0b11111110;
    FramePrim.dataS1S2 = 0b11111111;

    // Simulação de criação de um frame de dados
    frame.flagInicioFim = 0b11110000;
    frame.address = 0b01010101;
    frame.control = 0b00000000;
    frame.data = 0b00110011;
    frame.crc = 0b11110000;
    frame.flagFim = frame.flagInicioFim;
}

void loop()
{
    // Envia o frame
    key1 = digitalRead(pinBit1);
    key2 = digitalRead(pinBit2);
    key3 = digitalRead(pinBit3);

    if (key1 == 1 & key2 == 0)
    {
        frame.address = 0b01010101;
    }
    else if (key1 == 0 & key2 == 1)
    {
        frame.address = 0b00011100;
    }
    else if (key1 == 0 & key2 == 0 & key3 == 1)
    {
        if(sendToS1 == true) {
            frame.address = 0b01010101;
            frame.data = dataCOMS1S2;
        } else {
            frame.address = 0b00011100;
            frame.data = frameRecebido.dataRec;
            sendToS1 = true;
        }

    }

    frame.crc = calcularXOR(frame.address, frame.control, frame.data);

        if (enviardados)
    {
        enviarFrame(frame);
        processarEnviados();
        cont = cont + 1;
        if (cont >= 2)
        {
            enviardados = false;
            cont = 0;
            timeout = 0;
        }
        Serial.println("Contador");
        Serial.println(cont);

        delay(5000); // Aguarda 5 segundos antes de enviar o próximo frame
    }
    else
    {
        timeout = timeout + 1;
        receberResposta();
        if (timeout >= 100)
        {
            enviardados = true;
        }
    }
}

void enviarByte(uint8_t byte)
{
    Serial.println("Enviando Byte");
    for (int i = 0; i < 8; i++)
    {
        int a = byte & 0x01;
        int b = !(byte & 0x01);

        digitalWrite(txPin, a);
        digitalWrite(txPinInverse, b); // Envia o complemento do bit

        byte >>= 1; // Desloca para a direita para pegar o próximo bit
        delay(100); // Ajuste conforme necessário
    }
    Serial.println("");
}

void enviarFrame(const Frame &frame)
{
    // Envia cada campo do frame
    Serial.println("");
    Serial.println("Enviando Frame Primario");
    Serial.println("");
    enviarByte(frame.flagInicioFim);
    Serial.println("Flag inicio enviado");
    enviarByte(frame.address);
    Serial.println("Endereco enviado");
    enviarByte(frame.control);
    Serial.println("Controle enviado");
    enviarByte(frame.data);
    Serial.println("Dados enviado");
    enviarByte(frame.crc);
    Serial.println("CRC enviado");
    enviarByte(frame.flagFim);
    Serial.println("Flag Fim enviado");
    Serial.println("");
    Serial.println("Frame Primario Enviado");
    Serial.println("");
}

void receberResposta()
{
    // Recebe dados
    int c = digitalRead(rxPin);
    int d = digitalRead(rxPinInverse);

    if (d == LOW && c == HIGH)
    {
        // Flag de início/fim
        aguardandoFlagInicio = true;
        aguardandoFlagFim = false;
        recebendoDados = false;
    }

    if (aguardandoFlagInicio)
    {
        // Verifica se está aguardando o início
        if (d == HIGH && c == LOW)
        {
            Serial.print("Recebendo....");
            aguardandoFlagInicio = false;
            aguardandoFlagFim = true;
            recebendoDados = true;
        }
    }
    // Serial.println(recebendoDados);
    if (recebendoDados)
    {
        // Recebe os bits e armazena no frame
        frameRecebido.flagInicioRec = receberByte();
        frameRecebido.addressRec = receberByte();
        frameRecebido.controlRec = receberByte();
        frameRecebido.dataRec = receberByte();
        frameRecebido.crcRec = receberByte();
        frameRecebido.flagFimRec = receberByte();

        // Verifica se o último byte é o byte de fim
        if (frameRecebido.flagInicioRec == frameRecebido.flagFimRec)
        {
            //Serial.println(frameRecebido.flagInicioRec, BIN);
            // Processa os dados recebidos
            if (
                FramePrim.addressPrim == frameRecebido.addressRec &&
                verificaCrc(frameRecebido)
                )
            {
                Serial.println("Recebeu dados do ESP32 Secundario");
                processarDados();
                enviardados = true;
                delay(1000);
            }
            else
            {
                Serial.println("Endereço nao eh Compativel");
                Serial.println(frameRecebido.address, BIN);
                Serial.println(FramePrim.addressPrim, BIN);
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
    Serial.print("Flag de Inicio Recebido: ");
    Serial.println(frameRecebido.flagInicioRec, BIN);

    Serial.print("Address Recebido: ");
    Serial.println(frameRecebido.addressRec, BIN);

    Serial.print("Control Recebido: ");
    Serial.println(frameRecebido.controlRec, BIN);

    Serial.print("Data Recebido: ");
    Serial.println(frameRecebido.dataRec, BIN);

    Serial.print("CRC Recebido: ");
    Serial.println(frameRecebido.crcRec, BIN);

    Serial.print("Flag de Fim Recebido: ");
    Serial.println(frameRecebido.flagFimRec, BIN);
    Serial.print("");

    frame.control = ajustarControl(frameRecebido.controlRec);    
    
    if(frameRecebido.dataRec == 0b00001111)
    {
        sendToS1 = false;
    }
}

void processarEnviados()
{
    // Implemente aqui a lógica para processar os dados recebidos.
    // Neste exemplo, apenas exibimos os dados no monitor serial.
    Serial.print("Flag de Inicio Enviado: ");
    Serial.println(frame.flagInicioFim, BIN);

    Serial.print("Address Enviado: ");
    Serial.println(frame.address, BIN);

    Serial.print("Control Enviado: ");
    Serial.println(frame.control, BIN);

    Serial.print("Data Enviado: ");
    Serial.println(frame.data, BIN);

    Serial.print("CRC Enviado: ");
    Serial.println(frame.crc, BIN);

    Serial.print("Flag de Fim Enviado: ");
    Serial.println(frame.flagFim, BIN);
    Serial.print("");
}

uint8_t ajustarControl(uint8_t controlAtual)
{
    // Incrementa o valor nas posições 5, 6 e 7 do controle
    uint8_t controlNovo = controlAtual << 4; // Incrementa 2^4 (bit 6)
    controlAtual = controlAtual & 0b00000111;
    controlNovo = controlAtual + controlNovo;
    controlNovo = controlNovo & 0b01110111;
    return controlNovo;
}

uint8_t calcularXOR(uint8_t byte1, uint8_t byte2, uint8_t byte3)
{
    return byte1 ^ byte2 ^ byte3;
}

uint8_t verificaCrc(Frame frame)
{
  uint8_t result = calcularXOR(frame.addressRec, frame.controlRec, frame.dataRec);

  if (result == frame.crcRec)
    return true;

  return false;
}
