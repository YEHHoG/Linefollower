#include "SerialCommand.h"
#include "EEPROMAnything.h"

#define SerialPort Serial
#define Baudrate 115200
#define EEPROM_SIZE 512

SerialCommand sCmd(SerialPort);
bool debug;
unsigned long previous, calculationTime;

//ADC1 pinnen op de ESP32 (overeenkomend met 33, 32, 35, 34, VN, VP)
const int sensor[] = {33, 32, 35, 34, 39, 36};

struct param_t
{
  unsigned long cycleTime;
  /* andere parameters die in het eeprom geheugen moeten opgeslagen worden voeg je hier toe ... */
  int black[6];
  int white[6];
} params;

void setup()
{
  SerialPort.begin(Baudrate);
  delay(1000);
  EEPROM.begin(EEPROM_SIZE);
  sCmd.addCommand("set", onSet);
  sCmd.addCommand("debug", onDebug);
  sCmd.addCommand("calibrate", onCalibrate);
  sCmd.setDefaultHandler(onUnknownCommand);
  EEPROM_readAnything(0, params);
  SerialPort.println("ready");
}

void loop()
{
  sCmd.readSerial();
  unsigned long current = micros();
  if (current - previous >= params.cycleTime)
  {
    previous = current;
    /* code die cyclisch moet uitgevoerd worden programmeer je hier ... */
    if (debug)
    {
    SerialPort.print("Normalised values: ");
    long normalised[6];
    for (int i = 0; i<6; i++)
    {
      long value = analogRead(sensor[i]);
      normalised[i] = map(value, params.black[i], params.white[i], 0, 1000);   
      SerialPort.print(normalised[i]);   
      SerialPort.print(" ");
    }
    SerialPort.println();
    }
  }
  unsigned long difference = micros() - current;
  if (difference > calculationTime) calculationTime = difference;
  
}

void onUnknownCommand(char *command)
{
  SerialPort.print("unknown command: \"");
  SerialPort.print(command);
  SerialPort.println("\"");
}

void onSet()
{
  char* param = sCmd.next();
  char* value = sCmd.next();  

  if (strcmp(param, "debug") == 0)
    {
      if (strcmp(value, "on") == 0) debug = true;
      else if (strcmp(value, "off") == 0) debug = false;
    }
  
  if (strcmp(param, "cycle") == 0) params.cycleTime = atol(value);
  /* parameters een nieuwe waarde geven via het set commando doe je hier ... */
  
  EEPROM_writeAnything(0, params);
  EEPROM.commit();
}

void onDebug()
{
  SerialPort.print("cycle time: ");
  SerialPort.println(params.cycleTime);
  /* parameters weergeven met behulp van het debug commando doe je hier ... */
  
  SerialPort.print("calculation time: ");
  SerialPort.println(calculationTime);
  calculationTime = 0;

  //Zwart waardes van calib
  SerialPort.print("Black:");
  for (int i = 0; i <6; i++)
  {
    SerialPort.print(params.black[i]);
    SerialPort.print(" ");
  }
  SerialPort.println(" ");

  //Wit waardes van calib
  SerialPort.print("White:");
  for (int i = 0; i <6; i++)
  {
    SerialPort.print(params.white[i]);
    SerialPort.print(" ");
  }
  SerialPort.println(" ");
}

void onCalibrate()
{
  char* param = sCmd.next();

  if (strcmp(param, "black") == 0)
  {
    SerialPort.print("start calibrating black... ");
    for (int i = 0; i < 6; i++) params.black[i]=analogRead(sensor[i]);
    SerialPort.println("done");
  }
  else if (strcmp(param, "white") == 0)
  {
    SerialPort.print("start calibrating white... ");
    for (int i = 0; i < 6; i++) params.white[i]=analogRead(sensor[i]);
    SerialPort.println("done");
  }

  EEPROM_writeAnything(0, params);
  EEPROM.commit();
}
    
  