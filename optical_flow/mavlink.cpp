/*
   TODO: implement & use MAVLINK_SEND_UART_BYTES
  https://mavlink.io/en/mavgen_c/
  https://github.com/mavlink/c_library_v1/blob/master/common/mavlink_msg_optical_flow.h#L200
  

*/


#define SENSOR_ID 0 // TODO: 


#define MAVLINK_USE_CONVENIENCE_FUNCTIONS

#include "src/c_library_v1/mavlink_types.h"
//#include "src/c_library_v2/mavlink_types.h"

static mavlink_system_t mavlink_system = {
  100, // sysid: System ID, 1-255
  50, // compid: Component/Subsystem ID, 1-255
};

// TODO: implement & use MAVLINK_SEND_UART_BYTES instead of comm_send_ch
// see https://github.com/mavlink/c_library_v1/blob/master/mavlink_helpers.h#L620
/**
   @brief Send one char (uint8_t) over a comm channel

   @param chan MAVLink channel to use, usually MAVLINK_COMM_0 = UART0
   @param ch Character to send
*/
inline void comm_send_ch(mavlink_channel_t chan, uint8_t ch)
{
  if (chan == MAVLINK_COMM_0)
  {
    //uart0_transmit(ch);
  }
  if (chan == MAVLINK_COMM_1)
  {
    //uart1_transmit(ch);
  }
}


#include "src/c_library_v1/common/mavlink.h"
//#include "src/mavlink_c_library_v2/common/mavlink.h"

#include <esp_timer.h>

void send_flow(int16_t pixel_flow_x, int16_t pixel_flow_y, float speed_x, float speed_y, uint8_t flow_quality, float ground_distance)
{
  mavlink_msg_optical_flow_send(
    MAVLINK_COMM_0, // mavlink_channel_t chan
    esp_timer_get_time() * 1000, // uint64_t time_usec
    SENSOR_ID, // uint8_t sensor_id
    pixel_flow_x, // int16_t flow_x [dpix]
    pixel_flow_y, // int16_t flow_y [dpix]
    speed_x, // float flow_comp_m_x [m/s]
    speed_y, // float flow_comp_m_y [m/s]
    flow_quality, // uint8_t quality Optical flow quality / confidence. 0: bad, 255: maximum quality
    ground_distance // float ground_distance [m]
    //flow_rate_x, // float flow_rate_x [rad/s]
    //flow_rate_y // float flow_rate_y [rad/s]
  );
  /*
      mavlink_msg_optical_flow_rad_send(MAVLINK_COMM_0, get_boot_time_us(), SENSOR_ID,
          integration_timespan, accumulated_flow_x, accumulated_flow_y,
          accumulated_gyro_x, accumulated_gyro_y, accumulated_gyro_z,
          gyro_temp, accumulated_quality/accumulated_framecount,
      time_since_last_sonar_update,ground_distance);
  */
}
