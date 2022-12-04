#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP3XX.h>
#include <DHT.h>

#define BMP_SCK D5
#define BMP_MISO D6
#define BMP_MOSI D7
#define BMP_CS D8

#define SEALEVELPRESSURE_HPA (1019)

Adafruit_BMP3XX bmp;

#define DHTTYPE DHT11
uint8_t DHTPin = D4; 

DHT dht(DHTPin, DHTTYPE);

float Temperature;
float Humidity;

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Serial.begin(9600);

  pinMode(DHTPin, INPUT);

  dht.begin(); 

  //Serial.begin(115200);
  while (!Serial);
  Serial.println("Adafruit BMP388 / BMP390 test");
  if (! bmp.begin_SPI(BMP_CS)) {  // hardware SPI mode  
    if (! bmp.begin_SPI(BMP_CS, BMP_SCK, BMP_MISO, BMP_MOSI)) {  // software SPI mode
      Serial.println("Could not find a valid BMP3 sensor, check wiring!");
      while (1);
    }
  }

  // Set up oversampling and filter initialization
  bmp.setTemperatureOversampling(BMP3_OVERSAMPLING_8X);
  bmp.setPressureOversampling(BMP3_OVERSAMPLING_4X);
  bmp.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_3);
  bmp.setOutputDataRate(BMP3_ODR_50_HZ);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000); // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();
}

void loop() {
  delay(2000);
  
  display.clearDisplay();

  Temperature = dht.readTemperature(); // Gets the values of the temperature
  Humidity = dht.readHumidity(); // Gets the values of the humidity 

  display.setTextSize(1.1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  
  display.print("Temperature = ");
  display.println(static_cast<int>(Temperature),DEC);
  display.print("Humidity = ");
  display.println(static_cast<int>(Humidity),DEC);

  if (! bmp.performReading()) {
    Serial.println("Failed to perform reading :(");
  }
  else {
    display.print("Temperature = ");
    display.print(bmp.temperature);
    display.println("*C");
    
    display.print("Pressure = ");
    display.print(bmp.pressure / 100.0);
    display.println(" hPa");
  
    display.print("Approx. Altitude = ");
    display.print(bmp.readAltitude(SEALEVELPRESSURE_HPA));
    display.println(" m");
  
    Serial.println();
  }

  display.display(); 
}
