#include <Servo.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>



// https://gist.github.com/igrr/9ef4d5c74355503e3b1f

const char* ssid = "VaasaHacklab";
const char* pass = "wifikey";


const char* host     = "mikeful.kapsi.fi"; // Your domain  
String path          = "/ohjain/ohjain.json";  

//http://mikeful.kapsi.fi/ohjain/ohjain.json

int id = 0;
int robospeed =  14;


ESP8266WebServer server(80);

Servo servoleft;
Servo servoright;

void setup(void){
  Serial.begin(115200);
  Serial.println("");
  
  WiFi.begin(ssid, pass);
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  server.on("/", [](){
    server.send(200, "text/plain", "Open /servo?value=90 to control servo");
  });

  server.on("/servo", [](){
    String sval = server.arg("value");
    int ival = sval.toInt();
    Serial.print("Servo: ");
    Serial.println(ival);
    //myservo.write(ival);
    server.send(200, "text/plain", String(ival, DEC));
  });

  server.begin();
  Serial.println("HTTP server started");
  
  servoleft.attach(D1);   // Servo attached to D5 pin on NodeMCU board
  servoleft.write(91);

  servoright.attach(D2);   // Servo attached to D5 pin on NodeMCU board
  servoright.write(91);


}
 
void loop() {  
  Serial.print("connecting to ");
  Serial.println(host);
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  client.print(String("GET ") + path + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: keep-alive\r\n\r\n");

  delay(300); //500 // wait for server to respond

  // read response
  String section="header";
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
    // we’ll parse the HTML body here
    if (section=="header") { // headers..
      Serial.print(".");
      if (line=="\n") { // skips the empty space at the beginning 
        section="json";
      }
    }
    else if (section=="json") {  // print the good stuff
      section="ignore";
      String result = line.substring(1);

      // Parse JSON
      int size = result.length() + 1;
      char json[size];
      result.toCharArray(json, size);
      StaticJsonBuffer<200> jsonBuffer;
      JsonObject& json_parsed = jsonBuffer.parseObject(json);
      if (!json_parsed.success())
      {
        Serial.println("parseObject() failed");
        return;
      }

      int newid = json_parsed["id"];
      if (!(newid>id))
      {
        return;
      }
      id = newid;
      // Make the decision to turn off or on the LED
      servoright.write(91);
      servoleft.write(91);
      if (strcmp(json_parsed["command"], "left") == 0) {
//        digitalWrite(pin, HIGH); 
        Serial.println("command left");
        servoright.write(91+robospeed);
        servoleft.write(91+(robospeed/3));
        delay(220);
        servoright.write(91);
        servoleft.write(91);
      }
      else if (strcmp(json_parsed["command"], "right") == 0) {
//        digitalWrite(pin, HIGH); 
        Serial.println("command right");
        servoleft.write(91-robospeed);
        servoright.write(91-(robospeed/3));
        delay(220);
        servoleft.write(91);
        servoright.write(91);

      }
      else if (strcmp(json_parsed["command"], "forward") == 0) {
//        digitalWrite(pin, HIGH); 
        Serial.println("command forward");
        servoleft.write(91-robospeed);
        servoright.write(91+robospeed);
        delay(500);
        servoleft.write(91);
        servoright.write(91);
      }
      else if (strcmp(json_parsed["command"], "backward") == 0) {
//        digitalWrite(pin, HIGH); 
        Serial.println("command backward");
        servoleft.write(91+robospeed);
        servoright.write(91-robospeed);
        delay(300);
        servoleft.write(91);
        servoright.write(91);

      }
      else {
      //  digitalWrite(pin, LOW);
        Serial.println("no command");
      }
    }
  }
  Serial.print("closing connection. ");
}
