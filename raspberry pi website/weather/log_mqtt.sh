#! /bin/sh
mosquitto_sub -p 1883 -u esp8266bme280 -P 4liveweather -t /weather/pressure | xargs -d '\n' -L 1 bash -c 'date "+%d-%m-%Y_%T, $0"' > /var/www/html/weather/pressure.csv &
mosquitto_sub -p 1883 -u esp8266bme280 -P 4liveweather -t /weather/humidity | xargs -d '\n' -L 1 bash -c 'date "+%d-%m-%Y_%T, $0"' > /var/www/html/weather/humidity.csv &
mosquitto_sub -p 1883 -u esp8266bme280 -P 4liveweather -t /weather/temperature | xargs -d '\n' -L 1 bash -c 'date "+%d-%m-%Y_%T, $0"' > /var/www/html/weather/temperature.csv &
