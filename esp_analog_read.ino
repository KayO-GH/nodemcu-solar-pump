#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

const char *ssid = "ANCO FARMS";
const char *password = "12345678";

ESP8266WebServer server(80);

int signalPin = A0;
int vrPin = 2;
int delayLength = 2000;//delay length in milliseconds - should be 5 mins
int airBubblesThreshold = 50;// threshhold for pumping of air bubbles
int manualOff = 0;//boolen value
int mappedValue = 0;


void handleRoot() {
  server.send(200, "text/html", "<TITLE>Anco Pumps</TITLE>"
    "<HEAD>"
      "<meta name='apple-mobile-web-app-capable' content='yes' />"
      "<meta name='apple-mobile-web-app-status-bar-style' content='black-translucent' />"
      "<style>"
        "body{"
        "margin:60px 0px; padding:0px;"
        "text-align:center;"
        "}"
        "h1"
        "{"
        "text-align: center;"
        "font-family:Arial, \"Trebuchet MS\", Helvetica, sans-serif;"
        "}"
        "h2"
        "{"
        "text-align: center;"
        "font-family:Arial, \"Trebuchet MS\", Helvetica, sans-serif;"
        "}"
        "a"
        "{"
        "text-decoration:none;"
        "width:75px;"
        "height:50px;"
        "border-color:black;"
        "border-top:2px solid;"
        "border-bottom:2px solid;"
        "border-right:2px solid;"
        "border-left:2px solid;"
        "border-radius:10px 10px 10px;"
        "-o-border-radius:10px 10px 10px;"
        "-webkit-border-radius:10px 10px 10px;"
        "font-family:\"Trebuchet MS\",Arial, Helvetica, sans-serif;"
        "-moz-border-radius:10px 10px 10px;"
        "background-color:#293F5E;"
        "padding:8px;"
        "text-align:center;"
        "}"
        "a:link {color:white;}"      /* unvisited link */
        "a:visited {color:white;}"  /* visited link */
        "a:hover {color:white;}"  /* mouse over link */
        "a:active {color:white;}"  /* selected link */
      "</style>"
    
      "<TITLE>Anco Pumps</TITLE>"
    "</HEAD>"
    "<BODY>"
      "<H1>Anco Pumps</H1>"
      "<hr />"
      "<br />"
      "<H2>Control DashBoard</H2>"
      "<br />"
      "<h2>Pump Speed:</h2>"
      "<h1>"+String(mappedValue)+"/100</h1>"
      "<a href=\"/\">Refresh</a>"
      "<br/>"
      "<br/>"
      "<br/>"
      "<a href=\"/?pump=1\">Turn On Pump</a>"
      "<a href=\"/?pump=0\">Turn Off Pump</a><br />"
      "<br />"
      "<p>Powered by Maxim Nyansa</p>"
      "<br />"
    "</BODY>"
  "</HTML>");
  int pumpState = server.arg("pump").toInt();
  if(server.hasArg("pump"))
    manualOff = !pumpState;//set global bolean variable
  digitalWrite(vrPin, pumpState);
}  

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(vrPin, OUTPUT);
  Serial.println();
  //setup server
  Serial.print("Configuring access point...");
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  // put your main code here, to run repeatedly:
  yield();
  server.handleClient();//run webpage first
  Serial.print("manual off state");
  Serial.println(manualOff);
  if(manualOff){
    return;
  }else{
    digitalWrite(vrPin, HIGH);
    yield();
  }

  yield();
  int signalValue = analogRead(signalPin);
  yield();
  mappedValue = map(signalValue, 0, 1024, 0, 100);
  //delay(100);
  Serial.println(mappedValue);
  delay(100);

  if(mappedValue < 5){
    yield();
    digitalWrite(vrPin, LOW);//simulate disconnect
    yield();
    delay(100);
    yield();
    digitalWrite(vrPin, HIGH);//simulate connect
    yield();
    delay(2000);
    yield();
  }

  if(mappedValue > airBubblesThreshold){
    yield();
    digitalWrite(vrPin, LOW);//turn off
    yield();
    delay(delayLength);//wait and attempt to turn on again
    yield();
    digitalWrite(vrPin, HIGH);//simulate connect
    yield();
  }

}
