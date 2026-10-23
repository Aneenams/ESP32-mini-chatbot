#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// --- FILL IN YOUR DETAILS HERE ---
const char* ssid = "YOUR SSID";
const char* password = "YOUR PASSWORD";
const char* apiKey = "YOUR API KEY";
// ---------------------------------

const String apiUrl = "https://generativelanguage.googleapis.com/v1beta/models/gemini-3.5-flash-lite:generateContent?key=";

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Variables for smooth, non-blocking eye blinking
unsigned long lastBlinkTime = 0;
const long blinkInterval = 3500; // Blink every 3.5 seconds

void setup() {
  Serial.begin(115200);
  delay(1000);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println("Screen allocation failed.");
    for(;;); 
  }
  
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  
  updateScreen("Waking up...");
  
  WiFi.disconnect(true);
  delay(1000);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }
  
  Serial.println("\n*** BOT IS READY ***");
  Serial.println("Type your question and press Enter...");
  
  drawEyes(); // Draw open eyes
}

void loop() {
  // 1. Check if you typed a question in the Serial Monitor
  if (Serial.available() > 0) {
    String userPrompt = Serial.readStringUntil('\n'); 
    userPrompt.trim();

    if (userPrompt.length() > 0) {
      Serial.print("\n>>> YOU: ");
      Serial.println(userPrompt);
      askGemini(userPrompt);
    }
  }

  // 2. Idle Animation: Make the eyes blink automatically every few seconds
  unsigned long currentTime = millis();
  if (currentTime - lastBlinkTime >= blinkInterval) {
    lastBlinkTime = currentTime;
    
    // Quick blink action
    blinkEyes();
    drawEyes();
  }
}

void askGemini(String prompt) {
  if (WiFi.status() == WL_CONNECTED) {
    
    // Show the visual thinking face instead of text!
    drawThinkingFace();
    
    WiFiClientSecure client;
    client.setInsecure(); 
    
    HTTPClient http;
    http.setTimeout(40000); 
    
    String url = apiUrl + apiKey;
    http.begin(client, url);
    http.addHeader("Content-Type", "application/json");

    String payload = "{\"contents\": [{\"parts\":[{\"text\": \"" + prompt + "\"}]}]}";
    int httpResponseCode = http.POST(payload);

    if (httpResponseCode > 0) {
      String response = http.getString();
      JsonDocument doc; 
      deserializeJson(doc, response);

      const char* answer = doc["candidates"][0]["content"]["parts"][0]["text"];
      String ansStr = answer ? String(answer) : "(no text)";
      
      updateScreen(ansStr);
      Serial.println("\n=== GEMINI SAYS ===");
      Serial.println(ansStr);
      
      // Wait 15 seconds for you to read the answer before returning to normal eyes
      delay(15000);
      drawEyes();
      
    } else {
      updateScreen("Network Error");
      delay(3000);
      drawEyes();
    }
    http.end(); 
  }
}

// Custom function to draw a "Thinking" face (Eyes shifted up with brows)
void drawThinkingFace() {
  display.clearDisplay();
  
  // Left eye
  display.fillRoundRect(25, 18, 26, 24, 4, SSD1306_WHITE);
  // Right eye (shifted slightly to the right to look sideways/puzzled)
  display.fillRoundRect(79, 18, 26, 24, 4, SSD1306_WHITE);
  
  // Thinking eyebrows (slanted upward/asymmetrical for a curious look)
  // Left eyebrow tilted up
  display.drawLine(25, 13, 45, 9, SSD1306_WHITE);
  // Right eyebrow raised higher
  display.drawLine(79, 9, 99, 14, SSD1306_WHITE);
  
  // A slight smirk / wavy mouth line to show pondering
  display.drawLine(58, 48, 70, 46, SSD1306_WHITE);

  display.setTextSize(1);
  display.setCursor(102, 48); // Places it near the bottom right corner
  display.print("...");
    
  display.display();
}
// Function to draw text
void updateScreen(String text) {
  display.clearDisplay();      
  display.setCursor(0, 0);     
  display.println(text);       
  display.display();           
}

// Function to draw open robot eyes
void drawEyes() {
  display.clearDisplay();
  display.fillRoundRect(25, 18, 28, 28, 6, SSD1306_WHITE); // Left eye
  display.fillRoundRect(75, 18, 28, 28, 6, SSD1306_WHITE); // Right eye
  display.display();
}

// Function to make the eyes blink shut briefly
void blinkEyes() {
  display.clearDisplay();
  display.fillRoundRect(25, 30, 28, 4, 2, SSD1306_WHITE); // Left closed
  display.fillRoundRect(75, 30, 28, 4, 2, SSD1306_WHITE); // Right closed
  display.display();
  delay(120); // Keep them shut for 0.12 seconds
}