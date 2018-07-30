/*
 *Code to control sensorless pump driver from ESP NodeMCU 
 *Maxim Nyansa IT Solutions
 *28th July, 2018
 *
 *Note that yield() function is called multiple times to allow ESP to handle WiFi state 
 *without a break in code
 */
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

const char *ssid = "ANCO FARMS";
const char *password = "12345678";

ESP8266WebServer server(80);

int signalPin = A0;//connect signal pin of driver board to A0 on the ESP NodeMCU
//NOTE: the A0 pin takes a maximum of 3.3V
//the signal pin might have to be passed through a voltage divider if it works at 5V

int vrPin = 2;//connect VR pin of driver board to pin D4 of ESP board.
//D4 corresponds to GPIO pin 2

int delayLength = 2000;//delay length in milliseconds - should be 5 mins
//this is the time to wait before attempting to switch on the pump after encountering air bubbles

int airBubblesThreshold = 50;// threshhold speed for pumping of air bubbles
//This should be tested and the appropriate value assigned to the variable airBubblesThreshold

int manualOff = 0;//boolen value to tell if pump was manually turned off

int mappedValue = 0;//global variable for storing mapped value of pump speed

/*
 * decfine function handleRoot to render page displaying current pump speed
 */
void handleRoot() {
  //render HTML page to screen
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
          "font-family:Arial, \"Trebuchet MS\", Helvetica, sans-serif;"
        "}"
  
         //div which contains the navigation bar
        ".ancoPumps_container {"
          "background-color:#f8f8f8;"
          "padding-top: 5px;"
          "color: #3d3d3d;"
          "font_size: 18px;"
          "margin-bottom: 0px;"
          "border-bottom: 1px solid #999;"
          "box-shadow: 0 0 10px 0 #888"
        "}"
           
        "h1{"
          "text-align: center;"
          "font-family:Arial, \"Trebuchet MS\", Helvetica, sans-serif;"
        "}"
          
        "h2{"
          "text-align: center;"
          "font-family:Arial, \"Trebuchet MS\", Helvetica, sans-serif;"
        "}"
          
        "a{"
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
  
        //pumpSpeed
        ".pumpSpeed{"
          "color:#767B7A;"
          "font-size:18px;"
          "padding: 0px;"
        "}"

        //pumpSwitch
        ".pumpSwitch{"
          "color:#000;"
          "font-size:22px;"
          "padding: 0px;"
          "margin-bottom: 30px"
        "}"
        
        //pumpValues
        ".pumpValues{"
         "color:#000;"
         "font-size:45px;"
         "font-weight: bold"
         "background-color: #77777a"
         "border: 1px 1px 1px 1px solid #77777a"
         "-o-border-radius:1px 1px 1px;"
         "-webkit-border-radius:1px 1px 1px;"
         "border-radius: 50%"
        "}"
        
        ".on{"
          "background-color:#09004A;"
          "padding: 20px 50px;"
          "margin: 10px;"
          "color: #ffffff;"  
          "font-size: 1.4em"   
        "}"
  
        ".off{"
          "background-color:#6B0000;"
          "padding: 20px 50px;"
          "margin: 10px;"
          "color: #ffffff;"
          "font-size: 1.4em"
        "}"
      
        //Powered by Maxim   
        ".footer{"
          "font-size: 16px;"
          "color: #A6C247;"
          "position: fixed;"
          "left: 0;"
          "bottom: 0;"
          "width: 100%;"
          "padding: 5px;"
          "background-color:#767B7A;"
          "font-family:Arial, \"Trebuchet MS\", Helvetica, sans-serif;"
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
      "<h1 class=\"pumpSpeed\">Pump Speed</h1>"
      "<h1 class=\"pumpValues\" id=\"data\">"
      "</h1>"
      "<h1 class=\"pumpSwitch\">Pump Switch</h1>"
      "<a href=\"/?pump=1\" class=\"on\">ON</a>"
      "<a href=\"/?pump=0\" class=\"off\">OFF</a>"
      "<br />""<br />"

      "<div class=\"footer\">"
        "<p>Powered by Maxim Nyansa</p>"
      "</div>"

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
  //extract pump state value from arguments
  int pumpState = server.arg("pump").toInt();
  //make sure root url does not produce a false positive
  if(server.hasArg("pump")){
    manualOff = !pumpState;//set global bolean variable
  }
  //write pump state to VR Pin
  digitalWrite(vrPin, pumpState);
}  

void setup() {
  Serial.begin(115200);
  pinMode(vrPin, OUTPUT);
  Serial.println();
  //setup server
  Serial.print("Configuring access point...");
  WiFi.softAP(ssid, password);//start WiFi Access Point
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);//display IP address in serial monitor
  
  //call handleRoot function at root url
  server.on("/", handleRoot);
  
  //create data.txt page from which AJAX will call pump value
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
  //run webpage
  server.handleClient();
  //Serial.print("manual off state");
  //Serial.println(manualOff);

  if(manualOff){
    //if pump has manually been turned off, return to start of loop
    return;
  }else{
    //else turn on pump
    digitalWrite(vrPin, HIGH);
    yield();
  }

  yield();
  //read value coming in on signal pin
  int signalValue = analogRead(signalPin);
  yield();
  //map value to a scale of 0 to 100
  mappedValue = map(signalValue, 0, 1024, 0, 100);
  //display mapped valuein serial monitor
  Serial.println(mappedValue);
  delay(100);

  //if incoming signal is too low (pump is off), restart pump
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

  //if incoming signal is too high (pump is pumping air), turn off pump
  if(mappedValue > airBubblesThreshold){
    yield();
    digitalWrite(vrPin, LOW);//turn off
    yield();
    delay(delayLength);//wait and attempt to turn on again
    yield();
    digitalWrite(vrPin, HIGH);//turn on
    yield();
  }

}
