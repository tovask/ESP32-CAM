/*
https://github.com/mavlink/c_library_v1/blob/master/common/mavlink_msg_optical_flow.h

https://github.com/PX4/Flow/blob/master/src/modules/flow/flow.c
https://github.com/PX4/OpticalFlow/blob/master/src/px4flow.cpp

*/
#include "esp_camera.h"
#include "flow_px4.hpp"

// Select camera model
#define CAMERA_MODEL_AI_THINKER  // CAMERA_MODEL_WROVER_KIT CAMERA_MODEL_ESP_EYE CAMERA_MODEL_M5STACK_PSRAM CAMERA_MODEL_M5STACK_WIDE
#include "camera_pins.h"

OpticalFlowPX4 * opticalFlowPX4;

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();


  //  https://github.com/espressif/arduino-esp32/tree/master/libraries/ESP32/examples/Camera/CameraWebServer
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;

  config.xclk_freq_hz = 20000000;
  /*
    config.pixel_format = PIXFORMAT_JPEG;
    //init with high specs to pre-allocate larger buffers
    if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
    } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
    }
  */
  config.pixel_format = PIXFORMAT_GRAYSCALE;
  config.frame_size = FRAMESIZE_QQVGA;
  config.jpeg_quality = 10;
  config.fb_count = 1;

#if defined(CAMERA_MODEL_ESP_EYE)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  //initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1);//flip it back
    s->set_brightness(s, 1);//up the blightness just a bit
    s->set_saturation(s, -2);//lower the saturation
  }
  //drop down frame size for higher initial frame rate
  s->set_framesize(s, FRAMESIZE_QVGA);

#if defined(CAMERA_MODEL_M5STACK_WIDE)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif


  camera_fb_t * fb = esp_camera_fb_get();
  // OpticalFlowPX4(float f_length_x, float f_length_y, int ouput_rate = 15, int img_width = 64, int img_height = 64, int search_size = 6, int flow_feature_threshold = 30, int flow_value_threshold = 3000);
  opticalFlowPX4 = new OpticalFlowPX4(32, 32);
  //opticalFlowPX4 = new OpticalFlowPX4(fb->width/2, fb->height/2);
  //opticalFlowPX4->setImageWidth(fb->width);
  //opticalFlowPX4->setImageHeight(fb->height);
  opticalFlowPX4->setOutputRate(1000); // limit the times a flow reported per second
  
  int a;
  float b;
  opticalFlowPX4->calcFlow(fb->buf, a, a, b, b);

}

int64_t time_prev;
int64_t sec_prev;

void loop() {
  camera_fb_t * fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return; // ESP_FAIL;
  }
  //  if (fb->format != PIXFORMAT_JPEG) {
  //    bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
  //    esp_camera_fb_return(fb);
  //    fb = NULL;
  //    if (!jpeg_converted) {
  //      Serial.println("JPEG compression failed");
  //      res = ESP_FAIL;
  //    }
  //  }

  int64_t time_now = esp_timer_get_time();
  int64_t sec_now = time_now / 1000000;
  if (sec_now != sec_prev) {
    //Serial.printf("JPG: %u byte , %u ms, %u fps, %" PRId64 " \n", (uint32_t)(fb->len), (uint32_t)((time_now - time_prev) / 1000), (uint32_t)(1000 * 1000 / (time_now - time_prev)), time_now);
    sec_prev = sec_now;
  }
  time_prev = time_now;


  uint8_t avg = fb->buf[0];
  for (int i = 1; i < fb->len; i++)
  {
    avg += (fb->buf[i] - avg) / i;
  }

  int dt_us;
  float flow_x, flow_y;
  int flow_quality = opticalFlowPX4->calcFlow(fb->buf, (uint32_t)time_now, dt_us, flow_x, flow_y);
  Serial.printf("%" PRIu32 " dt: %d us,\tx: %f,\ty: %f,\tquality: %d,\tavg: %" PRIu8 "\n", (uint32_t)time_now, dt_us, flow_x, flow_y, flow_quality, avg);


/*
https://github.com/PX4/Flow/blob/master/src/modules/flow/main.c#L572
// send flow
	mavlink_msg_optical_flow_send(MAVLINK_COMM_0, get_boot_time_us(), global_data.param[PARAM_SENSOR_ID],
			pixel_flow_x_sum * 10.0f, pixel_flow_y_sum * 10.0f,
			flow_comp_m_x, flow_comp_m_y, qual, ground_distance);

	mavlink_msg_optical_flow_rad_send(MAVLINK_COMM_0, get_boot_time_us(), global_data.param[PARAM_SENSOR_ID],
			integration_timespan, accumulated_flow_x, accumulated_flow_y,
			accumulated_gyro_x, accumulated_gyro_y, accumulated_gyro_z,
			gyro_temp, accumulated_quality/accumulated_framecount,
time_since_last_sonar_update,ground_distance);
*/

  esp_camera_fb_return(fb);
}
