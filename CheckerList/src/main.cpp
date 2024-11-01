#include <WiFi.h>
#include "AiEsp32RotaryEncoder.h"
#include <TFT_eSPI.h>
#include <SPI.h>
// Replace with your network credentials
const char* ssid = "ESP32-Access-Point";
const char* password = "123456789";
// Set web server port number to 80
WiFiServer server(80);
AiEsp32RotaryEncoder Enc1 = AiEsp32RotaryEncoder(15, 4, 2, -1, 1);
AiEsp32RotaryEncoder Enc2 = AiEsp32RotaryEncoder(13, 14, 12, -1, 1);
TFT_eSPI tft = TFT_eSPI(320,480);   

void IRAM_ATTR readEnc1ISR()
{
	Enc1.readEncoder_ISR();
}
void IRAM_ATTR readEnc2ISR()
{
	Enc2.readEncoder_ISR();
}

void setup() {
	Serial.begin(115200);
	
	tft.init();
	tft.setRotation(3);
	tft.setCursor(0, 0, 2);  tft.fillScreen(TFT_BLACK);
	tft.setTextColor(TFT_WHITE,TFT_BLACK);  tft.setTextSize(1);
	tft.println("Hello World!");
	
	Enc1.begin();
	Enc1.setup(readEnc1ISR);
	Enc2.begin();
	Enc2.setup(readEnc2ISR);
	while(1){
		Serial.print("ENC1:");
		Serial.print(Enc1.readEncoder());
		Serial.print("ENC2:");
		Serial.println(Enc2.readEncoder());
	}
	// Connect to Wi-Fi network with SSID and password
	Serial.print("Setting AP (Access Point)…");
	// Remove the password parameter, if you want the AP (Access Point) to be open
	WiFi.softAP(ssid, password);

	IPAddress IP = WiFi.softAPIP();
	Serial.print("AP IP address: ");
	Serial.println(IP);

	server.begin();
}

void loop() {
	WiFiClient client = server.available();	 // Listen for incoming clients

	if (client) {						 // If a new client connects,
		Serial.println("New Client.");	 // print a message out in the serial port
		String currentLine = "";		 // make a String to hold incoming data from the client
		while (client.connected()) {	 // loop while the client's connected
			if (client.available()) {	 // if there's bytes to read from the client,
				char c = client.read();	 // read a byte, then
				Serial.write(c);		 // print it out the serial monitor
				currentLine += c;
				if (c == '\n') {  // if the byte is a newline character
					// if the current line is blank, you got two newline characters in a row.
					// that's the end of the client HTTP request, so send a response:
					if (currentLine.length() == 2) {
						// HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
						// and a content-type so the client knows what's coming, then a blank line:
						client.println("HTTP/1.1 200 OK");
						client.println("Content-type:text/html");
						client.println("Connection: close");
						client.println();

						// Send your "Hello World" HTML response
						client.println(
							"<html><head><meta name=\"viewport\" content=\"width=device-width, "
							"initial-scale=1\"></head>");
						client.println("<body><h1>Hello World</h1></body></html>");

						// The HTTP response ends with another blank line
						client.println();
						// Break out of the while loop
						break;
					} else {  // if you got a newline, then clear currentLine
						currentLine = "";
					}
				} else if (c != '\r') {	 // if you got anything else but a carriage return character,
					currentLine += c;	 // add it to the end of the currentLine
				}
			}
		}
		// Close the connection
		client.stop();
		Serial.println("Client disconnected.");
		Serial.println("");
	}
}