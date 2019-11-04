/*

  esp32:
  https://github.com/espressif/esp32-camera

  camera:
  https://github.com/espressif/arduino-esp32/tree/master/libraries/ESP32/examples/Camera/CameraWebServer

  optical flow:
  https://github.com/PX4/Flow/blob/master/src/modules/flow/flow.c
  https://github.com/PX4/OpticalFlow/blob/master/src/px4flow.cpp

  mavlink:
  https://mavlink.io/en/mavgen_c/
  https://github.com/mavlink/c_library_v2
  https://github.com/mavlink/c_library_v2/blob/master/common/mavlink_msg_optical_flow.h
  https://github.com/PX4/Flow/blob/master/src/modules/flow/main.c#L572

  i2c:
  https://github.com/PX4/Flow/blob/master/src/include/i2c_frame.h
  https://github.com/eschnou/arduino-px4flow-i2c

  TFmini Plus:


*/

#include "src/esp32_cam/camera.h"
#include "flow.hpp"

#define SENSOR_ID 0 // TODO: 

int64_t time_prev;
int64_t sec_prev;

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  camera_setup(); // setting up and configuring the camera

  flow_setup(); // setting up and configuring the flow computer // TODO: add parameter img_width, img_hight
  camera_fb_t * fb = esp_camera_fb_get();
  int dt_us;
  float flow_x, flow_y;
  int flow_quality = getFlow(fb->buf, dt_us, flow_x, flow_y); // TODO: do it in flow_setup
  esp_camera_fb_return(fb); // return the buffer to the pool

  time_prev = esp_timer_get_time();
}


void loop() {
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return; // ESP_FAIL;
  }

  int64_t time_now = esp_timer_get_time(); // current time in micro seconds
  int64_t sec_now = time_now / 1000000; // current time in seconds


  int dt_us;
  float flow_x, flow_y;
  int flow_quality = getFlow(fb->buf, dt_us, flow_x, flow_y); // TODO: do it in flow_setup

  Serial.printf("%" PRIu32 " dt: %d us,\tx: %f,\ty: %f,\tquality: %d\n", (uint32_t)time_now, dt_us, flow_x, flow_y, flow_quality);


  float ground_distance = 1; // from lidar
  //const float focal_length_px = (global_data.param[PARAM_FOCAL_LENGTH_MM]) / (4.0f * 6.0f) * 1000.0f; //original focal lenght: 12mm pixelsize: 6um, binning 4 enabled
  float focal_length = 250;

  float pixel_flow_x = flow_x;
  float flow_comp_m_x = - pixel_flow_x / focal_length / ( (time_now - time_prev) / 1000000.0f) * ground_distance;

  /*  // send flow
    mavlink_msg_optical_flow_send(
      MAVLINK_COMM_0, // mavlink_channel_t chan
      get_boot_time_us(), // uint64_t time_usec
      SENSOR_ID, // uint8_t sensor_id
      flow_x, // int16_t flow_x
      flow_y, // int16_t flow_y
      flow_comp_m_x, // float flow_comp_m_x
      flow_comp_m_y, // float flow_comp_m_y
      flow_quality, // uint8_t quality
      ground_distance, // float ground_distance
      0, // float flow_rate_x
      0 // float flow_rate_y
    );
    / *
      	mavlink_msg_optical_flow_rad_send(MAVLINK_COMM_0, get_boot_time_us(), SENSOR_ID,
      			integration_timespan, accumulated_flow_x, accumulated_flow_y,
      			accumulated_gyro_x, accumulated_gyro_y, accumulated_gyro_z,
      			gyro_temp, accumulated_quality/accumulated_framecount,
        time_since_last_sonar_update,ground_distance);
  */

  esp_camera_fb_return(fb); // return the buffer to the pool

  if (sec_now != sec_prev) {
    sec_prev = sec_now;
  }
  time_prev = time_now;
}
