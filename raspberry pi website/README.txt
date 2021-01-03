

### install nginx


#install
sudo apt-get install nginx

#start/stop server:
sudo /etc/init.d/nginx start
sudo /etc/init.d/nginx stop
sudo /etc/init.d/nginx restart

#testing
sudo nginx -t

#configuration files directory
/etc/nginx/

#configuration files are:
/etc/nginx/nginx.conf
/etc/nginx/conf.d/*.conf
/etc/nginx/sites-enabled/*
[#example configuration: /etc/nginx/sites-available/default]

#website files are located at:
/var/www/html/index.html

#error log:
/var/log/nginx/access.log
/var/log/nginx/error.log
#disable log inside nginx.conf:
access_log  /dev/null;
error_log /dev/null;


#change /var/www/ permissions to incorparate user: www-data
sudo chown -R root:www-data /var/www/
#nginx use user: www-data
sudo nano /etc/nginx/sites-enabled/default
---
user www-data;
---



### install mosquitto:
sudo apt-get install mosquitto
sudo apt-get install mosquitto-clients


#configure
sudo nano /etc/mosquitto/conf.d/default.conf
---
persistence false
# mqtt
listener 1883
protocol mqtt
---

#manual message/subscription:
#subscribe
mosquitto_sub -h #hostname# -t #topic#
#test:				mosquitto_sub -h localhost -t test

#publish message
mosquitto_pub -h #hostname# -t #topic# -m "message"
#test:				mosquitto_pub -h localhost -t test -m "hello world"


#password protection:
sudo mosquitto_passwd -c /etc/mosquitto/passwd #user#
#and edit config:
sudo nano /etc/mosquitto/conf.d/default.conf
---
allow_anonymous false
password_file /etc/mosquitto/passwd
---
#now mqtt is password protected; to publish message you have to type:
mosquitto_pub -h localhost -t "test" -m "hello world" -u "user" -P "password"


#restart server
sudo systemctl restart mosquitto

#show status
service mosquitto status


------------------------------
### log weather data

#create directory and files
sudo mkdir /var/www/html/weather
sudo touch /var/www/html/weather/pressure.csv
sudo touch /var/www/html/weather/humidity.csv
sudo touch /var/www/html/weather/temperature.csv
#allow access
sudo chmod -R 777 /var/www/html/weather
sudo chmod -R 777 /var/www/html/weather/pressure.csv
sudo chmod -R 777 /var/www/html/weather/humidity.csv
sudo chmod -R 777 /var/www/html/weather/temperature.csv
#save data
sudo nano /etc/init.d/log_mqtt.sh
---
#! /bin/sh

### BEGIN INIT INFO
# Provides:          none
# Required-Start:    $mosquitto $mosquitto_sub
# Required-Stop:     $mosquitto $mosquitto_sub
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
### END INIT INFO

# If you want a command to always run, put it here

# Carry out specific functions when asked to by the system
case "$1" in
  start)
    sleep 1m;
    echo "start logging weather data"
    mosquitto_sub -p 1883 -u esp8266bme280 -P 4liveweather -t /weather/pressure | xargs -d '\n' -L 1 bash -c 'date "+%d-%m-%Y_%T, $0"' >> /var/www/html/weather/pressure.csv &
    mosquitto_sub -p 1883 -u esp8266bme280 -P 4liveweather -t /weather/humidity | xargs -d '\n' -L 1 bash -c 'date "+%d-%m-%Y_%T, $0"' >> /var/www/html/weather/humidity.csv &
    mosquitto_sub -p 1883 -u esp8266bme280 -P 4liveweather -t /weather/temperature | xargs -d '\n' -L 1 bash -c 'date "+%d-%m-%Y_%T, $0"' >> /var/www/html/weather/temperature.csv &
    ;;
  stop)
    echo "stop logging weather data"
    ;;
  *)
    echo "Usage: /etc/init.d/log_mqtt.sh {start|stop}"
    exit 1
    ;;
esac

exit 0
---

#make executable
sudo chmod +x /etc/init.d/log_mqtt.sh
#add script to run on boot
sudo update-rc.d log_mqtt.sh defaults
###start script (for first time only):
sudo /etc/init.d/log_mqtt.sh start
