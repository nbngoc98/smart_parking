#include <FirebaseESP8266.h>
#include <FirebaseESP8266HTTPClient.h>
#include <FirebaseFS.h>
#include <FirebaseJson.h>

FirebaseJson json;
//3. Declare the Firebase Data object in the global scope
FirebaseData firebaseData;

#include <ESP8266FastTimerInterrupt.h>
#include <ESP8266_ISR_Servo.h>


#include <Servo.h>
#include <ESP8266WiFi.h>
#include <Wire.h>




#define FIREBASE_HOST "smart-parking-nbn98.firebaseio.com"                          
#define FIREBASE_AUTH "qjiBBh2ms0PVNJbbGRgD4DOKFG87jodjRqozD4R1"  

#define WIFI_SSID "Phong tro tang 2"            
#define WIFI_PASSWORD "minhhung@1234"
String Available = "";                                                  //availability string
String fireAvailable = "";

#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);         //i2c display address 27 and 16x2 lcd display
Servo myservo;                          //servo as gate
Servo myservos;                               //servo as gate
int Empty;                            //available space integer
int allSpace = 90;
int countYes = 0;
#define carEnter  16                      //DO// entry sensor 
#define carExited 2  //D4
#define senvo2 14 //D6
#define senvo1 12 //D5
#define TRIG 13     //D7             //ultrasonic trig  pin
#define ECHO 15  //D8              // ultrasonic echo pin
#define led 0 //D3                // spot occupancy signal
int pos;
int pos1;

int CLOSE_ANGLE = 80;  // The closing angle of the servo motor arm
int OPEN_ANGLE = 0;  // The opening angle of the servo motor arm      

long duration, distance;     

void setup() {
  delay(1000);
  Serial.begin (9600);     // serial debugging
  //Wire.begin(2, 0);       // i2c start
  myservo.attach(senvo2);      // servo pin to D6
  myservos.attach(senvo1);       // servo pin to D5
  pinMode(TRIG, OUTPUT);      // trig pin as output 
  pinMode(ECHO, INPUT);         // echo pin as input
  pinMode(led, OUTPUT);         // spot indication
  pinMode(carExited, INPUT);    // ir as input
  pinMode(carEnter, INPUT);     // ir as input

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);                                     //try to connect with wifi
  Serial.print("Connecting to ");
  Serial.print(WIFI_SSID);                          // display ssid
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");                          // if not connected print this
    delay(500);
  }
  Serial.println();
  Serial.print("Connected to ");
  Serial.println(WIFI_SSID);
  Serial.print("IP Address is : ");
  Serial.println(WiFi.localIP());                                            //print local IP address
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);        // begin firebase authentication

  lcd.begin();                      // initialize the lcd 
  // Print a message to the LCD.
  lcd.backlight();                           
  lcd.setCursor(0, 1);                      // 0th row and 0thh column
  lcd.print("Smart Parking");    
}

void loop() {

  digitalWrite(TRIG, LOW);         // make trig pin low
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);        // make trig pin high
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  duration = pulseIn(ECHO, HIGH);
  distance = (duration / 2) / 29.1;      // take distance in cm

    Serial.print("Centimeter: ");
    Serial.println(distance);

  int carEntry = digitalRead(carEnter);      // read ir input
  if (carEntry == LOW) {                     // if high then count and send data
   countYes = countYes+1; 
   Serial.print("Car Entered = " ); Serial.println(countYes);//increment count
    for (pos1 = 180; pos1 >= 90; pos1 -= 1) {         // change servo position
      myservo.write(pos1);
      delay(10);
    }
    delay(2000);
    for (pos1 = 90; pos1 <= 180; pos1 += 1) {            // change servo position
      // in steps of 1 degree
      myservo.write(pos1);
      delay(10);
    }

//    Firebase.pushString("status", fireAvailable );    // send string to firebase
    Empty = allSpace + countYes;         //calculate available data
    Firebase.pushJSON(firebaseData, "/status", json);
    lcd.clear();
  }
  int carExit = digitalRead(carExited);              //read exit ir sensor
  if (carExit == LOW) {                            //if high then count and send
    countYes = countYes-1;                                   //decrement count
    Serial.print("Car Exited = " ); Serial.println(countYes);
   for (pos1 = 180; pos1 >= 90; pos1 -= 1) {         // change servo position
      myservo.write(pos1);
      delay(10);
    }
    delay(2000);
    for (pos1 = 90; pos1 <= 180; pos1 += 1) {            // change servo position
      // in steps of 1 degree
      myservo.write(pos1);
      delay(10);
    }
//    Firebase.pushString("status",fireAvailable);  // send string to firebase
    Empty = allSpace - countYes;         //calculate available data
    Firebase.pushJSON(firebaseData, "/status", json);
    lcd.clear();
  }
  if (distance < 6) {                      //if distance is less than 6cm then on led 
        Serial.println("Occupied ");
    digitalWrite(led, HIGH);
  }

  if (distance > 6) {                        //if distance is greater than 6cm then off led 
        Serial.println("Available ");
    digitalWrite(led, LOW);
  }

  
    Available = String("Available= ") + String(Empty) + String("/") + String(allSpace);       // convert the int to string
    fireAvailable = String("Available=") + String(Empty) + String("/") + String(allSpace);
    json.set("status",fireAvailable);
    lcd.setCursor(0, 0);
    lcd.print(Available);                 //print available data to lcd
}
