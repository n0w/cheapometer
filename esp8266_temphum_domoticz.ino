// ESP8266 Temperature + Humidity cheap'o'meter
// Reads sensor data from DHT22 and sends it to domoticz via http get request
// 07/2016 Angel Suarez-B. Martin (n0w)

#include <ESP8266WiFi.h>
#include <DHT.h>
#include <stdlib.h> 

#define DHTTYPE DHT22           // Sensor type
#define DHTPIN  2               // Sensor pin [GPIO2]
#define WAITTIME 10000          // Time in milliseconds to wait for on main loop
#define SERIALSPEED 115200      // Serial port speed (for debugging purposes)
#define UPPERTHR 80             // Upper threshold for DHT22 operating range
#define LOWERTHR -20            // Lower threshold for DHT22 operating range

// --------- Per sensor data --------------------------

// Target wifi network
const char* ssid     = "";
const char* password = "";

// Domoticz IP & port
const char* host = "";
const int port = ;

// Device index to update at domoticz
int idx = 17;


// --------- Globals       ----------------------------

DHT dht(DHTPIN, DHTTYPE, 11);
float hum, temp = 0;

unsigned long previousMillis = 0; 
const long DHTinterval = 2000;


// --------- Functions  ------------------------------

int getSensorData() 
// Reads temperature and humidity from DHT22 sensor
// Returns 1 on success, -1 on failure
{
  unsigned long currentMillis = millis();

 // DHT sensors can't be read within 2 seconds from the last read
  if(currentMillis - previousMillis >= DHTinterval) 
  { 
    previousMillis = currentMillis;   
 
    hum = dht.readHumidity();  
    temp = dht.readTemperature();

    // Sanity check
    if (isnan(hum) || isnan(temp) || temp > UPPERTHR || temp < LOWERTHR || hum < 0 || hum > 100) 
    {
      Serial.println("[e] Can't read DHT sensor!");
      return -1;
    }
    else return 1;
  }
}

void setup() 
{
  Serial.begin(SERIALSPEED);
  delay(10);

  Serial.println("n0w ESP8266 temp+hum cheap'o'meter");
  Serial.println("==================================");
  Serial.print("[-] Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("I'm in!");
  Serial.print("[-] IP address: ");
  Serial.println(WiFi.localIP());
}


void loop() 
{
  Serial.print("[+] Connecting to domoticz on ");
  Serial.print(host);
  Serial.print(":");
  Serial.println(port);

  // Get sensors values
  while (getSensorData() == -1)
  {
    Serial.println ("[mainLoop] Skipping. Retry in 2 seconds");
    delay(2000);
  }

  // Build the url to domoticz api to update the device with index "idx" and values temp_f and humidity
  /* From domoticz api guide:
      IDX = id of your device (This number can be found in the devices tab in the column "IDX")
      TEMP = Temperature
      HUM = Humidity
      HUM_STAT = Humidity status
        0=Normal
        1=Comfortable
        2=Dry
        3=Wet
  */
  
  String url = "/json.htm?type=command&param=udevice&idx=" + String(idx) + "&nvalue=0&svalue=" + String(temp) + ";" + String(hum) + ";1";

  // Perform HTTP GET request
  WiFiClient client;
  if (!client.connect(host, port)) 
  {
    Serial.println("[e] Connection failed!");
    return;
  }

  Serial.print("[+] Requesting URL: ");
  Serial.println(url);
  
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
              "Host: " + host + "\r\n" + 
              "Connection: close\r\n\r\n");
  delay(10);

  Serial.println("Respond:");
  while(client.available())
  {
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }

  delay(WAITTIME);
}

