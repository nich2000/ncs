#define SPEED_L      5 
#define DIR_L        4
#define SPEED_R      6
#define DIR_R        7

int pin1 = 8;
int pin2 = 9;
int pin3 = 10;
int pin4 = 11;

int val1 = 0;
int val2 = 0;
int val3 = 0;
int val4 = 0;

int counter = 0;

char buffer[16];

void setup() 
{ 
  pinMode(SPEED_L, OUTPUT); 
  pinMode(DIR_L,   OUTPUT); 
  pinMode(SPEED_R, OUTPUT); 
  pinMode(DIR_R,   OUTPUT); 
  
  pinMode(pin1, INPUT);
  pinMode(pin2, INPUT);
  pinMode(pin3, INPUT);
  pinMode(pin4, INPUT);
    
  Serial.begin(9600);   
}

void loop() 
{
  counter++;
  
  val1 = digitalRead(pin1);
  val2 = digitalRead(pin2);
  val3 = digitalRead(pin3);
  val4 = digitalRead(pin4);
  
  if(val1 == HIGH)
    analogWrite(SPEED_L, 92);
  else
    analogWrite(SPEED_L, 0);
    
  digitalWrite(DIR_L, val2);
  
  if(val3 == HIGH)
    analogWrite(SPEED_R, 92);
  else
    analogWrite(SPEED_R, 0);
    
  digitalWrite(DIR_R, val4);
  
  sprintf (buffer, "%-5d %d %d %d %d", counter, val1, val2, val3, val4);
  Serial.println(buffer);   
  
  delay(100);
}
