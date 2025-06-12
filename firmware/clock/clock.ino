#include <WiFi.h>
#include <time.h>
#include <RTClib.h>


const int HV_ENABLE_PIN = 32; //change to your EN pin

//replace with your actual credentials
const char* wifi_ssid = "YOUR_WIFI_SSID";
const char* wifi_password = "YOUR_WIFI_PASSWORD";


const char* ntp_server = "pool.ntp.org"; //connects to ntp server
const long gmt_offset_sec = -5 * 3600;  //adjust for your timezone
const int daylight_offset_sec = 0;

class NixieTube {
private:
int c1_pin, c2_pin, c3_pin, c4_pin;

public:
  NixieTube(int pin1, int pin2, int pin3, int pin4) { //constructor
    c1_pin = pin1;
    c2_pin = pin2;
    c3_pin = pin3;
    c4_pin = pin4;

    pinMode(c1_pin, OUTPUT);
    pinMode(c2_pin, OUTPUT);
    pinMode(c3_pin, OUTPUT);
    pinMode(c4_pin, OUTPUT);

    //turn all off
    digitalWrite(c1_pin, HIGH);
    digitalWrite(c2_pin, HIGH);
    digitalWrite(c3_pin, HIGH);
    digitalWrite(c4_pin, HIGH);
  }

  void displayDigit(int digit) {
    String binary = "";
    for (int i = 3; i >= 0; i--) {
      binary += String((digit >> i) & 1); //converts digit to binary
    }

    Serial.println(binary);

    //today I learned digitalWrite takes booleans (this is awesome)
    digitalWrite(c4_pin, binary[3] == '1');  //LSB (rightmost bit)
    digitalWrite(c3_pin, binary[2] == '1');
    digitalWrite(c2_pin, binary[1] == '1');
    digitalWrite(c1_pin, binary[0] == '1');  //MSB (leftmost bit)
  }

  void clear() {
    //turn off all
    digitalWrite(c1_pin, HIGH);
    digitalWrite(c2_pin, HIGH);
    digitalWrite(c3_pin, HIGH);
    digitalWrite(c4_pin, HIGH);
  }
};

NixieTube nixie1(1, 2, 4, 5);
NixieTube nixie2(6, 7, 9, 10);
NixieTube nixie3(11, 12, 14, 15);
NixieTube nixie4(16, 17, 19, 20);
NixieTube nixie5(21, 22, 24, 25);
NixieTube nixie6(26, 27, 29, 31); //change according to your schematic

void setup() {
  Serial.begin(115200);

  pinMode(HV_ENABLE_PIN, OUTPUT);
  pinMode(WL_GPIO0, OUTPUT);

  Serial.print("Connecting to WiFi");
  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  configTime(gmt_offset_sec, daylight_offset_sec, ntp_server);

  Serial.print("Waiting for NTP time sync");
  while (!time(nullptr)) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println();
  Serial.println("Time synced");


  digitalWrite(HV_ENABLE_PIN, HIGH);

  Serial.println("cycling through all digits");
  for (int digit = 0; digit < 10; digit++) {
    digitalWrite(WL_GPIO0, !digitalRead(WL_GPIO0));

    nixie1.displayDigit(digit);
    nixie2.displayDigit(digit);
    nixie3.displayDigit(digit);
    nixie4.displayDigit(digit);
    nixie5.displayDigit(digit);
    nixie6.displayDigit(digit);

    delay(300);
  }

  //turn all off
  nixie1.clear();
  nixie2.clear();
  nixie3.clear();
  nixie4.clear();
  nixie5.clear();
  nixie6.clear();

  Serial.println("testing complete. starting clock"); //if you didn't see anything you're in trouble
}

void loop() {
  //get current time
  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);

  //print time to serial monitor
  Serial.printf("%02d:%02d:%02d\n",
                timeinfo->tm_hour,
                timeinfo->tm_min,
                timeinfo->tm_sec);


  int hour = timeinfo->tm_hour;
  int minute = timeinfo->tm_min;
  int second = timeinfo->tm_sec;


  if (second % 2 == 0) {  //I am so cool
    digitalWrite(WL_GPIO0, HIGH);
  } else {
    digitalWrite(WL_GPIO0, LOW);
  }


  nixie1.displayDigit(hour / 10);    //Hour tens
  nixie2.displayDigit(hour % 10);    //Hour ones
  nixie3.displayDigit(minute / 10);  //Minute tens
  nixie4.displayDigit(minute % 10);  //Minute ones
  nixie5.displayDigit(second / 10);  //Second tens
  nixie6.displayDigit(second % 10);  //Second ones

  delay(10);
}