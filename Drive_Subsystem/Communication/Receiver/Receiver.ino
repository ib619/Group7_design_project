void setup() {
  Serial.begin(38400);
  Serial1.begin(38400);
  Serial.setTimeout(5);
  Serial1.setTimeout(5);
}

void loop() {
  if (Serial1.available())
  {
    char message[30];
    int amount = Serial1.readBytesUntil(';', message, 30);
    message[amount] = NULL;

    int data[10];
    int count = 0;
    char* offset = message;
    while (true)
    {
      data[count++] = atoi(offset);
      offset = strchr(offset, ',');
      if (offset) offset++;
      else break;
    }

    int systemSelect = data[0];
    int actionSelect = data[1];
    int value = data[2];

    //Serial.println(systemSelect);
    //Serial.println(actionSelect);
    //Serial.println(value);

    switch(systemSelect){

      case 0: 
      Serial.println("Drive System selected");
      decodeDriveAction(actionSelect);
      break;
    }
    
    
  }

}

void decodeDriveAction(int actionSelect)
{
  switch(actionSelect){
    case 0: Serial.println("Rest");
    break;

    case 1: Serial.println("Move Forward");
    break;

    case 2: Serial.println("Move Backwards");
    break;

    case 3: Serial.println("Turn Left");
    break;

    case 4: Serial.println("Turn Right");
    break;
  }
}
