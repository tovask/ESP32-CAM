
#include <esp_timer.h>
#include "src/flow_calc/flow_px4.hpp"


OpticalFlowPX4 * opticalFlowPX4;
#define FOCAL_LENGTH 200
//const float focal_length_px = (global_data.param[PARAM_FOCAL_LENGTH_MM]) / (4.0f * 6.0f) * 1000.0f; //original focal lenght: 12mm pixelsize: 6um, binning 4 enabled
//float focal_length = 250;
#define OUTPUT_RATE 1000 // the maximum times a flow reported per second
#define IMAGE_WIDTH 128 // FRAMESIZE_QQVGA2 => 128x160

void flow_setup(uint8_t *init_img) {
  // OpticalFlowPX4(float f_length_x, float f_length_y, int ouput_rate = 15, int img_width = 64, int img_height = 64, int search_size = 6, int flow_feature_threshold = 30, int flow_value_threshold = 3000);
  // note: img_height not used, img_width is used and the algorith assume a square image!
  opticalFlowPX4 = new OpticalFlowPX4(FOCAL_LENGTH, FOCAL_LENGTH, OUTPUT_RATE, IMAGE_WIDTH);

  int a; float b; // placeholders, not used
  // init the image buffer: for the first calculation, we will need a reference image to compare with
  opticalFlowPX4->calcFlow(init_img, a, a, b, b); // this will not compute anything when first called
}

int getFlow(uint8_t *img_current, int &dt_us, float &flow_x, float &flow_y) {
  int64_t time_now = esp_timer_get_time(); // time in microseconds since boot

  int flow_quality = opticalFlowPX4->calcFlow(img_current, (uint32_t)time_now, dt_us, flow_x, flow_y);

  return flow_quality;
}
