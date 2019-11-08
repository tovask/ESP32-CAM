/*
  Main code, where the other component used from.
*/

#include "src/esp32_cam/camera.h"
#include "flow.hpp"
#include "lidar.hpp"
#include "mavlink.hpp"

int64_t time_prev;
int64_t sec_prev;

void setup() {


  //mavlink_system.sysid = 100; // System ID, 1-255
  //mavlink_system.compid = 50; // Component/Subsystem ID, 1-255



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
  float flow_rate_x, flow_rate_y;
  int flow_quality = getFlow(fb->buf, dt_us, flow_rate_x, flow_rate_y);

  uint16_t ground_distance = get_distance(); // from lidar

  // TODO: check if dt_us == (time_now - time_prev)

  //float flow_comp_m_x = - pixel_flow_x / focal_length / ( (time_now - time_prev) / 1000000.0f) * ground_distance;

  // TODO: calculate pixel flow
  int16_t pixel_flow_x = flow_rate_x;
  int16_t pixel_flow_y = flow_rate_y;

  float speed_x = tan(flow_rate_x) * (float)ground_distance;
  float speed_y = tan(flow_rate_y) * (float)ground_distance;

  Serial.printf("%" PRIu32 " dt: %d us,\tx: %f,\ty: %f,\tquality: %d,\tlidar: %" PRIu16 " \n", (uint32_t)time_now, dt_us, flow_rate_x, flow_rate_y, flow_quality, ground_distance);


  // sending the data
  send_flow(
    pixel_flow_x, // int16_t flow_x [dpix]
    pixel_flow_y, // int16_t flow_y [dpix]
    speed_x, // float flow_comp_m_x [m/s]
    speed_y, // float flow_comp_m_y [m/s]
    flow_quality, // uint8_t quality Optical flow quality / confidence. 0: bad, 255: maximum quality
    ground_distance // float ground_distance [m]
    //flow_rate_x, // float flow_rate_x [rad/s]
    //flow_rate_y // float flow_rate_y [rad/s]
  );

  esp_camera_fb_return(fb); // return the buffer to the pool

  time_prev = time_now;
}
