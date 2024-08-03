#include <SoftwareSerial.h>
#include <DHT.h>

#define SOUND_SENSOR_PIN 2
#define PIR_SENSOR_PIN 3
#define DHT_PIN 4
#define DHT_TYPE DHT11

SoftwareSerial sim900(7, 8); // RX, TX for SIM900 module
DHT dht(DHT_PIN, DHT_TYPE);

bool soundDetected = false;
bool motionDetected = false;

void setup() {
  pinMode(SOUND_SENSOR_PIN, INPUT);
  pinMode(PIR_SENSOR_PIN, INPUT);
  Serial.begin(9600);
  sim900.begin(9600);
  dht.begin();

  Serial.println("Initializing SIM900A...");
  delay(10000); // Wait for GSM module to initialize

  sim900.println("AT+CMGF=1"); // Set SMS to text mode
  delay(100);
}

void loop() {
  soundDetected = digitalRead(SOUND_SENSOR_PIN);
  motionDetected = digitalRead(PIR_SENSOR_PIN);

  Serial.print("Sound Detected: ");
  Serial.println(soundDetected);
  Serial.print("Motion Detected: ");
  Serial.println(motionDetected);

  if (soundDetected && motionDetected) {
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();
    
    if (isnan(temperature) || isnan(humidity)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }

    String msg = "Alert! Baby movement and sound is detected. Temp: " + String(temperature) + " C, Humidity: " + String(humidity) + "%";
    
    SendMessage(msg);

    delay(5000); // Shorter delay before making the call to avoid rapid successive calls
    makeCall("+919019845647"); // Replace with your phone number
  }

  delay(1000); // Main loop delay
}

void SendMessage(String msg) {
  sim900.println("AT+CMGF=1"); // Set the GSM module to text mode
  delay(500); // Shorter delay for SMS settings

  sim900.print("AT+CMGS=\"+919019845647\"\r"); // Replace with recipient's mobile number
  delay(500);

  sim900.println(msg); // The SMS text you want to send
  delay(100);
  sim900.write(26); // ASCII code of CTRL+Z to send the SMS
  delay(2000); // Wait longer to ensure SMS is sent
}

void makeCall(const char* phoneNumber) {
  Serial.print("Dialing: ");
  Serial.println(phoneNumber);

  sim900.print("ATD");
  sim900.print(phoneNumber);
  sim900.println(";");
  delay(1000); // Delay to allow the call to initiate

  unsigned long startTime = millis();
  while (millis() - startTime < 10000) { // Wait up to 10 seconds for a response
    if (sim900.available()) {
      String response = sim900.readString();
      Serial.println(response);
      if (response.indexOf("OK") != -1) {
        Serial.println("Call initiated successfully.");
        return;
      }
      if (response.indexOf("ERROR") != -1) {
        Serial.println("Error in making call.");
        return;
      }
    }
  }

  Serial.println("No response or unknown response from SIM900A.");
}
