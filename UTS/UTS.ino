#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <HTTPClient.h> 

const char* ssid = "Kost ema eksklusif";
const char* password = "gejayan251124";
const char* serverName = "http://192.168.18.10/insert.php";

unsigned long lastTime = 0;
unsigned long timerDelay = 10000; // 10 detik interval

#define DHTPIN 4
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);
WebServer server(80);

// Fungsi pembaca suhu
String readDHTTemperature() {
  float t = dht.readTemperature();
  if (isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return "--";
  } else {
    Serial.println("Temperature: " + String(t));
    return String(t, 1);
  }
}

// Fungsi pembaca kelembaban
String readDHTHumidity() {
  float h = dht.readHumidity();
  if (isnan(h)) {
    Serial.println("Failed to read from DHT sensor!");
    return "--";
  } else {
    Serial.println("Humidity: " + String(h));
    return String(h, 1);
  }
}

// HTML template
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0/css/all.min.css">
  <style>
    html {
      font-family: Arial;
      text-align: center;
    }
    h2 {
      font-size: 2.5rem;
    }
    p {
      font-size: 2.0rem;
    }
    .units {
      font-size: 1.2rem;
    }
    .dht-labels {
      font-size: 1.5rem;
      padding-bottom: 15px;
    }
  </style>
</head>
<body>
  <h2>ESP32 DHT Server</h2>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i>
    <span class="dht-labels">Temperature</span>
    <span id="temperature">%TEMPERATURE%</span>
    <sup class="units">&deg;C</sup>
  </p>
  <p>
    <i class="fas fa-tint" style="color:#00add6;"></i>
    <span class="dht-labels">Humidity</span>
    <span id="humidity">%HUMIDITY%</span>
    <sup class="units">%</sup>
  </p>
</body>
<script>
setInterval(function () {
  fetch("/temperature").then(response => response.text()).then(data => {
    document.getElementById("temperature").innerHTML = data;
  });
}, 10000);

setInterval(function () {
  fetch("/humidity").then(response => response.text()).then(data => {
    document.getElementById("humidity").innerHTML = data;
  });
}, 10000);
</script>
</html>)rawliteral";

// Fungsi untuk replace placeholder HTML
String processor(const String& var) {
  if (var == "TEMPERATURE") return readDHTTemperature();
  else if (var == "HUMIDITY") return readDHTHumidity();
  return String();
}


void setup() {
  Serial.begin(115200);
  dht.begin();

  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi. IP address: ");
  Serial.println(WiFi.localIP());

  // Route halaman utama
  server.on("/", HTTP_GET, []() {
    String html = index_html;
    html.replace("%TEMPERATURE%", readDHTTemperature());
    html.replace("%HUMIDITY%", readDHTHumidity());
    server.send(200, "text/html", html);
  });

  // Route suhu
  server.on("/temperature", HTTP_GET, []() {
    server.send(200, "text/plain", readDHTTemperature());
  });

  // Route kelembaban
  server.on("/humidity", HTTP_GET, []() {
    server.send(200, "text/plain", readDHTHumidity());
  });

  server.begin();
  
}

void postDataToServer() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    String temperature = readDHTTemperature();
    String humidity = readDHTHumidity();

    // Cek kalau datanya valid
    if (temperature == "--" || humidity == "--") {
      Serial.println("Sensor error, tidak kirim data.");
      return;
    }

    http.begin(serverName);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    String httpRequestData = "temperature=" + temperature + "&humidity=" + humidity;
    Serial.println("Posting data: " + httpRequestData);

    int httpResponseCode = http.POST(httpRequestData);

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("WiFi disconnected");
  }
}

void loop() {
  server.handleClient();
  unsigned long currentTime = millis();
  if (currentTime - lastTime >= timerDelay) {
    lastTime = currentTime;
    postDataToServer();
  }
}
