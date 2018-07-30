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
  server.send(200, "text/html", 
    "<!DOCTYPE html>"
    "<HEAD>"
      "<meta charset=\"utf-8\" />"
      "<TITLE>Anco Pumps</TITLE>"
      "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
      "<style>"
      //styling the body
        "body{"
        "margin:0px 0px; padding:0px;"
        "text-align:center;"
        "background-color:#fff"
        "}"

       //div which contains the navigation bar
        ".ancoPumps_container {background-color:#fff;"
         "padding-top: 5px;"
         "color: #767B7A;"
         "font_size: 18px;"
         "margin-bottom: 0px;"
         "border-bottom: 1px solid #767B7A;"
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
        
    //Dashboard
           ".dashboard{"
           "padding-top: 5px"
            "padding: 5px;"
           "color: #767B7A;"
            "font-size: 16px;" 
         "}"
    //Pump Container
//      ".pump_container{"
//        "background-color: #9A9A9A;"
//        "width: 50%;"
//        "margin: 1px auto;"
//        "margin-bottom: 10px;"
//        "padding: 10px;"
//        "border: 1px 1px 1px 1px solid #9A9A9A;"
//        "border-radius:3px 3px 3px;"
//        "-o-border-radius:10px 10px 10px;"
//        "-webkit-border-radius:10px 10px 10px;"   
      "}"
    //pumpSpeed
      ".pumpSpeed{"
        "color:#767B7A;"
        "font-size:18px;"
        "padding: 0px;"
      "}"
      
   //pumpValues
       ".pumpValues{"
        "color:#000;"
        "font-size:35px;"
        "padding: 5px;"
      "}"
      
    
        ".on{"
          "background-color:#09004A;"
          "padding: 20px;"
          "margin: 10px;"
          "color: #ffffff;"
          
        "}"

        ".off{"
          "background-color:#6B0000;"
          "padding: 20px;"
          "margin: 10px;"
          "color: #ffffff;"
        "}"
    
     //Powered by Maxim   
        ".footer" 
        "{"
        "font-size: 16px;"
        "color: #A6C247;"
         "position: fixed;"
         "left: 0;"
         "bottom: 0;"
        "width: 100%;"
        "padding: 5px;"
        "background-color:#767B7A;"
       "}"
       
        "a:link {color:white;}"      /* unvisited link */
        "a:visited {color:white;}"  /* visited link */
        "a:hover {color:white;}"  /* mouse over link */
        "a:active {color:white;}"  /* selected link */
      "</style>"
    
      "<TITLE>Anco Pumps</TITLE>"
    "</HEAD>"
    "<BODY>"
      "<div class=\"ancoPumps_container\">"
        "<h1 class=\"ancoPumps\">Anco Pumps</h1>"
        "<h2 class=\"dashboard\">Control DashBoard</h2>"
      "</div>"
        "<h2 class=\"pumpSpeed\">Pump Speed:</h2>"
      "<h1 class=\"pumpValues\" id=\"data\">"
      "</h1>"
    "<br />""<br />"
        "<a href=\"/?pump=1\" class=\"on\">Turn On Pump</a>"
        "<a href=\"/?pump=0\" class=\"off\">Turn Off Pump</a>"
    "<br />""<br />"

    "<div class=\"footer\">"
      "<p>Powered by Maxim Nyansa</p>"
    "</div>"
//      "<script>"
//        "var x = setInterval(function(){document.getElementById(\"data\").innerHTML = "+String(mappedValue)+"}() , 1000);"
//      "</script>"

      "<script>"
        "var x = setInterval(function() {loadData(\"data.txt\",updateData)}, 1000);"
        "function loadData(url, callback){"
          "var xhttp = new XMLHttpRequest();"
          "xhttp.onreadystatechange = function(){"
            "if(this.readyState == 4 && this.status == 200){"
              "callback.apply(xhttp);"
            "}"
          "};"
          "xhttp.open(\"GET\", url, true);"
          "xhttp.send();"
        "}"
        "function updateData(){"
          "document.getElementById(\"data\").innerHTML = this.responseText;"
        "}"
      "</script>"
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
  server.on("/data.txt", [](){
   String text = (String)mappedValue;
   server.send(200, "text/html", text);
 });
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
