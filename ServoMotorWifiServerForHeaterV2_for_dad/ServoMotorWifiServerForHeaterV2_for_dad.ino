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

[ ] Resoulder other esp32 to headers?
[ ] Find a power supply that is: 2500mA output 5-12Volts
[ ] New 5v regulator LM785
https://www.jaycar.com.au/7805-5v-1a-voltage-regulator-to-220-case/p/ZV1505?srsltid=AfmBOorxDBXAPCxJBhZxmMGVcjPbzGa2Dt5_nABtKZ0WKYEHeH0aqbdP

output tab is one thing, Serial Monitor tab is another. Watch this one for web server output like ip:
http://192.168.86.30
 
 */

#include <WiFi.h>
#include <ESP32Servo.h>  // Include the ESP32Servo library
// #include <vector>

Servo myservo;  // Create a servo object
int homePosition = 90;
int buttonPressPosition = 43;
int defaultDurationInMins = 20;
//int currentPosition = homePosition; 
bool currentState = false;

const int servoPin = 21;  // Define the GPIO pin connected to the servo signal wire

const char* ssid = "Marmalade";
const char* password = "100%Minnie!!";

struct Event {
  bool newState;
  unsigned long timeToChange;
  bool hasFired;
};

Event currentPendingEvent = {true, 0, true};
unsigned long timeOfMostRecentOn = 0;

// std::vector<Event> events;

WiFiServer server(80);

void setup()
{
    Serial.begin(115200);
    Serial.println('setup');
    myservo.attach(servoPin);  // Attach the servo to the specified pin

    delay(10);

    // We start by connecting to a WiFi network

    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    
    server.begin();

    currentPendingEvent ;
}

void loop(){
  WiFiClient client = server.available();   // listen for incoming clients

  int newPosition = -1;

  if (client) {                             // if you get a client,
    Serial.println("New Client.");          // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    bool wasApiRequest = false;
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        // Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            if (!wasApiRequest) {
              // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
              // and a content-type so the client knows what's coming, then a blank line:
              client.println("HTTP/1.1 200 OK");
              client.println("Content-type:text/html");
              client.println();

              // the content of the HTTP response follows the header:
              client.println("<html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no\"></head>");
              client.println("<style>button{min-height:50px; cursor: pointer;}</style>");
              client.println("<body>");
              client.println("<h1>Heater</h1>");
              client.printf("<p>Uptime: <span data-date=\"%lu\" data-dateDirection=\"1\" id=\"uptimeFill\"></span></p>", millis()); 
              if (!currentPendingEvent.hasFired) {
                client.println("<p>Ontime: <span id=\"ontimeFill\"></span></p>"); 
                client.printf("<p>Countdown: <span data-date=\"%lu\"data-dateDirection=\"0\" id=\"countdownFill\"></span></p>", currentPendingEvent.timeToChange - millis()); 
                client.printf("<p>State upon countdown: %d</p>", currentPendingEvent.newState); 
              }
              client.println("<form id=\"servoForm\" action=\"/\" method=\"GET\">");
              client.printf("Enter a value (0-180): <input type=\"number\" name=\"position\" min=\"0\" max=\"180\" value=\"%d\"><br><br>", buttonPressPosition);
              client.printf("Enter duration in minutes: <input type=\"number\" name=\"duration\" min=\"0\" step=\"5\" value=\"%d\"><br><br>", defaultDurationInMins);
              //client.println("<input type=\"submit\" value=\"Submit\">");
              client.println("</form>");
              client.println("<div style=\"display: grid; grid-template-columns: 1fr 1fr 1fr; gap: 20px;\">");
              client.println("<button id=\"onButton\">On</button>");
              client.println("<button id=\"toggleButton\">Toggle</button>");
              client.println("<button id=\"offButton\">Off</button>");
              client.println("</div>");
              client.println("<button id=\"showerButton\" style=\"margin: 20px 0px; width: 100%;\">10 min shower</button>");

              client.println("<script src=\"https://joshprojects.site/othersLibraries/chroma.min.js\"></script>");
              client.println("<script>");
              client.println("function formatTime(ms) {");
              client.println("  let w = Math.floor(ms / 604800000); ms %= 604800000;");
              client.println("  let d = Math.floor(ms / 86400000); ms %= 86400000;");
              client.println("  let h = Math.floor(ms / 3600000); ms %= 3600000;");
              client.println("  let m = Math.floor(ms / 60000); ms %= 60000;");
              client.println("  let s = Math.floor(ms / 1000);");
              client.println("  return `${w}w ${d}d ${h}h ${m}m ${s}s`;");
              client.println("}");
              client.println("let dateElements = document.querySelectorAll('[data-date]');");
              client.println("let pageLoadTime = new Date();");
              client.printf("let timeOfMostRecentOn = %d;", timeOfMostRecentOn);
              client.println("console.log(timeOfMostRecentOn);");
              client.println("let body = document.querySelector('body');");
              client.println("let updateDates = ()=>{");
              client.println("  console.log('firing');");
              
              client.println("  let intervalFireTime = new Date();");
              client.println("  let intervalFireDifferenceFromLoadInMs = intervalFireTime - pageLoadTime;");
              client.println("  dateElements.forEach(dateElement => {");
              client.println("      let originalDate = +dateElement.getAttribute('data-date');");
              client.println("      let currentDate;");
              client.println("      let dateDirection = +dateElement.getAttribute('data-dateDirection');");
              client.println("      if (dateDirection==1) {");
              client.println("        currentDate = originalDate + intervalFireDifferenceFromLoadInMs");
              client.println("      } else {");
              client.println("        currentDate = originalDate - intervalFireDifferenceFromLoadInMs");
              client.println("      }");
              client.println("      dateElement.textContent = formatTime(currentDate);");
              client.println("      if (dateElement.id === 'countdownFill') {");

              client.println("        let onDuration = +document.querySelector('#uptimeFill').getAttribute('data-date') - timeOfMostRecentOn + intervalFireDifferenceFromLoadInMs");
              client.println("        console.log('---');");
              client.println("        console.log(+document.querySelector('#uptimeFill').getAttribute('data-date'));");
              client.println("        console.log(timeOfMostRecentOn);");
              client.println("        console.log(intervalFireDifferenceFromLoadInMs);");
              client.println("        console.log(onDuration);");
              client.println("        document.querySelector('#ontimeFill').textContent = formatTime(onDuration);");

              client.println("        let countdownMs = currentDate;");
              client.println("        // 10 mins");
              client.println("        let letOnThreshold = 10 * 60000;");
              client.println("        let lowerIdealCountdown = 10 * 60000;");
              client.println("        let upperIdealCountdown = 15 * 60000;");
              client.println("        if (onDuration > letOnThreshold) {");
              client.println("          if (countdownMs > lowerIdealCountdown && countdownMs < upperIdealCountdown) {");
              client.println("            // red, it's hot because there's the ideal amount of time remaining");     
              client.println("            body.style.backgroundColor = '#ffe5db';");                
              client.println("          } else {");
              client.println("            // yellow, it's hot but since there's the ideal amount of time remaining you should set it to go for 10 more mins");     
              client.println("            body.style.backgroundColor = '#f8ffdb';");   
              client.println("          }");
              client.println("        } else {");
              client.println("          body.style.backgroundColor = '#edf8ff';");
              client.println("        }");

              client.println("        if (countdownMs < 0) {");
              client.println("          window.location.reload();");
              client.println("        }");

              client.println("      }");
              client.println("  });");
              client.println("};");
              client.println("updateDates();");
              client.println("setInterval(updateDates, 1000);");

              client.println("async function submitData(newState, duration = null) {");
              client.println("  console.log('Submitting data');");
              client.println("  console.log(`The state is ${newState}`);");
              client.println("  const form = document.getElementById('servoForm');");
              client.println("  const formData = new FormData(form);");
              client.println("  const position = formData.get('position');");
              client.println("  console.log(`The position is ${position}`);");
              client.println("  if (duration == null) {");
              client.println("    duration = formData.get('duration');");
              client.println("  }");
              client.println("  console.log(`The duration is ${duration}`);");
              client.println("  let response = await fetch(`http://" + WiFi.localIP().toString() + "/api/${newState}/${position}/${duration}`);");
              client.println("  let value = await response.json();");
              client.println("  console.log(value);");
              client.println("  alert(JSON.stringify(value));");
              client.println("  window.location.reload();");
              client.println("}");


              client.println("document.getElementById('servoForm').addEventListener('submit', async function(event) {");
              client.println("  event.preventDefault();");
              client.println("});");
              client.println("document.getElementById('onButton').addEventListener('click', function() { submitData('on'); });");
              client.println("document.getElementById('toggleButton').addEventListener('click', function() { submitData('toggle'); });");
              client.println("document.getElementById('offButton').addEventListener('click', function() { submitData('off'); });");
              client.println("document.getElementById('showerButton').addEventListener('click', function() { submitData('on',10); });");
              client.println("</script>");

              client.println("</body></html>");
            } else {
              client.println("HTTP/1.1 200 OK");
              client.println("Content-type:application/json");
              client.println();
              client.println("{\"oldState\": \""+ String(currentState) + "\", \"newPosition\": \""+ String(newPosition) +"\", \"currentPendingHasFired\": \"" + currentPendingEvent.hasFired + "\", \"currentPendingTimeToChange\": \"" + currentPendingEvent.timeToChange + "\", \"currentPendingNewState\": \"" + currentPendingEvent.newState + "\"}");
              client.println();
            }
            

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else {    
            if (currentLine.startsWith("GET /api/")) {
              wasApiRequest = true;
              String trimmed = currentLine.substring(9);
              Serial.println(trimmed);

              // Find the index of the first space
              int spaceIndex = trimmed.indexOf(' ');
              
              // If there is a space, truncate the string to remove everything after it
              if (spaceIndex != -1) {
                  trimmed = trimmed.substring(0, spaceIndex);
              }

              int firstSlashIndex = trimmed.indexOf('/');
              String newStateString = trimmed.substring(0, firstSlashIndex); 
              trimmed = trimmed.substring(firstSlashIndex + 1); 

              int secondSlashIndex = trimmed.indexOf('/');
              String positionString = trimmed.substring(0, secondSlashIndex);
              String durationString = trimmed.substring(secondSlashIndex + 1);

              int positionValue = positionString.toInt();
              // If the parsed value is 0 but the original value wasn't "0", then that means that the parsing to int has failed 
              // and it should default to the most recently set value
              if (positionValue == 0 && positionString != "0") {
                positionValue = buttonPressPosition;
              }

              int durationValue = durationString.toInt();
              // If the parsed value is 0 but the original value wasn't "0", then that means that the parsing to int has failed 
              // and it should default to the most recently set value
              if (durationValue == 0 && durationString != "0") {
                durationValue = defaultDurationInMins;
              }

              // default to turning off
              bool newState = false;
              if (newStateString == "toggle") {
                newState = !currentState;
              } else if (newStateString == "on") {
                newState = true;
              }

              if (positionValue > 0 && positionValue <= 180) {
                buttonPressPosition = positionValue;  
                defaultDurationInMins = durationValue;
                if (newState) {
                  // set newPosition so that it changes state right now
                  if (newState != currentState) {
                    newPosition = buttonPressPosition;
                    // only set this when it's newly being turned on, so if you extend then it persists
                    timeOfMostRecentOn = millis();
                  }
                  // and then set the currentPendingEvent so that it turns itself off later
                  currentPendingEvent = { false, millis() + durationValue * 60000, false };
                } else {
                  // when turning off, we use the currentPendingEvent so that it overrides any existing currentPendingEvent
                  currentPendingEvent = { false, millis() - 1, false };
                }
              }                
            }
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }        
      }
    }
    // close the connection:
    client.stop();
    Serial.println("Client Disconnected.");
  }

  // process the pending event if it's time
  if (!currentPendingEvent.hasFired & millis() >= currentPendingEvent.timeToChange) {
    if (currentPendingEvent.newState != currentState) {
      newPosition = buttonPressPosition;
    }
    currentPendingEvent.hasFired = true;
  }

  // Move the servo to the new position if a valid new position was received
  if (newPosition != -1 && newPosition != homePosition) {
    myservo.write(newPosition); 
    delay(600); 
    myservo.write(homePosition); 
    currentState = !currentState;
  }
}
