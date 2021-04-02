# esp-wifi-temp

ESP8266WiFi - temperatur and humidity insert into MariaDB / MYSQL even for mulitple rooms

Using a µController and just a DHT Sensor for getting temperature and humidity into a MYSQL Database.
Can be used for Grafana or smarthome.

Here the SQL command:
``
CREATE TABLE IF NOT EXISTS `tblmeasure` (
  `ID` int(11) NOT NULL AUTO_INCREMENT,
  `temp` float NOT NULL DEFAULT '0',
  `datetime` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `roomnr` int(11) DEFAULT NULL,
  `humidity` float DEFAULT '0',
  PRIMARY KEY (`ID`)
)
``
