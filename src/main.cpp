#include <Arduino.h>
#include <WiFi.h>

const char* ssid = "Skinny";
const char* password = "vinicius";

// separamento
const int trigPin = 5;
const int echoPin = 18;
const int relePin = 21;

//define sound speed in cm/uS
#define SOUND_SPEED 0.034
#define CM_TO_INCH 0.393701

const float dist_limite = 20.0;

long duration;
float distanceCm;
float distanceInch;
bool status;

WiFiServer server(80);


void setup() {
  Serial.begin(115200); // Starts the serial communication
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input
  pinMode(relePin, OUTPUT);
  digitalWrite(relePin, HIGH);

  WiFi.begin(ssid, password);
  Serial.print("Conectando ao Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi Conectado!");
  Serial.print("Endereço IP do ESP32: http://");
  Serial.println(WiFi.localIP());

  server.begin();
  Serial.println("Servidor HTTP iniciado com sucesso.");
}

void loop() {
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  
  // Calculate the distance
  distanceCm = duration * SOUND_SPEED/2;
  
  // Convert to inches
  distanceInch = distanceCm * CM_TO_INCH;
  
  // Prints the distance in the Serial Monitor
  Serial.print("Distance (cm): ");
  Serial.println(distanceCm);
  Serial.print("Distance (inch): ");
  Serial.println(distanceInch);

  if (distanceCm > 0 && distanceCm <=dist_limite ) {
    digitalWrite(relePin, LOW);
    status = true;
  }
  else {
    digitalWrite(relePin, HIGH);
    status = false;
  }

  WiFiClient client = server.available();
  if (client) {
    String currentLine = "";
    while (client.connected()) {
      if (client.available()) { 
        char c = client.read();
        if (c == '\n') {
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html; charset=utf-8");
            client.println("Connection: close");
            client.println();

            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<meta http-equiv=\"refresh\" content=\"2\">"); 
            client.println("<style>body{font-family:Arial; text-align:center; margin-top:50px;}");
            client.println(".status{font-size:28px; font-weight:bold; padding:15px; display:inline-block; border-radius:8px;}");
            client.println(".on{background-color:#4CAF50; color:white;}");
            client.println(".off{background-color:#f44336; color:white;}</style></head>");
            client.println("<body>");
            client.println("<h1>Painel IoT - Status da Luz</h1>");
            client.print("<p>Distância Atual: <strong>");
            client.print(distanceCm);
            client.println(" cm</strong></p>");
            
            if (status) {
              client.println("<p>Estado do Interruptor: <span class=\"status on\">LIGADA</span></p>");
            } else {
              client.println("<p>Estado do Interruptor: <span class=\"status off\">DESLIGADA</span></p>");
            }

            client.println("</body></html>");
            client.println();
            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }
    client.stop();
  }

  delay(200);

}
