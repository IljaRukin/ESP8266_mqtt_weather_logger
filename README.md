# ESP8266_mqtt_weather_logger
ESP8266 with BME280 sensor sends temperature, humidity, pressure every 5min over mqtt to a raspberry pi. the data is stored there in a text file. the raspberry pi runs a ngingx web server. therefore the weather data can be accesses from the local network and is visualized with javascript graphics library d3 on a web page.

the arduino codes for the individual parts (sleep,bme280,mqtt_receive) are supplied as well as the final program (esp8266_weather) for debugging purposes. \
folder "raspberry pi website" contains the instructions to setup nginx, mosquitto mqtt, and includes some example data and a web page with graphical visualization of the data. \
a working copy of the library "pubsubclient-master.zip" by knolleary is supplied as well. It is crucial for sending mqtt messages.

### issue
- ESP deep-sleep with RF disabled saves more power, but then Wifi doesn't work on wake up

here are some images of the setup and the web page:
<img src="/esp8266_weather.jpg" style="width:20%;">
<img src="/website.PNG" style="width:20%;">

data was taken with unmodified nodemcu 1.0 with 4AAA NiMh Batteries. \
If the ESP8266 is separated from board and powerd by a more efficient LDO and used with a LiPo, the batery would last way longer. \
note: the 3 outliners at the end happened when the battery was low and the BME280 didn't receive enough energy.

TODO: warning on low battery
