#include <DHT.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <ChaCha.h>

#define DHTPIN 4       // Pin connected to the DHT sensor
#define DHTTYPE DHT11  // DHT sensor type

const char* ssid = "T4";
const char* password = "END#skripsi#8055";
const char* serverUrl = "http://178.128.119.144:999/post_esp_sensor1.php"; // Change with your server URL
String apiKeyValue = "tPmAT5Ab3j7F9";

DHT dht(DHTPIN, DHTTYPE);

ChaCha  CC20(20);
const byte CC20Key[32] =
{
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
  0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
  0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
};
const byte CC20Nonce[12] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4a, 0x00, 0x00, 0x00, 0x00};
const byte CC20Counter[4] = {0x01, 0x00, 0x00, 0x00};  // Little-endian 4 or 8 byte integer

String getIndicatorColor(float temperature) {
  if (temperature > 40) {
    return "merah";
  } else if (temperature >= 30 && temperature <= 40) {
    return "kuning";
  } else {
    return "hijau";
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi...");
  }
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Read temperature and humidity from DHT11
   if(WiFi.status()== WL_CONNECTED){
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Create a JSON object
  StaticJsonDocument<200> doc;

 //Mengambil indicator color
 String indicatorColor = getIndicatorColor(temperature);

  // Add temperature and humidity data to the JSON object
  doc["location"] = "Ruang Server";
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;
  doc["indikator"] = indicatorColor;

  // Serialize the JSON object to a string
  String jsonString;
  serializeJson(doc, jsonString);
  Serial.println(jsonString);
    
  // Enkripsi data dengan ChaCha20
  // Convert the String to a byte array
  size_t bufferSize = jsonString.length() + 1; // +1 for null terminator
  uint8_t buffer[bufferSize];
  jsonString.getBytes(buffer, bufferSize);
  
  byte Plaintext[bufferSize];
//  int PlaintextSize = (sizeof buffer) - 1; // Leave out the terminating null
  byte Ciphertext[bufferSize];
  CC20.clear();
  CC20.setKey(CC20Key, 32);
  CC20.setIV(CC20Nonce, 12);
  CC20.setCounter(CC20Counter, 4);
  CC20.encrypt(Ciphertext, Plaintext, bufferSize);
    Serial.print("cipher: ");
  for (int i = 0; i < bufferSize; i++)
  {
    Serial.print((Ciphertext[i]),HEX);
  }

  
  // Send the JSON data to the server
  HTTPClient http;

  // Make a POST request
  http.begin(serverUrl);
  http.addHeader("Content-Type", "application/json");

  String post_data = "api_key=" + apiKeyValue + "&encStr=" + Ciphertext[bufferSize] + "";
  Serial.println(post_data);
  
  int httpResponseCode = http.POST(post_data);

  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
  } else {
    Serial.println("Error on sending POST request");
  }

  // Close the connection
  http.end();
    }
  else {
    Serial.println("WiFi Disconnected");
  }

  // Wait for some time before sending the next request
  delay(5000);

}
