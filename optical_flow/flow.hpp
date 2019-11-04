
#pragma once

void flow_setup(uint8_t *init_img);
int getFlow(uint8_t *img_current, int &dt_us, float &flow_x, float &flow_y);
