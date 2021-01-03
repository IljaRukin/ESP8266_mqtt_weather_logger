# ESP8266_mqtt_weather_logger
ESP8266 with BME280 sensor sends temperature,humidity,pressure every 5min over mqtt to raspberry pi. the data is stored there in a text file. the raspberry pi runs a ngingx web server. therefore the weather data can be accesses from the local network and it is visualized with javascript graphics library d3.

the arduino codes for the individual parts (sleep,bme280,mqtt_receive) are supplied as well as the final programm (esp8266_weather) for debugging purposes. \
folder "raspberry pi website" contains the instructuion and to setup nginx, mosquitto mqtt, and includes some example data and a website with graphical visualization of the data. \
a working copy of the library "pubsubclient-master.zip" by knolleary is supplied as well.

### issue
- ESP deep-sleep with RF disabled saves more power, but then Wifi doesn't work on wake up

<img src="/esp8266_weather.jpg" style="width:20%;">
<img src="/website.PNG" style="width:20%;">
