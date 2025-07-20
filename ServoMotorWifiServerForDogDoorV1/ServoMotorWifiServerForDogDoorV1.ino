/*
 WiFi Web Server control a servo motor

Additional steps if still jittering 
[] Adding a capacitor (100-470μF) between power and ground near the servo
[] Ensuring your power supply can handle the servo's current requirements

output tab is one thing, Serial Monitor tab is another. Watch this one for web server output like ip:
http://192.168.86.30

any request returns the webpage
a request with /api/ returns the current servo state
a request with /api/?state=true|false sets the server to open or close position
a request with /api/?pos=[0:180] sets the servo to that position
http://192.168.86.250/api/?state=true
 
 */

#include <WiFi.h>
#include <ESP32Servo.h>  // Include the ESP32Servo library
#include <HTTPClient.h> // for fetching the time from a seb service

// #include <vector>

Servo myservo;  // Create a servo object
int openPosition = 0;
int closedPosition = 180;
bool currentState = false;
String currentInfo = "";
String nextEvent = "";
String lastEvent = "";
int currentPos = 0;
int servoStep = 1; // how many degrees per step
int servoDelay = 10;
unsigned long startMillis = millis(); // for counting minutes
int hours = 0;  // holds 24hr hours
int mins = 0; // hold 60 min mins
const char* timeApiUrl = "http://worldtimeapi.org/api/timezone/Etc/UTC"; // Change to your timezone
int timerInterval = 1000 * 60 * 1; // check every 15mins to update time or run scheduled events 
int lastCheckMillis = 0;

const int servoPin = 21;  // Define the GPIO pin connected to the servo signal wire

const char* ssid = "Marmalade";
const char* password = "100%Minnie!!";

struct ScheduledEvent {
  int hours;
  int minutes;
  const char* action;
};

// hard-coded schedule chronological order
ScheduledEvent events[] = {
  {6, 0, "OPEN"},
  {22, 0, "CLOSED"}
};

const int numEvents = sizeof(events) / sizeof(events[0]);


String serverAddress;
WiFiServer server(80);

void setup()
{
    Serial.begin(115200);
    delay(10);
    WiFi.begin(ssid, password); // We start by connecting to a WiFi network

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    serverAddress = WiFi.localIP().toString();
    Serial.println("\n\nDogDoor connected to " + String(ssid) + " " + serverAddress);
    
    server.begin();
    showCurrentPos();
    startMillis = millis(); // Reset to the current time
    checkTime();
}

void incrementTime() {
  if (millis() - startMillis >= 60000) { // 60 seconds in milliseconds
    startMillis = millis(); // Reset to the current time
    
    mins++;
    
    if (mins == 60) {
      mins = 0;
      hours++;
      
      if (hours == 24) {
        hours = 0;
      }
    }
  }
}

void updateTimeFromAPI() {
  HTTPClient http;
  http.begin(timeApiUrl);
  
  int httpResponseCode = http.GET();
  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println("Response: " + response);

    // Extract time using simple string parsing (better to use JSON library)
    int timeIndex = response.indexOf("\"datetime\":\"") + 11;
    String timeString = response.substring(timeIndex, timeIndex + 5); // Format: HH:MM
    
    hours = timeString.substring(0, 2).toInt();
    mins = timeString.substring(3, 5).toInt();
    
    Serial.printf("Updated time: %02d:%02d\n", hours, mins);
  } else {
    Serial.printf("No response from %s\n", timeApiUrl);
  }

  http.end();
}

void attachServo() {
  if (!myservo.attached()) {
    myservo.attach(servoPin);
  }
}

void showCurrentPos() {
   Serial.println("Current pos = " + String(currentPos) + " state: "+ String(currentState));
}

void calibrateServo() {
  moveServo(90);
  moveServo(0);

}

void moveServo(int pos) {
  attachServo();
  myservo.write(pos);
  delay(servoDelay);  // wait for things to settle - maybe?
  currentPos = pos; // record this as we cant rely on servo.read()
  showCurrentPos();
}

/**
Move the servo to the specified position slowly
*/
void moveServoSlowly(int endPos) {
    attachServo();
 
    int startPos = currentPos;
    int pos = startPos;
    int step = (endPos > startPos) ? servoStep : -servoStep;
    
    while (pos != endPos) {
      pos += step;
      pos = constrain(pos, min(startPos, endPos), max(startPos, endPos));
      moveServo(pos);
    }
    
    moveServo(endPos); // Ensure we reach exact position
    myservo.detach();
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
  client.println("{\"state\":\""+ String(currentState) + "\",\"info\":\"" + currentInfo + "\"}");
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
  String style = "<style>body{background-color:darkslategray;display:flex;justify-content:center;align-items:center;} .button{position:relative;width:100px;height:100px;cursor:hand;} .hole{position:absolute;width:100%;height:100%;background-color:rgba(0,0,0,0.8);border-radius:10px;} .door{position:absolute;width:100%;height:100%;background-color:rgb(171,113,75);border-radius:10px;border-top:1px solid white;border-left:1px solid rgba(255,255,255,0.5);border-bottom:1px solid rgba(0,0,0,0.8);border-right:1px solid rgba(0,0,0,0.5);transition:transform 0.5s ease-in-out;} .open{transform:translateX(-100%);} .closed{transform:rotateY(-180deg);} .info{position:absolute;top:1rem;left:1rem;color:white;}</style>";

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
          const data = fetch(this.url + `?time=${this.getTime()}`) \
            .then(response => response.json()) \
            .then(data => {                    \
              console.log(data);               \
              this.setState(data.state);       \
              this.showInfo(data.info);         \
            })                                 \
            .catch(error => {                                     \
              console.error('Error fetching door state:', error); \
            });                                                   \
            this.showState();                                     \
        };                                                        \
\        
        getTime() { \                                              \                                         
          const now = new Date();                                 \
          const hours = String(now.getHours()).padStart(2, '0');  \
          const mins = String(now.getMinutes()).padStart(2, '0'); \
          return `${hours}:${mins}`;                              \
        };                                                        \ 
\
        setPos(pos) { \
          const data = fetch(this.url + `?pos=${pos}`); \
        }; \
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
          .then(reply => {                                        \
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
        showInfo(info) {    \
          const infoElement = document.querySelector('.info'); \
          infoElement.innerHTML=info; \
        } \
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
        setInterval(() => door.getState(), 3000);              \
      });                                                      \
</script>";
  client.println(script);
  client.println("<body> \
  <div class='info'>   \
  </div>               \
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
  // Serial.println("New Client. state: "+ String(currentState)); 
  String currentLine = "";    
  bool wasApiRequest = false;  
  unsigned long startTime = millis();

  while (client.connected()) {  
    if (millis() - startTime > 3000) {  // Timeout after .5 seconds
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
            // Serial.println("Request Line: [" + currentLine + "]");  

            String pos = getParameter(currentLine, "pos");
            if (!pos.isEmpty()) {
              int newPos = atoi(pos.c_str()); // Convert string to integer
              moveServo(newPos);
            }

            String state = getParameter(currentLine, "state");
            String timeString = getParameter(currentLine, "time");
            if (!timeString.isEmpty()) {
              hours = timeString.substring(0, 2).toInt();
              mins = timeString.substring(3, 5).toInt();
            }
            Serial.printf("Set state %s time: %02d:%02d\n", state, hours, mins);
            if (!state.isEmpty()) { 
              currentState = state == "true";
              updateServoToState();
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
  // Serial.println("Client Disconnected. state: "+ String(currentState));
}

void updateServoToState() {
  if (currentState) {
    moveServoSlowly(openPosition);
  } else {
    moveServoSlowly(closedPosition);
  }
}

void checkTime() {
  incrementTime();
  currentInfo = makeTimeString(hours, mins);
  int nowTotal = hours * 60 + mins;

  Serial.printf("time: %s\n", currentInfo);
  nextEvent = "none";
  for (int i = 0; i < numEvents; i++) {
    int eventTotal = events[i].hours * 60 + events[i].minutes;
    // no next event then set it to the next upcomming event
    if (nextEvent == "none" && nowTotal < eventTotal) {
      nextEvent = makeTimeString(events[i].hours, events[i].minutes);
    }

    // exact hour and minue match so change state
    if (nowTotal == eventTotal && lastEvent != currentInfo) {
      currentState = events[i].action == "OPEN";
      lastEvent = currentInfo;
      updateServoToState();
    }
  }
    // Wrap to first event if none found
  if (nextEvent == "none") {
    nextEvent = makeTimeString(events[0].hours, events[0].minutes);;
  }
  currentInfo = currentInfo + " next=" + nextEvent;
}

// converts hrs am mins to zero padded hh:mm eg 01:08
String makeTimeString(int hrs, int mins) {
 return (hrs < 10 ? "0" : "") + String(hrs) + ":" + (mins < 10 ? "0" : "") + String(mins);
}


void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - lastCheckMillis >= timerInterval) {
    lastCheckMillis = currentMillis;
    checkTime();
  }

  WiFiClient client = server.available();
  if (client) {
    handleClient(client);
  }
}
