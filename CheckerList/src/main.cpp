#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>

#include "my_home_pw.h"
// Replace with your network credentials
AsyncWebServer server(80);	// REPLACE WITH YOUR NETWORK CREDENTIALS
#include <SPI.h>
#include <TFT_eSPI.h>

#include "AiEsp32RotaryEncoder.h"
AiEsp32RotaryEncoder Enc1 = AiEsp32RotaryEncoder(15, 4, 2, -1, 2);
AiEsp32RotaryEncoder Enc2 = AiEsp32RotaryEncoder(13, 14, 12, -1, 2);
TFT_eSPI tft = TFT_eSPI(320, 480);

void IRAM_ATTR readEnc1ISR() { Enc1.readEncoder_ISR(); }
void IRAM_ATTR readEnc2ISR() { Enc2.readEncoder_ISR(); }
const char* PARAM_INPUT_1 = "input1";
const char* PARAM_INPUT_2 = "input2";
const char* PARAM_INPUT_3 = "input3";  // HTML web page to handle 3 input fields (input1, input2, input3)
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>ESP Input Form</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  <form action="/get">
    input1: <input type="text" name="input1"style="width: 50px">
    |<input type="text" name="input2">
    <input type="submit" value="Submit">
  </form><br>
</body></html>)rawliteral";
void notFound(AsyncWebServerRequest* request) { request->send(404, "text/plain", "Not found"); }
void setup() {
	Serial.begin(115200);

	tft.init();
	tft.setRotation(3);
	tft.setCursor(0, 0, 2);
	tft.fillScreen(TFT_BLACK);
	tft.setTextColor(TFT_WHITE, TFT_BLACK);
	tft.setTextSize(1);
	tft.println("Hello World!");

	Enc1.begin();
	Enc1.setup(readEnc1ISR);
	Enc2.begin();
	Enc2.setup(readEnc2ISR);
	while (1) {
		Serial.print("ENC1:");
		Serial.print(Enc1.readEncoder());
		Serial.print("ENC2:");
		Serial.println(Enc2.readEncoder());
	}
	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, password);
	if (WiFi.waitForConnectResult() != WL_CONNECTED) {
		Serial.println("WiFi Failed!");
		return;
	}
	Serial.println();
	Serial.print("IP Address: ");
	Serial.println(WiFi.localIP());
	// Send web page with input fields to client
	server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) { request->send_P(200, "text/html", index_html); });
	// Send a GET request to <ESP_IP>/get?input1=<inputMessage>
	server.on("/get", HTTP_GET, [](AsyncWebServerRequest* request) {
		String inputMessage;
		String inputParam;
		// GET input1 value on <ESP_IP>/get?input1=<inputMessage>
		if (request->hasParam(PARAM_INPUT_1)) {
			inputMessage = request->getParam(PARAM_INPUT_1)->value();
			inputParam = PARAM_INPUT_1;
		}
		Serial.println(inputMessage);
		// GET input2 value on <ESP_IP>/get?input2=<inputMessage>
		if (request->hasParam(PARAM_INPUT_2)) {
			inputMessage = request->getParam(PARAM_INPUT_2)->value();
			inputParam = PARAM_INPUT_2;
		}
		Serial.println(inputMessage);
		request->send_P(200, "text/html", index_html);
	});
	server.onNotFound(notFound);
	server.begin();
}
void loop() {}
