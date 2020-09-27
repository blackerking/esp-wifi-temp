/*
    This sketch establishes a TCP connection to a "quote of the day" service.
    It sends a "hello" message, and then prints received data.
*/

#include <ESP8266WiFi.h>
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>
#include "DHT.h"

#define DHTPIN D2            // Digital pin connected to the DHT sensor 
#define DHTTYPE    DHT22     // DHT 22 
#define STASSID "SSID"
#define STAPSK  "PASSWORD"
const char* ssid     = STASSID;
const char* wlanpassword = STAPSK;
DHT dht(DHTPIN, DHTTYPE);

IPAddress server_addr(0,0,0,0);  // IP of the MySQL *server* here
char user[] = "ROOT";              // MySQL user login username
char password[] = "SQLPASSWORD";        // MySQL user login password
char USE_DB_SQL[] = "USE DATABASE;";
uint32_t delayMS;
  
// Use WiFiClient class to create TCP connections
WiFiClient client;
MySQL_Connection conn(&client);
MySQL_Cursor* cursor;

void setup() {
  Serial.begin(9600);
  dht.begin();
  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  /* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
     would try to act as both a client and an access-point and could cause
     network-issues with your other WiFi-devices on your WiFi-network. */
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, wlanpassword);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
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
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();


 // char INSERT_SQL[] = "INSERT INTO temperatur (temp, roomnr) VALUES (99,1)";
  String sqlcmd = "INSERT INTO temperatur (temp, humidity, roomnr) VALUES (";
  sqlcmd += String(t,2);
  sqlcmd += ",";
  sqlcmd += String(h,2);
  sqlcmd += ",1)";
  Serial.print(sqlcmd);
  Serial.print(F("Temperature: "));
  Serial.print(t);
  Serial.print(F("°C - Feuchtigkeit: "));
  Serial.print(h);

int str_len = sqlcmd.length() + 1; 
 
// Prepare the character array (the buffer) 
char sqlbefehl[str_len];
 
// Copy it over 
sqlcmd.toCharArray(sqlbefehl, str_len);

  if (conn.connected())
    cursor->execute(USE_DB_SQL);
    cursor->execute(sqlbefehl);


 // delay(300000); // execute once every 5 minutes, don't flood remote service
}
