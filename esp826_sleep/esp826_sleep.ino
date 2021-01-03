/*
 * connect D0 to Reset for waking up
 * => on board LED (at D0) not usable
*/

void setup() {
  Serial.begin(9600);
  Serial.setTimeout(2000);

  while(!Serial) {
    delay(5);
  }
  //sleep mode for 10 s
  Serial.println("");
  Serial.println("woke up to go to sleep for 10s again");
  ESP.deepSleep( 10e6, WAKE_RF_DEFAULT);
}

void loop() {
}
