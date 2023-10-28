// C++ code
//

const int pinBit1 = 2;
const int pinBit2 = 3;
int key1 = 0;
int key2 = 0;
void setup()
{
    Serial.begin(9600);
    pinMode(pinBit1, INPUT);
    pinMode(pinBit2, INPUT);
}

void loop()
{

    key1 = digitalRead(pinBit1);
    key2 = digitalRead(pinBit2);
    //OFF
    if (key2 == 0 && key1 == 0)
        Serial.println("Professor me dá 10");
    else if (key2 == 0 && key1 == 1)
        sendDataToS1();
    else if (key2 == 1 && key1 == 0)
        sendDataToS2();
    else 
        s1SendDataToS2();

    delay(1000);
}


void sendDataToS1(){
    Serial.println("Try Send Data do S1");

}

void sendDataToS2(){
    Serial.println("Try Send Data do S2");
    
}

void s1SendDataToS2(){
    Serial.println("Try S1 Send Data do S2");
    
}