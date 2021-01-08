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
String sqlcmd;
int str_len;  
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


 // char INSERT_SQL[] = "INSERT INTO wohnzimmer (temp, roomnr) VALUES (99,1)";
  sqlcmd = "INSERT INTO wohnzimmer (temp, humidity, roomnr) VALUES (";
  sqlcmd += String(t,2);
  sqlcmd += ",";
  sqlcmd += String(h,2);
  sqlcmd += ",1)";
  Serial.print(sqlcmd);
  Serial.print(F("Temperature: "));
  Serial.print(t);
  Serial.print(F("°C - Humidity: "));
  Serial.print(h);

  str_len = sqlcmd.length() + 1; 
// Prepare the character array (the buffer) 
char sqlbefehl[str_len];
 
// Copy it over 
sqlcmd.toCharArray(sqlbefehl, str_len);

  if (conn.connected())
    cursor->execute(USE_DB_SQL);
    cursor->execute(sqlbefehl);


 // delay(300000); // execute once every 5 minutes, don't flood remote service
}
