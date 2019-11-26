#include <RTClib.h>
#include <SPI.h>
#include <SD.h>

// Date and time functions using a PCF8523 RTC connected via I2C and Wire lib
RTC_PCF8523 rtc;

int previousBatState = LOW;
volatile int pulseCount =0;
int littlePeriodCount =0;
int bigPeriodCount =0;
String myString = "";
String dataFileName = "Test.txt";
int fileCount = 0;
String myTimeString = "";
int newFileExtension = 0;
int emptyFile = LOW;
int littlePeriod = 1000;        // Was 1000
int bigPeriod = 10000000;       // Ratio of big to little needs to be no greater than 2000 or else unstable. Was 2000000.
unsigned long previousMicros =0;
unsigned long batTimePeriod =0;
int timeToCreateNewFile = LOW;
int myStringLength = 0;
// char batData[1000];
String batString[1000];
int batStringCount =0;
int anotherCount = 0;

const byte ledPin = 13;
const byte batReadPin = 6;
int batState = LOW;
int batReading = LOW;

File myFile;

void setup() 
{
  // Delete next 4 lines to deploy:
  while (!Serial) 
  {
    delay(1);  // for Leonardo/Micro/Zero
  }
  pinMode(batReadPin, INPUT_PULLUP);
  // attachInterrupt(digitalPinToInterrupt(interruptPin), blink, CHANGE);
  
  pinMode(12, OUTPUT);
  pinMode(11, OUTPUT);
  digitalWrite(12, HIGH);
  digitalWrite(11, HIGH);
  
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  
  if (! rtc.begin()) 
  {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (! rtc.initialized()) 
  {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

  Serial.print("Initializing SD card...");

  if (!SD.begin(10)) 
  {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open("test.txt", FILE_WRITE);
  // if the file opened okay, write to it:
  String myTimeString = "";
  if (myFile) 
  {
    DateTime now = rtc.now();
    Serial.println("Writing to test.txt...");
    DateTime time = rtc.now();
    myTimeString = String(time.timestamp(DateTime::TIMESTAMP_FULL));
    dataFileName = myTimeString + ".txt";
    Serial.print("myTimeString: ");Serial.println(myTimeString);
    myFile.println(myTimeString);
    // close the file:
    myFile.close();
  } 
  else 
  {
    Serial.println("error opening test.txt");
  }

  // re-open the file for reading:
  myFile = SD.open("test.txt");
  if (myFile) 
  {
    Serial.println("test.txt:");

    // read from the file until there's nothing else in it:
    while (myFile.available()) 
    {
      Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
  } 
  else 
  {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }

// Look for previous files that we want to keep and dont want to append:
  while (emptyFile == LOW)
  {
    dataFileName = "File_" + String(newFileExtension) + ".txt";
    myFile = SD.open(dataFileName, FILE_WRITE);
    if (myFile) 
    {
      Serial.print(dataFileName); Serial.print("  File size Kb: "); Serial.print(  (myFile.size())/1000  );
      if (   ((myFile.size())/1000)>1 )
      {
        Serial.println("  This file already has data !!!! ");
        newFileExtension ++;
        emptyFile = LOW;
///////////////////////////////////////////////////////////////////////////////
        // SD.remove(dataFileName);
///////////////////////////////////////////////////////////////////////////////
      }
      else
      {
        emptyFile = HIGH;
        Serial.print("  New file is: ");Serial.println(dataFileName);
      }
      myFile.close();
    } 

  }         // while (emptyFile == False)
  digitalWrite(11, LOW);
  digitalWrite(12, LOW);
  
}      // Setup.
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void loop() 
{
  digitalWrite(13, LOW);
  bigPeriodCount ++;
  // delayMicroseconds(100);
  batReading = digitalRead(batReadPin);
  
  if ( (batReading == HIGH) && (batState == LOW) )
  {
    // Serial.print("pulseCount: "); Serial.println(pulseCount);
    pulseCount ++;
    batState =! batState;
  }

  if ( (batReading == LOW) && (batState == HIGH) )
  {
    pulseCount ++;
    batState =! batState;
  }
  
//////////////////////////////////////////////////////////////////////////////////////////

    if ( pulseCount > 4 )
    {
      // digitalWrite(13, HIGH);
      littlePeriodCount ++;
      unsigned long currentMicros = micros();
      batTimePeriod = currentMicros - previousMicros;
      previousMicros = currentMicros;
      // Serial.print("batTimePeriod: "); Serial.println(batTimePeriod);
      batString[batStringCount] = batString[batStringCount] + ";" + "["  + String(currentMicros)  + "," + String(batTimePeriod) + "]" ;
      // myString =   myString + ";"      + "["  + String(currentMicros)  + "," + String(batTimePeriod) + "]" ;
      // Serial.print("myString: "); Serial.println(myString);
      // Serial.print("littlePeriodCount: "); Serial.println(littlePeriodCount);
      pulseCount =0;
      if ( anotherCount == 10 )
      {
        batStringCount ++;
        anotherCount = 0;
      }
      anotherCount++;
      //Serial.print("batStringCount: "); Serial.println(batStringCount);

    }                                    //   if ( pulseCount > 4 )

  if ((bigPeriodCount == bigPeriod)||(batStringCount > 100))
  {
    // Serial.print("MyString: "); Serial.println(myString);
    // myStringLength = myString.length();
    // Serial.print("Length of myString: "); Serial.print(myString.length());Serial.print("   ");
    
    myStringLength = 0;
    digitalWrite(11, HIGH);
    DateTime time = rtc.now();
    fileCount ++;
    if ((fileCount > 1000)||( timeToCreateNewFile == HIGH))
    {
      newFileExtension ++;
      fileCount = 0;
      timeToCreateNewFile = LOW;
    }
    dataFileName = "File_" + String(newFileExtension) + ".txt";
    Serial.print("Current file name: "); Serial.print(dataFileName);Serial.print("  ");

    myFile = SD.open(dataFileName, FILE_WRITE);
    if (   ((myFile.size())/1000)>300 )
    {
      timeToCreateNewFile = HIGH;
    }
    else
    {
      timeToCreateNewFile = LOW;
      // Serial.print("Current file size: "); Serial.print((myFile.size())/1000);Serial.print("  ");
    }

    if (myFile) 
    {
      String myTimeString = "";
      myTimeString = String (time.timestamp(DateTime::TIMESTAMP_FULL));
      myFile.println(myTimeString);
      for ( int i = 0; i< batStringCount; i++)
      {
        myFile.print( (batString[i]));
      }
      batStringCount = 0;
      Serial.print(myTimeString); Serial.print("  File size Kb: "); Serial.println(  (myFile.size())/1000  );
      myFile.close();
      myString = "";
    } 
    else 
    {
      Serial.println("error opening " +dataFileName );
    }

    bigPeriodCount =0;
    digitalWrite(11, LOW);
  }                                    // if (bigPeriodCount == bigPeriod)

}                                      // Loop

void blink() 
{
  //batState = !batState;
  pulseCount ++;
}
