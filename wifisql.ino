/*
    This Application for an ESP8266 WiFi MicroController uses a DHT22 Sensor to get temperatur and humidity.
    After this it stores this information inside a mysql database via WiFi connection.

    This code is a kind of merge of different sketches and tutorials.
    Testrun was more than 48h successful. 
*/

#include <ESP8266WiFi.h>
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>
#include "DHT.h"

//--------------NEEDED WLAN INFORMATION-------------------------//
#define DHTPIN      D2                  // Digital pin connected to the DHT sensor 
#define DHTTYPE     DHT22               // DHT 22 (AM2302)
#define STASSID     "SSID"              // SSID of your WLAN
#define STAPSK      "LONGPASSWORD"      // Password of your WLAN
//--------------NEEDED DB INFORMATION--------------------------//
#define SERVER      192,168,1,2         // IP-Address of your Server - coma instead of points
#define SQLUSER     "SQLUSER"           // User of your database-server
#define SQLPASS     "dkjfhgkjdf"        // Password of your database-server
#define DBNAME      "USE roomtemp;"     // databasename
#define ROOMNR      "3"

//------------------------------------------------------------//
#define VERSION     "1.1 - 2021.04.02"

void(* resetFunc) (void) = 0;
const char* ssid     = STASSID;
const char* wlanpassword = STAPSK;
DHT dht(DHTPIN, DHTTYPE);

IPAddress server_addr(SERVER);       // IP of the MySQL *server* here
char user[] = SQLUSER;                   // MySQL user login username
char password[] = SQLPASS;               // MySQL user login password
const char USE_DB_SQL[] = DBNAME;
uint32_t delayMS;
int lostconnection = 0;
char resultt[8];
char resulth[8];

float h;
float t;
float hold;
float told;

// Use WiFiClient class to create TCP connections
WiFiClient client;
MySQL_Connection conn(&client);
MySQL_Cursor* cursor;

void connectsql(void)
{
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.print("Connecting to SQL...  ");
  if (conn.connect(server_addr, 3306, user, password))
    Serial.println("OK.");
  else
  {
    Serial.println("FAILED.");
    return;
  }
  // create MySQL cursor object
  cursor = new MySQL_Cursor(&conn);
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
  dht.begin();
  // We start by connecting to a WiFi network
  delay(500); // for serialmonitor
  Serial.println();
  Serial.println();
  Serial.println("Simple ESP-WIFI-TEMP to MYSQL Server Chip");
  Serial.println("Version: ");
  Serial.println(VERSION);
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, wlanpassword);
  int count = 0;
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
    Serial.print(".");
    count++;
    digitalWrite(LED_BUILTIN, HIGH);
      if(count == 20)
      {
        h = dht.readHumidity();
        t = dht.readTemperature();
        Serial.println("");
        Serial.println("No WiFi connection established! Try again...");
        digitalWrite(LED_BUILTIN, LOW);
        Serial.print(F("Temperature: "));
        Serial.print(t);
        Serial.print(F("°C - Humidity: "));
        Serial.print(h);
        Serial.println();
        count =0;
      }
    }
 connectsql();
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
    if(lostconnection >= 30)
    {
       Serial.println("No Wifi connection about more than a Minute, Try restart...");
       resetFunc();
    }
    Serial.println("WiFi connection lost, Try reconnnecting...");
    WiFi.begin(ssid, wlanpassword);
    lostconnection++;
    digitalWrite(LED_BUILTIN, HIGH);
    return;
   }
  if((WiFi.status() == WL_CONNECTED) && (lostconnection >= 1))
  {
    lostconnection = 0;
    connectsql();
  }

  if((h == hold) && (told == t))
  {
    //Nothing changed
    Serial.println("No new Values...");
    digitalWrite(LED_BUILTIN, LOW);
    return;
  }
  if(((h != hold) || (told != t))  && (WiFi.status() == WL_CONNECTED))
  {
    char sqlcmdc[99] ="INSERT INTO tblmeasures (temp, humidity, roomnr) VALUES (";
    dtostrf(t,2,2,resultt);
    dtostrf(h,2,2,resulth);
    strcat(sqlcmdc, resultt);
    strcat(sqlcmdc, ",");
    strcat(sqlcmdc, resulth);
    strcat(sqlcmdc, ",");
    strcat(sqlcmdc, ROOMNR);
    strcat(sqlcmdc, ")\0");
    Serial.println(sqlcmdc);
  if (conn.connected())
  {
    cursor->execute(USE_DB_SQL);
    cursor->execute(sqlcmdc);
    digitalWrite(LED_BUILTIN, LOW);
  }
  else
  {
    Serial.println("No Connection to SQL Server!\n");
  }
  hold = h;
  told = t;

  // delay(300000); // execute once every 5 minutes, don't flood remote service
  }
}
