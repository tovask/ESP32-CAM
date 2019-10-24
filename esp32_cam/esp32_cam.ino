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

  TFmini Plus:


*/

#include <esp_camera.h>

#include "src/esp32_cam/camera.h"
#include "src/flow_calc/flow_px4.hpp"

#define SENSOR_ID 0 // TODO: 

OpticalFlowPX4 * opticalFlowPX4;

int64_t time_prev;
int64_t sec_prev;

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  camera_setup(); // setting up and configuring the camera

  camera_fb_t * fb = esp_camera_fb_get();
  // OpticalFlowPX4(float f_length_x, float f_length_y, int ouput_rate = 15, int img_width = 64, int img_height = 64, int search_size = 6, int flow_feature_threshold = 30, int flow_value_threshold = 3000);
  opticalFlowPX4 = new OpticalFlowPX4(32, 32);
  opticalFlowPX4->setOutputRate(1000); // the maximum times a flow reported per second

  int a;
  float b;
  opticalFlowPX4->calcFlow(fb->buf, a, a, b, b); // init: the first image has nothing to compare to
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
  int flow_quality = opticalFlowPX4->calcFlow(fb->buf, (uint32_t)time_now, dt_us, flow_x, flow_y);

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
