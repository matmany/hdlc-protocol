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
  Serial.print("Key1 = ");
  Serial.println(key1);
  Serial.print("Key2 = ");
  Serial.println(key2);
  delay(500);
  Serial.println("Loop end");
}