#ifndef CONFIG_H
#define CONFIG_H

#include "stm32f4xx_hal.h"

#define ROPE_COUNT                       4U

#define PLATFORM_HEIGHT_CM              20.0f
#define CAMERA_Y_OFFSET_CM               5.0f
#define SPOOL_DIAMETER_CM                3.4f
#define PI_F                             3.1415926f
#define SPOOL_CIRCUMFERENCE_CM          (PI_F * SPOOL_DIAMETER_CM)

#define MOTOR_STEPS_PER_REV            200.0f
#define MOTOR_MICROSTEP                16.0f
#define MOTOR_TO_SPOOL_RATIO             1.0f
#define CM_PER_PULSE                   (SPOOL_CIRCUMFERENCE_CM / (MOTOR_STEPS_PER_REV * MOTOR_MICROSTEP))

#define MAX_MOTOR_PULSE_RATE_HZ       1600U
#define MIN_MOTOR_PULSE_RATE_HZ        180U
#define HOME_CENTER_TOL_CM              1.0f
#define HOME_RETRY_MAX                    3U
#define MOTION_FEEDBACK_PERIOD_MS       50U
#define ABS_ENCODER_RETRY_MAX            3U
#define ABS_ENCODER_TIMEOUT_MS          30U
#define VISION_PACKET_TIMEOUT_MS       300U
#define APP_LOOP_DELAY_MS                2U
#define HMI_REFRESH_PERIOD_MS          120U
#define HMI_ENABLED                      0U
#define FIRE_LINK_DEBOUNCE_MS           20U
#define FIRE_RECORD_MAX                  2U

/* 缁濆鍊肩紪鐮佸櫒/椹卞姩鍣ㄤ覆鍙� TTL 鍦板潃 */
#define M1_TTL_ADDR                      1U
#define M2_TTL_ADDR                      2U
#define M3_TTL_ADDR                      3U
#define M4_TTL_ADDR                      4U

/*
 * 缁濆鍊肩紪鐮佸櫒鏍囧畾鍙傛暟
 * 闇€瑕佸湪瑁呴厤瀹屾垚鍚庯紝浠ユ煇涓€宸茬煡濮挎€佽褰曞悇杞寸粷瀵硅搴﹀拰瀵瑰簲缁抽暱銆�
 * 涓嬮潰绀轰緥榛樿鍙傝€冨Э鎬佸氨鏄縺鍏夌偣浣嶄簬涓績鐐� (0,0)銆�
 */
#define M1_REF_ANGLE_DEG                0.0f
#define M2_REF_ANGLE_DEG                0.0f
#define M3_REF_ANGLE_DEG                0.0f
#define M4_REF_ANGLE_DEG                0.0f

#define M1_REF_ROPE_CM                 57.45f
#define M2_REF_ROPE_CM                 57.45f
#define M3_REF_ROPE_CM                 57.45f
#define M4_REF_ROPE_CM                 57.45f

/*
 * 缂栫爜鍣ㄨ搴﹀彉鍖栧缁抽暱鐨勫奖鍝嶆柟鍚戙€�
 * 鑻ヨ杞磋搴﹀澶ф椂鍗风怀鏀剁揣锛屽垯璁句负 -1锛�
 * 鑻ヨ杞磋搴﹀澶ф椂鍗风怀鏀鹃暱锛屽垯璁句负 +1銆�
 */
#define M1_ANGLE_TO_LEN_SIGN           (-1.0f)
#define M2_ANGLE_TO_LEN_SIGN           (-1.0f)
#define M3_ANGLE_TO_LEN_SIGN           (-1.0f)
#define M4_ANGLE_TO_LEN_SIGN           (-1.0f)

/* 鍥涗釜閿氱偣鍧愭爣锛屽崟浣� cm锛屼互搴曟澘涓績涓哄師鐐� */
#define A1_X_CM                       (-40.0f)
#define A1_Y_CM                        (40.0f)
#define A1_Z_CM                        (30.0f)

#define A2_X_CM                        (40.0f)
#define A2_Y_CM                        (40.0f)
#define A2_Z_CM                        (30.0f)

#define A3_X_CM                        (40.0f)
#define A3_Y_CM                       (-40.0f)
#define A3_Z_CM                        (30.0f)

#define A4_X_CM                       (-40.0f)
#define A4_Y_CM                       (-40.0f)
#define A4_Z_CM                        (30.0f)

/* 鍥涗釜杈圭紭姗欒壊鍦嗕腑蹇冪殑鍙傝€冨潗鏍囷紝鍚庣画鍙寜瀹炴祴淇敼 */
#define PATROL_P1_X_CM               (-20.0f)
#define PATROL_P1_Y_CM                (20.0f)
#define PATROL_P2_X_CM                (20.0f)
#define PATROL_P2_Y_CM                (20.0f)
#define PATROL_P3_X_CM                (20.0f)
#define PATROL_P3_Y_CM               (-20.0f)
#define PATROL_P4_X_CM               (-20.0f)
#define PATROL_P4_Y_CM               (-20.0f)

/* UART 鍗忚 */
#define VISION_UART_SOF1               0xAA
#define VISION_UART_SOF2               0x55
#define VISION_UART_CMD_HOME           0x31
#define VISION_UART_PAYLOAD_LEN        10U

#endif
