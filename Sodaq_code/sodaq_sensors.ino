#include <Adafruit_BME680.h>
#include <Sodaq_RN2483.h>

#define debugSerial SerialUSB

#define loraSerial Serial2

#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

#define MOISTURE_PIN 8 // pin that connects to AOUT pin of moisture sensor

#define SEALEVELPRESSURE_HPA (1013.25)

#define BUFFER_SIZE  16

Adafruit_BME680 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK);

int16_t counter = 0;

// USE YOUR OWN KEYS!
const uint8_t DevEUI[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

// USE YOUR OWN KEYS!
const uint8_t AppEUI[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

// USE YOUR OWN KEYS!
const uint8_t AppKey[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

void setup() {
  //block till serial monitor is opened
  while(!debugSerial);

  //begin usb serial
  debugSerial.begin(57600);

  loraSerial.begin(LoRaBee.getDefaultBaudRate());

  LoRaBee.setDiag(debugSerial); // optional
  LoRaBee.init(loraSerial, LORA_RESET);

  debugSerial.println();  

  setupLoRa();

  debugSerial.println(F("BME680 async test"));
 
  if (!bme.begin()) {
    debugSerial.println(F("Could not find a valid BME680 sensor, check wiring!"));
    while (1);
  }
 
  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms
}

void loop() {
  debugSerial.println("Sleeping for 3 seconds before starting sending out packets.");
  for (uint8_t i = 3; i > 0; i--)
  {
    debugSerial.println(i);
    delay(1000);
  }

  // Tell BME680 to begin measurement.
  unsigned long endTime = bme.beginReading();
  if (endTime == 0) {
    debugSerial.println(F("Failed to begin reading :("));
    return;
  }

  // Buffer sensor readings
  uint8_t buffer[BUFFER_SIZE] = {0};  // init to zero!
  
  debugSerial.print(F("Reading started at "));
  debugSerial.print(millis());
  debugSerial.print(F(" and will finish at "));
  debugSerial.println(endTime);
 
  debugSerial.println(F("You can do other work during BME680 measurement."));
  delay(50); // This represents parallel work.
  // There's no need to delay() until millis() >= endTime: bme.endReading()
  // takes care of that. It's okay for parallel work to take longer than
  // BME680's measurement time.
 
  // Obtain measurement results from BME680. Note that this operation isn't
  // instantaneous even if milli() >= endTime due to I2C/SPI latency.
  if (!bme.endReading()) {
    debugSerial.println(F("Failed to complete reading :("));
    return;
  }
  debugSerial.print(F("Reading completed at "));
  debugSerial.println(millis());
  
  float moisture = analogRead(MOISTURE_PIN);
  float temperature = bme.temperature;
  float pressure = bme.pressure / 100.0;
  float altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
  float humidity = bme.humidity;
  float gas = bme.gas_resistance / 1000.0;
  double dewPoint = dewPointFast(temperature, humidity);

  // uint8_t data[] = {temperature, pressure, altitude, humidity, gas, moisture};

  debugSerial.print("Temperature = ");
  debugSerial.print(temperature);
  debugSerial.println(" *C");
 
  debugSerial.print("Pressure = ");
  debugSerial.print(pressure);
  debugSerial.println(" hPa");
 
  debugSerial.print("Humidity = ");
  debugSerial.print(humidity);
  debugSerial.println(" %");

  debugSerial.print("Dew Point = ");
  debugSerial.print(dewPoint);
  debugSerial.println(" *C");
 
  debugSerial.print("Approx. Altitude = ");
  debugSerial.print(altitude);
  debugSerial.println(" m");
 
  debugSerial.print("Gas = ");
  debugSerial.print(gas);
  debugSerial.println(" KOhms");

  debugSerial.print("Moisture = ");
  debugSerial.print(moisture);
  debugSerial.println(" ");

  int16_t temperature_raw = (int16_t)(temperature * 100);
  int16_t pressure_raw = (int16_t)(pressure * 100);
  int16_t humidity_raw = (int16_t)(humidity * 100);
  int16_t dewPoint_raw = (int16_t)(dewPoint * 100);
  int16_t altitude_raw = (int16_t)(altitude * 100);
  int16_t gas_raw = (int16_t)(gas * 100);
  int16_t moisture_raw = (int16_t)(moisture);

  // Put the sensor values into the buffer
  int_to_byte_array(temperature_raw, &buffer[0]);
  int_to_byte_array(pressure_raw, &buffer[2]);
  int_to_byte_array(humidity_raw, &buffer[4]);
  int_to_byte_array(dewPoint_raw, &buffer[6]);
  int_to_byte_array(altitude_raw, &buffer[8]);
  int_to_byte_array(gas_raw, &buffer[10]);
  int_to_byte_array(moisture_raw, &buffer[12]);
  int_to_byte_array(++counter, &buffer[14]);

  debugSerial.print("Readings byte array: ");
  for (int i = 0; i < BUFFER_SIZE; i++) {
    debugSerial.print(buffer[i], HEX);
    debugSerial.print(" ");
  }
  debugSerial.println();

  switch (LoRaBee.send(1, (uint8_t*)&buffer, 16))
    {
    case NoError:
      debugSerial.println("Successful transmission.");
      break;
    case NoResponse:
      debugSerial.println("There was no response from the device.");
      break;
    case Timeout:
      debugSerial.println("Connection timed-out. Check your serial connection to the device! Sleeping for 20sec.");
      delay(20000);
      break;
    case PayloadSizeError:
      debugSerial.println("The size of the payload is greater than allowed. Transmission failed!");
      break;
    case InternalError:
      debugSerial.println("Oh No! This shouldn't happen. Something is really wrong! Try restarting the device!\r\nThe program will now halt.");
      while (1) {};
      break;
    case Busy:
      debugSerial.println("The device is busy. Sleeping for 10 extra seconds.");
      delay(10000);
      break;
    case NetworkFatalError:
      debugSerial.println("There is a non-recoverable error with the network connection. You should re-connect.\r\nThe program will now halt.");
      while (1) {};
      break;
    case NotConnected:
      debugSerial.println("The device is not connected to the network. Please connect to the network before attempting to send data.\r\nThe program will now halt.");
      setupLoRa();
      break;
    case NoAcknowledgment:
      debugSerial.println("There was no acknowledgment sent back!");
      break;
    default:
      break;
    }
    delay(30000);

  delay(5000);
}

double dewPointFast(double celsius, double humidity)
{
  double a = 17.271;
  double b = 237.7;
  double temp = (a * celsius) / (b + celsius) + log(humidity * 0.01);
  double Td = (b * temp) / (a - temp);
  return Td;
}

// Convert 16-bit int to 8-bit array (Big endian)
// where buf points to start byte in array
void int_to_byte_array(int16_t n, uint8_t* buf) {
  *buf = n >> 8;
  *++buf = n & 0xFF;
}

void setupLoRa(){
  //OTAA
  setupLoRaOTAA();

  LoRaBee.setSpreadingFactor(9);
}

void setupLoRaOTAA(){

  if (LoRaBee.initOTA(loraSerial, DevEUI, AppEUI, AppKey))
  {
    debugSerial.println("Network connection successful.");
  }
  else
  {
    debugSerial.println("Network connection failed!");
  }
}