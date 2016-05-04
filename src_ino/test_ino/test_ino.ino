int pin5 = 5;
int pin6 = 6;
int pin7 = 7;
int pin8 = 8;

int val5 = 0;
int val6 = 0;
int val7 = 0;
int val8 = 0;

char buffer[16];

void setup() 
{                
  pinMode(pin5, INPUT);
  pinMode(pin6, INPUT);
  pinMode(pin7, INPUT);
  pinMode(pin8, INPUT);
  
  Serial.begin(9600);   
}

void loop() 
{
  val5 = digitalRead(pin5);
  val6 = digitalRead(pin6);
  val7 = digitalRead(pin7);
  val8 = digitalRead(pin8);
  
  sprintf (buffer, "%d %d %d %d", val5, val6, val7, val8);
  Serial.println(buffer);   
}
