void setup() {
  Serial.begin(38400);
  Serial1.begin(38400);
  Serial.setTimeout(5);
  Serial1.setTimeout(5);
  
}

void loop() {
  if (Serial.available())
  {
    char message[30];
    int amount = Serial.readBytesUntil(';', message, 30);
    message[amount] = NULL; 
    
    Serial1.write(message);
  }
}
