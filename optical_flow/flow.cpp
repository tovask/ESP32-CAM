
#include <esp_timer.h>
#include "src/flow_calc/flow_px4.hpp"


OpticalFlowPX4 * opticalFlowPX4;

void flow_setup(){
	// OpticalFlowPX4(float f_length_x, float f_length_y, int ouput_rate = 15, int img_width = 64, int img_height = 64, int search_size = 6, int flow_feature_threshold = 30, int flow_value_threshold = 3000);
	opticalFlowPX4 = new OpticalFlowPX4(32, 32);
	opticalFlowPX4->setOutputRate(1000); // the maximum times a flow reported per second
	// TODO: init buffer
}

int getFlow(uint8_t *img_current, int &dt_us, float &flow_x, float &flow_y){
	int64_t time_now = esp_timer_get_time(); // time in microseconds since boot
	
	return opticalFlowPX4->calcFlow(img_current, (uint32_t)time_now, dt_us, flow_x, flow_y);
}
