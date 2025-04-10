// Emulador de NE555 en ESP32 con WiFi, cálculo de modo Astable y Monostable
// Incluye dos salidas LED para indicar el modo actual (GPIO26 y GPIO27)

#include <WiFi.h>
#include <WebServer.h>

// Credenciales WiFi
const char* ssid = "TU_SSID";
const char* password = "TU_PASSWORD";

WebServer server(80);

// Pines para LEDs
#define LED_ASTABLE 26
#define LED_MONO 27

void setup() {
  Serial.begin(115200);

  pinMode(LED_ASTABLE, OUTPUT);
  pinMode(LED_MONO, OUTPUT);

  digitalWrite(LED_ASTABLE, LOW);
  digitalWrite(LED_MONO, LOW);

  // Conexión WiFi
  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado");
  Serial.println(WiFi.localIP());

  // Página principal con formulario
  server.on("/", HTTP_GET, []() {
    String html = "<html><body>"
      "<h2>Simulador de NE555</h2>"
      "<form action='/simulate' method='get'>"
      "Modo: <select name='mode'>"
      "<option value='astable'>Astable</option>"
      "<option value='monostable'>Monostable</option>"
      "</select><br><br>"
      "R1 (Ohm): <input type='number' name='r1'><br>"
      "R2 (Ohm): <input type='number' name='r2'><br>"
      "C (uF): <input type='number' step='0.01' name='c'><br><br>"
      "<input type='submit' value='Ejecutar'>"
      "</form><br>"
      "<form action='/stop' method='get'>"
      "<input type='submit' value='Detener'>"
      "</form>"
      "</body></html>";
    server.send(200, "text/html", html);
  });

  // Ruta para ejecutar simulación
  server.on("/simulate", HTTP_GET, []() {
    String mode = server.arg("mode");
    float r1 = server.arg("r1").toFloat();
    float r2 = server.arg("r2").toFloat();
    float c = server.arg("c").toFloat();
    c /= 1000000.0; // Convertimos uF a Faradios

    String response = "";

    if (mode == "astable") {
      float T = 0.693 * (r1 + 2 * r2) * c;
      float f = 1.0 / T;
      float duty = (r1 + r2) / (r1 + 2 * r2) * 100;

      response = "<h3>Modo Astable</h3>";
      response += "Frecuencia: " + String(f, 2) + " Hz<br>";
      response += "Ciclo de trabajo: " + String(duty, 2) + " %<br>";

      digitalWrite(LED_ASTABLE, HIGH);
      digitalWrite(LED_MONO, LOW);
    } else {
      float T = 1.1 * r1 * c;
      response = "<h3>Modo Monostable</h3>";
      response += "Tiempo de pulso: " + String(T, 3) + " segundos<br>";

      digitalWrite(LED_MONO, HIGH);
      digitalWrite(LED_ASTABLE, LOW);
    }

    response += "<br><a href='/'>Volver</a>";
    server.send(200, "text/html", response);
  });

  // Ruta para detener simulación y apagar LEDs
  server.on("/stop", HTTP_GET, []() {
    digitalWrite(LED_ASTABLE, LOW);
    digitalWrite(LED_MONO, LOW);
    server.send(200, "text/plain", "Simulación detenida");
  });

  server.begin();
  Serial.println("Servidor HTTP iniciado");
}

void loop() {
  server.handleClient();
}
