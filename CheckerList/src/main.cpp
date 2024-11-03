#include "main.h"

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

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
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
	tft.fillScreen(TFT_BLACK);
	tft.setTextSize(1);
	tft.println("Hello World!");

	Enc1.begin();
	Enc1.setup(readEnc1ISR);
	Enc2.begin();
	Enc2.setup(readEnc2ISR);
	Enc1.setBoundaries(0, 360, false);
	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, password);
	if (WiFi.waitForConnectResult() != WL_CONNECTED) {
		Serial.println("WiFi Failed!");
		return;
	}
	timeClient.begin();
	timeClient.setTimeOffset(28800);
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
String formattedDate;
struct tm timeinfo;
static char str[20];
uint8_t butt_curr_state = 0, butt_last_state = 0;
Timer_state time_state = CHANGING_TIME, last_time_state = CHANGING_TIME;
uint8_t do_once_flag = 1;
Timer timer = {.minutes = 0, .seconds = 0};
uint32_t last_tick = 0, timer_tick = 0;
void timer_fsm() {
	if (last_time_state != time_state) {
		do_once_flag = 1;
	}
	switch (time_state) {
		case CHANGING_TIME:
			if (do_once_flag) {
				Enc1.setEncoderValue(timer.minutes * 6 + timer.seconds / 10);
				do_once_flag = 0;
			}
			timer.minutes = Enc1.readEncoder() / 6;
			timer.seconds = Enc1.readEncoder() % 6 * 10;
			break;
		case TIMING:
			if (do_once_flag) {
				do_once_flag = 0;
			}
			if (millis() - timer_tick > 1000) {
				timer.minutes = (timer.seconds == 0) ? timer.minutes - 1 : timer.minutes;
				timer.seconds = (timer.seconds == 0) ? 60 : timer.seconds;
				if (timer.seconds == 60 && timer.minutes == -1) {
					time_state = CHANGING_TIME;
					timer.minutes = 0;
					timer.seconds = 10;
				}
				timer.seconds--;
				timer_tick = millis();
			}
			break;
	}
	last_time_state = time_state;
}
void loop() {
	if (millis() - last_tick > 100) {
		timer_fsm();
		butt_curr_state = Enc1.isEncoderButtonDown();
		if (butt_curr_state && !butt_last_state) {
			time_state = (Timer_state)!time_state;
		}
		timeClient.update();
		sprintf(str, "%02d:%02d", timeClient.getHours(), timeClient.getMinutes());
		if (timer.minutes * 6 + timer.seconds / 10 != 360)
			tft.drawSmoothArc(80, 160, 80, 60, timer.minutes * 6 + timer.seconds / 10 - 1, 359, TFT_BLACK, TFT_BLACK,
							  0);
		if (timer.minutes * 6 + timer.seconds / 10 != 0)
			tft.drawSmoothArc(80, 160, 80, 60, 0, timer.minutes * 6 + timer.seconds / 10, TFT_ORANGE, TFT_BLACK, 0);
		tft.setTextSize(5);
		tft.setCursor(0, 20, 1);
		tft.setTextColor(TFT_WHITE, TFT_BLACK);
		tft.println(str);
		tft.setCursor(40, 150, 1);
		tft.setTextSize(3);
		sprintf(str, "%02d:%02d", timer.minutes, timer.seconds);
		tft.println(str);
		last_tick = millis();
		butt_last_state = butt_curr_state;
	}
}
