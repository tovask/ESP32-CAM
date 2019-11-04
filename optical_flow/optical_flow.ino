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

  TFmini Plus (std9040):
  http://en.benewake.com/product/detail/5c345cd0e5b3a844c472329b.html
  https://github.com/senegalo/TFMiniPlus


*/

#include "src/esp32_cam/camera.h"
#include "flow.hpp"
#include "lidar.hpp"

#define SENSOR_ID 0 // TODO: 

int64_t time_prev;
int64_t sec_prev;

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  camera_setup(); // setting up and configuring the camera

  // setting up and configuring the flow computer, initializing it's image buffer
  camera_fb_t * fb = esp_camera_fb_get();
  // TODO: check/use fb->width (fb->height, fb->len)
  flow_setup(fb->buf);
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

  int dt_us;
  float flow_x, flow_y;
  int flow_quality = getFlow(fb->buf, dt_us, flow_x, flow_y);

  uint16_t ground_distance = get_distance(); // from lidar

  // TODO: check if dt_us == (time_now - time_prev)

  //float flow_comp_m_x = - pixel_flow_x / focal_length / ( (time_now - time_prev) / 1000000.0f) * ground_distance;

  float speed_x = tan(flow_x) * (float)ground_distance;
  float speed_y = tan(flow_y) * (float)ground_distance;

  Serial.printf("%" PRIu32 " dt: %d us,\tx: %f,\ty: %f,\tquality: %d,\tlidar: %" PRIu16 " \n", (uint32_t)time_now, dt_us, flow_x, flow_y, flow_quality, ground_distance);


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

  time_prev = time_now;
}
