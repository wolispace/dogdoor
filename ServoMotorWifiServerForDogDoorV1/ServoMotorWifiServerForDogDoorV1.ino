/*
 WiFi Web Server LED Blink

 A simple web server that lets you blink an LED via the web.
 This sketch will print the IP address of your WiFi Shield (once connected)
 to the Serial monitor. From there, you can open that address in a web browser
 to turn on and off the LED on pin 5.

 If the IP address of your shield is yourAddress:
 http://yourAddress/H turns the LED on
 http://yourAddress/L turns it off

 This example is written for a network using WPA2 encryption. For insecure
 WEP or WPA, change the Wifi.begin() call and use Wifi.setMinSecurity() accordingly.

 Circuit:
 * WiFi shield attached
 * LED attached to pin 5

 created for arduino 25 Nov 2012
 by Tom Igoe

ported for sparkfun esp32 
31.01.2017 by Jan Hendrik Berlin

output tab is one thing, Serial Monitor tab is another. Watch this one for web server output like ip:
http://192.168.86.30

needs /api for now
http://192.168.86.250/hi
 
 */

#include <WiFi.h>
#include <ESP32Servo.h>  // Include the ESP32Servo library
// #include <vector>

Servo myservo;  // Create a servo object
int openPosition = 179;
int closedPosition = 1;
bool currentState = false;

const int servoPin = 21;  // Define the GPIO pin connected to the servo signal wire

const char* ssid = "Marmalade";
const char* password = "100%Minnie!!";

String serverAddress;


WiFiServer server(80);

void setup()
{
    Serial.begin(115200);
    delay(10);
    myservo.attach(servoPin);  // Attach the servo to the specified pin
    WiFi.begin(ssid, password); // We start by connecting to a WiFi network

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    serverAddress = WiFi.localIP().toString();
    Serial.println("\nDogDoor connected to " + String(ssid) + " " + serverAddress);
    
    server.begin();

    int currentPos = myservo.read();
    Serial.print("Current pos = ");
    Serial.println(currentPos);
    Serial.println("state: "+ String(currentState));
}



// returns a query param eg state=true as a string
String getParameter(String query, String param) {
  int paramStart = query.indexOf(param + "=");
  if (paramStart == -1) return ""; // Parameter not found

  paramStart += param.length() + 1; // Move past "param="
  int paramEnd = query.indexOf("&", paramStart); // Find next delimiter
  if (paramEnd == -1) paramEnd = query.length(); // If last parameter, set endpoint

  return query.substring(paramStart, paramEnd);
}

void sendJsonResponse(WiFiClient &client) {
  //Serial.println("Send json");
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:application/json");
  client.println();
  client.println("{\"state\": \""+ String(currentState) + "\"}");
  client.println();
}
// HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
// and a content-type so the client knows what's coming, then a blank line:
void sendHtmlResponse(WiFiClient &client) {
  //Serial.println("Send html");
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();
  String style = "<style>body{background-color:darkslategray;display:flex;justify-content:center;align-items:center;} .button{position:relative;width:100px;height:100px;cursor:hand;} .hole{position:absolute;width:100%;height:100%;background-color:rgba(0,0,0,0.8);border-radius:10px;} .door{position:absolute;width:100%;height:100%;background-color:rgb(171,113,75);border-radius:10px;border-top:1px solid white;border-left:1px solid rgba(255,255,255,0.5);border-bottom:1px solid rgba(0,0,0,0.8);border-right:1px solid rgba(0,0,0,0.5);transition:transform 0.5s ease-in-out;} .open{transform:translateX(-100%);} .closed{transform:rotateY(-180deg);}</style>";

  // the content of the HTTP response follows the header:
  client.println("<html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no\"></head>");
  client.println(style);

  String script = "<script>                         \
      class Door {                                  \
        url = 'http://" + serverAddress + "/api/';  \
 \
        constructor() {                                    \
          this.button = document.querySelector('.button'); \
          this.door = document.querySelector('.door');     \
          this.getState();                                 \
          this.showState();                                \
        };                                                 \
 \
        getState() {                           \
          const data = fetch(this.url)         \
            .then(response => response.json()) \
            .then(data => {                    \
              console.log(data);               \
              this.setState(data.state);       \
            })                                 \
            .catch(error => {                                     \
              console.error('Error fetching door state:', error); \
            });                                                   \
            this.showState();                                     \
        };                                                        \
        setPos(pos) { \
          const data = fetch(this.url + `?pos=${pos}`); \
        } \
 \
        writeState() {                               \
          const data = { state: this.state };        \
          fetch(this.url + `?state=${this.state}`, { \
            method: 'GET',                           \
            headers: {                               \
              'Content-Type': 'application/json'     \
            },                                       \
          })                                         \
          .then(response => response.json())         \
          .then(reply => {                                       \
            console.log('State sent successfully:', data, reply); \
          })                                                      \
          .catch(error => {                                     \
            console.error('Error updating door state:', error); \
          });                                                   \
        };                                                      \
 \
        setState(state) {               \
          this.state = parseInt(state); \
          this.showState();             \
        };                              \
 \
        showState() {       \
          if (this.state) { \
            this.open();    \
          } else {          \
            this.close();   \
          }                 \
        };                  \
 \
        open() {                                \
          /*this.state = true;*/                \
          this.door.classList.remove('closed'); \
          this.door.classList.add('open');      \
        };                                      \
 \
        close() {                             \
          /*this.state = false;*/             \
          this.door.classList.remove('open'); \
          this.door.classList.add('closed');  \
        };                                    \
 \
        toggle() {                  \
          this.state = !this.state; \
          this.showState();         \
          this.writeState();        \
        };                          \
      };                            \
 \
      document.addEventListener('DOMContentLoaded', () => {    \
        const door = new Door();                               \
        const button = document.querySelector('.button');      \
        button.addEventListener('click', () => door.toggle()); \
        setInterval(() => door.getState(), 30000);             \
      });                                                      \
</script>";
  client.println(script);
  client.println("<body> \
  <div class='button'> \
    <div class='hole'> \
    </div> \
    <div class='door closed'> \
    </div>   \
  </div> \
</body> \
</html>");

}

void handleClient(WiFiClient client) {
  Serial.println("New Client. state: "+ String(currentState)); 
  String currentLine = "";    
  bool wasApiRequest = false;  
  unsigned long startTime = millis();

  while (client.connected()) {  
    if (millis() - startTime > 500) {  // Timeout after .5 seconds
      Serial.println("Timeout waiting for client data.");
      break;
    }
    
    if (client.available()) {  
      char c = client.read();  
      
      if (c == '\n') {  
        if (currentLine.length() == 0) {  
          if (!wasApiRequest) {
            sendHtmlResponse(client);
          } else {
            sendJsonResponse(client);
          }
          break;
        } else {  
          if (currentLine.indexOf("GET /api/") != -1) {
            wasApiRequest = true;  
            int spaceIndex = currentLine.indexOf(" HTTP");
            if (spaceIndex != -1) {
              currentLine = currentLine.substring(0, spaceIndex);  
            }
            Serial.println("Request Line: [" + currentLine + "]");  

            String pos = getParameter(currentLine, "pos");
            if (!pos.isEmpty()) {
              int newPos = atoi(pos.c_str()); // Convert string to integer
              myservo.write(newPos);
              delay(500);
              Serial.println("set new pos: " + String(newPos));
            }

            String state = getParameter(currentLine, "state");
            if (!state.isEmpty()) { 
              String time = getParameter(currentLine, "time");

              Serial.println("set new state: " + state);
              //Serial.println("set new time: " + time);
              currentState = state == "true";
              if (currentState) {
                myservo.write(openPosition);
                Serial.println("set new pos: " + String(openPosition));
              } else {
                myservo.write(closedPosition);
                 Serial.println("set new pos: " + String(closedPosition));
              }
               delay(10);
            }
          }
          currentLine = "";  
        }
      } else if (c != '\r') {  
        currentLine += c;  
      }  
    }
  }
  client.flush();
  delay(10);
  client.stop();  
  Serial.println("Client Disconnected. state: "+ String(currentState));
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    handleClient(client);
  }
}
