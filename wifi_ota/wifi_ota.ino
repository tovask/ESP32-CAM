


void setup() {
  Serial.begin(115200);
  Serial.println();

  wifi_setup();

  ota_setup();

}


void loop() {

  // ota needs to continously check for incoming update requests
  ota_handle();

}
