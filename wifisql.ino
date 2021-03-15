/*
    This Application for an ESP8266 WiFi MicroController uses a DHT22 Sensor to get temperatur and humidity.
    After this it stores this information inside a mysql database via WiFi connection.

    This code is a kind of merge of different sketches.
*/

#include <ESP8266WiFi.h>
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>
#include "DHT.h"

#define DHTPIN      D2              // Digital pin connected to the DHT sensor 
#define DHTTYPE     DHT22           // DHT 22 
#define STASSID     "SSID"          // SSID of your WLAN
#define STAPSK      "PASSWORD"      // Password of your WLAN
#define SQLUSER     "ROOT"
#define SQLPASS     "SQLPASS"
#define BAUDRATE    9600
#define ROOMNR      2

const char* ssid         = STASSID;
const char* wlanpassword = STAPSK;
DHT dht(DHTPIN, DHTTYPE);

IPAddress server_addr(0,0,0,0);     // IP of the MySQL *server* here ; uses comata as points!
char user[]         = SQLUSER;      // MySQL user login username
char password[]     = SQLPASS;      // MySQL user login password
char USE_DB_SQL[]   = "USE DATABASE;";
uint32_t delayMS;
float h;
float t;
float hold;
float told;

// Use WiFiClient class to create TCP connections
WiFiClient client;
MySQL_Connection conn(&client);
MySQL_Cursor* cursor;

void setup() {
  Serial.begin(9600);
  dht.begin();
  // We start by connecting to a WiFi network
  delay(500); // for serialmonitor
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, wlanpassword);
  int count = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    count++;
      if(count == 20)
      {
        h = dht.readHumidity();
        t = dht.readTemperature();
        Serial.println("");
        Serial.println("No WiFi connection established! Try again...");
        Serial.print(F("Temperature: "));
        Serial.print(t);
        Serial.print(F("°C - Humidity: "));
        Serial.print(h);
        Serial.println();
        count =0;
      }
    }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.print("Connecting to SQL...  ");
  if (conn.connect(server_addr, 3306, user, password))
    Serial.println("OK.");
  else
    Serial.println("FAILED.");
  
  // create MySQL cursor object
  cursor = new MySQL_Cursor(&conn);
}

void loop() {


  delay(5000);
  h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  t = dht.readTemperature();

  Serial.println();
  Serial.print(F("Temperature: "));
  Serial.print(t);
  Serial.print(F("°C - Humidity: "));
  Serial.print(h);
  Serial.println(); 

  //Check WIFI
if(WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WiFi connection lost, Try reconnnecting...");
    WiFi.begin(ssid, wlanpassword);
    return;
   }

if((h == hold) && (told == t))
{
  //Nothing changed
  Serial.println("No new Values...");
  return;
}
  char sqlcmdc[99] ="INSERT INTO wohnzimmer (temp, humidity, roomnr) VALUES (";
  char resultt[8];
  char resulth[8];
  dtostrf(t,2,2,resultt);
  dtostrf(h,2,2,resulth);

  strcat(sqlcmdc, resultt);

  strcat(sqlcmdc, ",");
  strcat(sqlcmdc, resulth);
  strcat(sqlcmdc, ",ROOMNR)\0");
  Serial.println(sqlcmdc);

  if (conn.connected())
    cursor->execute(USE_DB_SQL);
    cursor->execute(sqlcmdc);
  hold = h;
  told = t;

 // delay(300000); // execute once every 5 minutes, don't flood remote service
}
