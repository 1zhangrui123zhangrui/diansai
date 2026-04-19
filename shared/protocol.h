#ifndef __PROTOCOL_H
#define __PROTOCOL_H

#include <stdint.h>

/*
 * Serial HMI -> F407 frame format:
 * 0x55 0x55 + CMD + [DATA] + 0xFE
 */
#define CMD_HOME            0xA0u
#define CMD_START           0xA1u
#define CMD_AREA_PATROL     0xA2u
#define CMD_SEQ_PATROL      0xA3u
#define CMD_AUTO_PATROL     0xA4u
#define CMD_CALIBRATE       0xA5u

#define SCREEN_FRAME_HEAD   0x55u
#define SCREEN_FRAME_TAIL   0xFEu

#define SCREEN_PAGE_REPORT_BASE  0xB0u
#define SCREEN_PAGE_HOME         0u
#define SCREEN_PAGE_START        1u
#define SCREEN_PAGE_AREA         2u
#define SCREEN_PAGE_SEQUENCE     3u
#define SCREEN_PAGE_AUTO         4u
#define SCREEN_PAGE_FIRE         5u

/*
 * F103 -> F407 vision packet over nRF24L01.
 */
#define VISION_PKT_HEAD     0xAAu

typedef struct __attribute__((packed)) {
    uint8_t head;
    uint8_t fire_flag;
    uint8_t seq;
    uint8_t checksum;
} VisionPacket_t;

#define NRF_CHANNEL         90u
#define NRF_PAYLOAD_SIZE    sizeof(VisionPacket_t)
#define NRF_ADDR_WIDTH      5u

#define NRF_ADDR_BYTE0      0xE7u
#define NRF_ADDR_BYTE1      0xE7u
#define NRF_ADDR_BYTE2      0xE7u
#define NRF_ADDR_BYTE3      0xE7u
#define NRF_ADDR_BYTE4      0xE7u

#define CAM_UART_BAUD       115200u

#define ANCHOR_POS_X1       (-40.0f)
#define ANCHOR_POS_Y1       (-40.0f)
#define ANCHOR_POS_X2       ( 40.0f)
#define ANCHOR_POS_Y2       (-40.0f)
#define ANCHOR_POS_X3       ( 40.0f)
#define ANCHOR_POS_Y3       ( 40.0f)
#define ANCHOR_POS_X4       (-40.0f)
#define ANCHOR_POS_Y4       ( 40.0f)
#define ANCHOR_POS_Z        ( 30.0f)
#define PLATFORM_H          ( 10.0f)

#define ORANGE_CENTER_X     (  0.0f)
#define ORANGE_CENTER_Y     (  0.0f)
#define ORANGE_LB_X         (-20.0f)
#define ORANGE_LB_Y         (-20.0f)
#define ORANGE_RB_X         ( 20.0f)
#define ORANGE_RB_Y         (-20.0f)
#define ORANGE_RT_X         ( 20.0f)
#define ORANGE_RT_Y         ( 20.0f)
#define ORANGE_LT_X         (-20.0f)
#define ORANGE_LT_Y         ( 20.0f)

#define MAX_SPEED_CM_S      10.0f
#define POSITION_TOL_CM     0.5f
#define SAFETY_BOUND        28.0f

#define MAX_FIRE_COUNT      2u
#define FIRE_DEDUP_DIST_CM  5.0f

#endif /* __PROTOCOL_H */
