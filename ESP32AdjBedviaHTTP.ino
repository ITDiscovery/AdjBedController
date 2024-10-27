// Load Wi-Fi library
#include <WiFi.h>
#include <Preferences.h>
//#define DEBUG

#ifdef DEBUG
#include "USB.h"
#endif

// Replace with your network credentials
char ssid[] = "xxxxxxxx";
char wifipwd[] = "xxxxxxxxx";
byte bdmem[5][3] = {{0,0,0},{0,4,4},{0,3,2},{0,14,5},{0,1,1}};
Preferences preferences;

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;
String swheader;
String motordata;

// Variables to store the current output state
// 0 = off
// 1 = up
// 2 = down
byte outputM1 = 0;
byte outputM2 = 0;

//RelayK1 - 35
#define RelayK1 35
//RelayK2 - 33
#define RelayK2 33
//RelayK3 - 21 - Not really sure 16 had sporadic strobes
#define RelayK3 21
//RelayK4 - 18 
#define RelayK4 18

//Motor Speed Analog Read:
//Motor1 - 3
//Motor2 - 5
#define Motor1In 3
#define Motor2In 5
#define MotorOnThresh 100

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

void MotorOn(int relaypin, int motorpin, int runtm) {
    motordata = relaypin;
    motordata.concat(" - ");
    motordata.concat(motorpin);
    motordata.concat(" - ");
    motordata.concat(runtm*100);
    digitalWrite(relaypin, HIGH);
    int j=0;
    delay(100);
    while ((j<=runtm) && (analogRead(motorpin) >  MotorOnThresh)) {
      j++;
      delay(100);
    }
    digitalWrite(relaypin,LOW);
    //Add delay for multiple MotorOn commands
    delay(400);
}

void MotorTest(int relaypin, int motorpin, int runtm) {
    //Test Motor1 to see the delay
    int j;
    motordata = relaypin;
    motordata.concat(" - ");
    motordata.concat(motorpin);
    motordata.concat(" - ");
    motordata.concat(runtm*100);
    motordata.concat("<BR>");
    digitalWrite(relaypin, HIGH);
    for (j=0; j <= 100; j=j+10) {
      delay(10);
      motordata.concat(j);
      motordata.concat(" - ");
      motordata.concat(analogRead(motorpin));
      motordata.concat("<BR>");
    }
    j=0;
    while ((j<=runtm) && (analogRead(motorpin) >  MotorOnThresh)) {
      j++;
      motordata.concat(j*100+100);
      motordata.concat(" - ");
      motordata.concat(analogRead(motorpin));
      motordata.concat("<BR>");
      delay(100);
    }
    digitalWrite(relaypin,LOW);
}

void setup() {

  #ifdef DEBUG 
  USBSerial.begin();  //USB seems to be strobing GPIO18
  USB.begin();
  #endif

  // Initialize the output variables as outputs
  pinMode(BUILTIN_LED, OUTPUT);
  
  pinMode(RelayK1, OUTPUT);
  pinMode(RelayK2, OUTPUT);
  pinMode(RelayK3, OUTPUT);
  pinMode(RelayK4, OUTPUT);
  pinMode(Motor1In, INPUT);
  pinMode(Motor2In, INPUT);
  
  // Set outputs to LOW
  digitalWrite(RelayK1, LOW);
  digitalWrite(RelayK2, LOW);
  digitalWrite(RelayK3, LOW);
  digitalWrite(RelayK4, LOW);
  
  digitalWrite(BUILTIN_LED,HIGH);
  delay(500);

  #ifdef DEBUG
  USBSerial.write("Connecting to ",14);
  USBSerial.write(ssid,strlen(ssid));
  USBSerial.write("!\n\r",3);
  #endif

  // Connect to Wi-Fi network with SSID and password
  WiFi.begin(ssid, wifipwd);
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(BUILTIN_LED,LOW);
    delay(250);
    digitalWrite(BUILTIN_LED,HIGH);
    #ifdef DEBUG
    USBSerial.write(".",1);
    #endif
  }
  // Print local IP address and start web server
  #ifdef DEBUG
  USBSerial.write("\n\rIP Address: ",14);
  #endif
  
  IPAddress ip = WiFi.localIP();
  #ifdef DEBUG
  Serial.println(ip);
  #endif

  digitalWrite(BUILTIN_LED,LOW);
  server.begin();

  preferences.begin("esp32app",false);
  unsigned int appcounter = preferences.getUInt("appcounter", 0);
  //See if this has been run before
  if (appcounter == 0) {
    #ifdef DEBUG
    USBSerial.println("First run!",10);
    #endif
    preferences.putString("ssid",ssid);
    preferences.putString("wifipwd",wifipwd);
    preferences.putBytes("bdmem", (byte*)(&bdmem), sizeof(bdmem));
  //It's not, so populate it
  } else {
    #ifdef DEBUG
    USBSerial.println("Not first run!",14);
    #endif
    preferences.getString("ssid",ssid);
    preferences.getString("wifipwd",wifipwd);
    preferences.getBytes("bdmem", &bdmem, sizeof(bdmem));
  }
  appcounter++;
  preferences.putUInt("appcounter",appcounter);
}

void loop(){

  digitalWrite(RelayK1, LOW);
  digitalWrite(RelayK2, LOW);
  digitalWrite(RelayK3, LOW);
  digitalWrite(RelayK4, LOW);

  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    #ifdef DEBUG
    USBSerial.write("New Client.\n\r",13);          // print a message out in the serial port
    #endif
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        //Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            swheader = header.substring(5,13);
            // Processes commands sent by buttons

            //if it's a get, setup a loop to grab the variables
            if (swheader == "get?ssid") {
              for (byte x=1;x<5;x++) {
                String firstarg = "mem";
                firstarg.concat(x);
                firstarg.concat("m1");
                int ind1 = header.indexOf(firstarg);
                int ind2 = header.indexOf("&",ind1+1);
                firstarg = header.substring(ind1+7,ind2);
                bdmem[x][1] = byte(firstarg.toInt());
                firstarg = "mem";
                firstarg.concat(x);
                firstarg.concat("m2");
                ind1 = header.indexOf(firstarg);
                ind2 = header.indexOf("&",ind1+1);
                firstarg = header.substring(ind1+7,ind2);
                bdmem[x][2] = byte(firstarg.toInt());
              }
              preferences.putBytes("bdmem", (byte*)(&bdmem), sizeof(bdmem));  
            }

            if (swheader == "M1-stpup") {
              MotorOn(RelayK1, Motor1In, 10);
            } else if (swheader == "M1-allup") {
              MotorOn(RelayK1, Motor1In, 200);
            } else if  (swheader == "M1-stpdn") {
              MotorOn(RelayK2, Motor1In, 10);
            } else if (swheader == "M1-alldn") {
              MotorOn(RelayK2, Motor1In, 200);
            } else if (swheader == "M2-stpup") {
              MotorOn(RelayK3, Motor2In, 10);
            } else if (swheader == "M2-allup") {
              MotorOn(RelayK3, Motor2In, 200);
            } else if (swheader == "M2-stpdn") {
              MotorOn(RelayK4, Motor2In, 10);
            } else if (swheader == "M2-alldn") {
              MotorOn(RelayK4, Motor2In, 200);
            } else if (swheader == "M3-sleep") {
              //Go all down, then up motor1 bdfmem[1][1] secs up motor2 bdmem[1][2] sec
              MotorOn(RelayK2, Motor1In, 200);
              MotorOn(RelayK4, Motor2In, 200);
              MotorOn(RelayK1, Motor1In, bdmem[1][1]*10);
              MotorOn(RelayK3, Motor2In, bdmem[1][2]*10);
            } else if (swheader == "M3-slep1") {
              MotorOn(RelayK2, Motor1In, 200);
              MotorOn(RelayK4, Motor2In, 200);
              MotorOn(RelayK1, Motor1In, bdmem[2][1]*10);
              MotorOn(RelayK3, Motor2In, bdmem[2][2]*10);
            } else if (swheader == "M3-read0") {
              MotorOn(RelayK2, Motor1In, 200);
              MotorOn(RelayK4, Motor2In, 200);
              MotorOn(RelayK1, Motor1In, bdmem[3][1]*10);
              MotorOn(RelayK3, Motor2In, bdmem[3][2]*10);
            } else if (swheader == "M3-flat0") {
              MotorOn(RelayK2, Motor1In, 200);
              MotorOn(RelayK4, Motor2In, 200);
              MotorOn(RelayK1, Motor1In, bdmem[4][1]*10);
              MotorOn(RelayK3, Motor2In, bdmem[4][2]*10);
            } else if (swheader == "M4-mtr1u") {
              MotorTest(RelayK1, Motor1In, 5);
            } else if (swheader == "M4-mtr2u") {
              MotorTest(RelayK3, Motor2In, 5);
            } else if (swheader == "M4-mtr1d") {
              MotorTest(RelayK2, Motor1In, 5);
            } else if (swheader == "M4-mtr2d") {
              MotorTest(RelayK4, Motor2In, 5);
            }
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, no-cache, no-store, must-revalidate, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #d303fc;}");
            client.println(".button3 {background-color: #0703fc;}");
            client.println(".button4 {background-color: #fc5203;}");
            client.println(".button5 {background-color: #555555;}</style></head>");

            // Web Page Heading
            client.println("<body><h1>ESP-32 Adjustable Bed Web Server</h1>");
            
            // Motor 1
            client.println("<p>Motor 1: ");
            client.println("<a href=\"/M1-stpup\"><button class=\"button\">Step Up</button></a>");
            client.println("<a href=\"/M1-allup\"><button class=\"button button2\">All Up</button></a>");
            client.println("<a href=\"/M1-stpdn\"><button class=\"button button3\">Step Down</button></a>");
            client.println("<a href=\"/M1-alldn\"><button class=\"button button4\">M1 Flat</button></a>");
            client.println(" </p>");

            // Motor 2
            client.println("<p>Motor 2: ");
            client.println("<a href=\"/M2-stpup\"><button class=\"button\">Step Up</button></a>");
            client.println("<a href=\"/M2-allup\"><button class=\"button button2\">All Up</button></a>");
            client.println("<a href=\"/M2-stpdn\"><button class=\"button button3\">Step Down</button></a>");
            client.println("<a href=\"/M2-alldn\"><button class=\"button button4\">M2 Flat</button></a>");
            client.println(" </p>");               
            
            // Memory Slots
            client.println("<p>Memory: ");
            client.println("<a href=\"/M3-sleep\"><button class=\"button\">Sleep</button></a>");
            client.println("<a href=\"/M3-slep1\"><button class=\"button button2\">Sleep 1</button></a>");
            client.println("<a href=\"/M3-read0\"><button class=\"button button3\">Reading</button></a>");
            client.println("<a href=\"/M3-flat0\"><button class=\"button button4\">Flat</button></a>");
            client.println(" </p>");  

            // Test Slots
            client.println("<p>Testing: ");
            client.println("<a href=\"/M4-mtr1u\"><button class=\"button\">Motor 1 Up Test</button></a>");
            client.println("<a href=\"/M4-mtr2u\"><button class=\"button button2\">Motor 2 Up Test</button></a>");
            client.println("<a href=\"/M4-mtr1d\"><button class=\"button button3\">Motor 1 Down Test</button></a>");
            client.println("<a href=\"/M4-mtr2d\"><button class=\"button button4\">Motor 2 Down Test</button></a>");
            client.println(" </p>");  

            // The HTTP response ends with the output and 
            client.println("<p>");

            // Wifi Part
            client.print("<p><form action=\"/get\">  SSID: <input type=\"text\" name=\"ssid\" value=\"");
            client.print(ssid);
            client.print(" \"> WiFi PWD: <input type=\"password\" name=\"wifipwd\" value=\"");
            client.print(wifipwd);
            client.println("\"><BR>");

            //EEPROM Part
            for (byte x=1;x<5;x++) {
              client.print("Memory ");
              client.print(x);
              client.print(" M1:<input type=\"text\" name=\"mem");
              client.print(x);
              client.print("m1\" value=\"");
              client.print(bdmem[x][1]);
              client.print("\"> M2:<input type=\"text\" name=\"mem");
              client.print(x);
              client.print("m2\" value=\"");
              client.print(bdmem[x][2]);
              client.println("\"><BR>");
            }
            client.println("<input type=\"submit\" value=\"Submit\"> </form></p>");

            #ifdef DEBUG
            client.println("Test Data: ");
            client.println(swheader);
            client.println("=");
            client.println(motordata);
            #endif
            client.println("</p></body>");
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    #ifdef DEBUG
    USBSerial.write("Client disconnected.\n\r",22);
    #endif
  }
}
