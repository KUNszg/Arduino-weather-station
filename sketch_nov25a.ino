#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <PMserial.h>
#include <ESP8266WiFi.h>
#include <OLED_SSD1306_Chart.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

SerialPM pms(PMS1003, D6, D5);

uint8_t DHTPin = D4;
float Temperature;
float Humidity;

#define DHTTYPE DHT11

DHT dht(DHTPin, DHTTYPE);

#define SEALEVELPRESSURE_HPA (1030.00)

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

OLED_SSD1306_Chart display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

char actualThickness;

// WiFi parameters to be configured
char* ssid = "gigachad"; // Write here your router's username
char* password = "aewk1234"; // Write here your router's passward

WiFiServer server(80);

void setup() {
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("Delaying start: WiFi not connected");
  }

  Serial.begin(9600);

  pinMode(DHTPin, INPUT);

  pms.init();
  dht.begin();
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  server.begin();

  // Print the IP address
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");

  display.clearDisplay();
  display.setChartCoordinates(0, 60);      //Chart lower left coordinates (X, Y)
  display.setChartWidthAndHeight(128, 35); //Chart width = 123 and height = 60
  display.setXIncrement(2);                //Distance between Y points will be 5px
  display.setYLimits(0, 100);             //Ymin = 0 and Ymax = 100
  display.setYLimitLabels("0", "100");    //Setting Y axis labels
  display.setYLabelsVisible(true);
  display.setAxisDivisionsInc(12, 6);    //Each 12 px a division will be painted in X axis and each 6px in Y axis
  display.setPlotMode(SINGLE_PLOT_MODE); //Set single plot mode
  // display.setPointGeometry(POINT_GEOMETRY_CIRCLE);
  actualThickness = LIGHT_LINE;
  display.setLineThickness(actualThickness);
  display.drawChart(); //Update the buffer to draw the cartesian chart
  display.display();
}

int monitorNum = 0;

int localhostServer(SerialPM pms, int Humidity, int Temperature) {
  WiFiClient client = server.available();
  String request = client.readStringUntil('\r');
  client.flush();

  if (request.indexOf("/monitor=0") != -1)  {
    monitorNum = 0;
  }

  if (request.indexOf("/monitor=1") != -1)  {
    monitorNum = 1;
  }

  if (request.indexOf("/monitor=2") != -1)  {
    monitorNum = 2;
  }

  if (request.indexOf("/monitor=3") != -1)  {
    monitorNum = 3;
  }

  if (request.indexOf("/monitor=4") != -1)  {
    monitorNum = 4;
  }

  if (request.indexOf("/monitor=5") != -1)  {
    monitorNum = 5;
  }

  Serial.println(monitorNum);

  return 1;
}

unsigned long lastTime = 0;
unsigned long timerDelay = 1000;
string serverName = "https://kunszg.com/api/university-project";

void loop() {
  // Send an HTTP POST request depending on timerDelay
  if ((millis() - lastTime) > timerDelay) {
    //  display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.setTextColor(WHITE, BLACK);

    Temperature = dht.readTemperature(); // Gets the values of the temperature
    Humidity = dht.readHumidity(); // Gets the values of the humidity

    //  display.print("Temperature = ");
    //  display.print(static_cast<int>(Temperature), DEC);
    //  display.print(" *C");
    //
    //  display.println("");
    //
    display.print("Wilgotnosc ");
    display.print(static_cast<int>(Humidity), DEC);
    display.print(" %RH");

    display.println("");
    //  display.println("");

    pms.read();

    localhostServer(pms, static_cast<int>(Humidity), static_cast<int>(Temperature));

    // put your main code here, to run repeatedly:
    auto value = static_cast<int>(Humidity);
    if (!display.updateChart(value)) {
      display.clearDisplay(); //If chart is full, it is drawn again
      if (actualThickness == NORMAL_LINE)
      {
        actualThickness = LIGHT_LINE;
      }
      else if (actualThickness == LIGHT_LINE)
      {
        actualThickness = NORMAL_LINE;
      }
      display.setLineThickness(actualThickness); \
      display.drawChart();
    }


    //Check WiFi connection status
    if (WiFi.status() == WL_CONNECTED) {
      WiFiClient client;
      HTTPClient http;

      String temperatureArg = "?temperature=" + static_cast<String>(Temperature);
      String humidityArg = "&humidity=" + static_cast<String>(Humidity);
      String pm10Arg = "&pm10=" + static_cast<String>(pms.pm10);
      String pm01Arg = "&pm01=" + static_cast<String>(pms.pm01);
      String pm25Arg = "&pm25=" + static_cast<String>(pms.pm25);
      String auth = "&auth=";

      String serverPath = serverName + temperatureArg + humidityArg + pm10Arg + pm01Arg + pm25Arg + auth;

      // Your Domain name with URL path or IP address with path
      http.begin(client, serverPath.c_str());

      // Send HTTP PUT request
      int httpResponseCode = http.PUT();

      if (httpResponseCode > 0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        String payload = http.getString();
        Serial.println(payload);
      }
      else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      http.end();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }

  //  display.print(F("PM10: "));
  //  display.print(pms.pm10);
  //  display.print(" ");
  //  display.print(F("g/m"));
  //  display.println("");
  //
  //  display.print(F("PM1.0: "));
  //  display.print(pms.pm01);
  //  display.print(" ");
  //  display.print(F("g/m"));
  //  display.println("");
  //
  //  display.print(F("PM2.5: "));
  //  display.print(pms.pm25);
  //  display.print(" ");
  //  display.print(F("g/m"));
  //  display.println("");
  //
  //  display.display();
}