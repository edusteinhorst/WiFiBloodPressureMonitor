#include <Wire.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>

const int I2C_ERROR = 10;

const char* ssid     = "YOUR WIFI SSD";
const char* password = "YOUR WIFI PWD";

// no support for hhtps!
const char* host = "YOUR WEBSITE URL";

boolean busActivity = false;
int error = 0;
int systolic = 0;
int diastolic = 0;
int hr = 0;

void setup() {

  // LED pin
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);

  delay(100); //add a small delay
  Serial.begin(115200);
  Wire.begin(0, 2); // initialise things
  delay(10); //add a small delay
  Serial.println("Wire initialized");

  // Connect to Wifi (on setup)
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // Turn the LED on (active low) so user knows WiFi is up
  digitalWrite(4, LOW);

  // We just need one byte, but 4 is the minimum.
  EEPROM.begin(4);
}

void loop() {

  int readCount = 0;
  int ourCount = 0;
  int addr = 0x00;

  // Trigger on SDA activity, user might have stored a new measurement
  busActivity = false;
  attachInterrupt(0, busActivityDetected, CHANGE);

  Serial.println("Waiting for bus activity.");

  // Lets take a nap while we wait.
  while (busActivity == false) {
    yield();
  }

  // Ok, forget the interrupts for now, there's work to do!
  detachInterrupt(0);

  // Delay a bit so the monitor's MCU can finish talking
  delay(500);

  // Get current measurement count
  readCount = getMeasurementCount();

  // The user might have turned off the unit causing an interrupt
  if (error == I2C_ERROR) {
    Serial.println("Can't communicate with the chip.");
    return;
  }

  // Check count in our own eeprom, so we know if we have sent this data before
  ourCount = EEPROM.read(0x00);

  if (readCount != 0 && readCount != ourCount) {

    Serial.println("New measurement detected!");

    // Get last measurement
    addr = (readCount - 1) * 4; // address for last measurement
    Serial.print("Reading from 0x");
    Serial.println(addr, HEX);
    refreshLastMeasurements(addr);

    // Upload to data.sparkfun.com
    uploadMeasurements();

    // Store updated count in eeprom
    EEPROM.write(0x00, readCount);
    EEPROM.commit();

    // Blink led so user knows everything is dandy
    blinkLED();

    // Print it for debugging
    Serial.print("Your last measurement is ");
    Serial.print(systolic, DEC);
    Serial.print(" mmHG X ");
    Serial.print(diastolic, DEC);
    Serial.print(" mmHG and ");
    Serial.print(hr, DEC);
    Serial.println(" bpm. Bye!");

  } else if (readCount == 0 && ourCount != 0) {

    // BPM's memory was probably erased by the user
    // So, update the count in EEPROM to keep it in sync
    EEPROM.write(0x00, 0);
    EEPROM.commit();
    Serial.println("Memory was reset - syncing EEPROM counter.");
    // Blink it
    blinkLED();

  } else {
    Serial.println("Hmm, I2C bus activity detected, but nothing to do.");
  }
}

void blinkLED() {

  for (int i = 0; i < 4; i++) {
    digitalWrite(4, HIGH);  // Turn the LED off
    delay(100);
    digitalWrite(4, LOW);   // Turn the LED on (active low)
    delay(100);
  }
}
void busActivityDetected() {

  busActivity = true;
}

int getMeasurementCount() {

  int addr = 0x00;
  byte rdata = 0x00;

  // Get current number of measurements at 0xFF
  Wire.beginTransmission(0x50);
  Wire.write(0xFF);
  Wire.endTransmission();
  Wire.requestFrom(0x50, 1);

  if (Wire.available()) {
    rdata = Wire.read();
    error = 0;
  } else {
    error = I2C_ERROR;
    // Wait a little for things to settle
    delay(300);
    return 0;
  }

  if (rdata == 0) {
    Serial.println("Nothing stored.");
    return 0;
  }

  return rdata;
}

void refreshLastMeasurements(int addr) {

  byte rdata = 0x00;

  // read measurement
  Wire.beginTransmission(0x50);
  Wire.write(addr);
  Wire.endTransmission();
  Wire.requestFrom(0x50, 4);

  if (Wire.available() != 4) {
    Serial.println("Less than 4 bytes found?");
    return;
  }

  hr = Wire.read();
  diastolic = Wire.read();
  rdata = Wire.read();
  systolic = Wire.read();
  // systolic is stored halved
  systolic *= 2;
  // if this byte is 0x80, add 1 to make it odd
  if (rdata == 0x80)
    systolic += 1;
}

void uploadMeasurements() {

  Serial.print("connecting to ");
  Serial.println(host);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  // We now create a URI for the request
  String body = "diastolic=";
  body += diastolic;
  body += "&systolic=";
  body += systolic;
  body += "&hr=";
  body += hr;

  String post = String("POST /api.php/bp HTTP/1.1\r\n") +
               "Host: " + host + "\r\n" +
               "Content-Type: text/plain\r\n" +
               "Content-Length: " + body.length() + "\r\n\r\n" + // must be twice
               body + "\r\n" +
               "Connection: close";

  Serial.println("Full request:");
  Serial.println(post);
  
  client.print(post);
  delay(10);

  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) {
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }

  Serial.println();
  Serial.println("closing connection");
}

