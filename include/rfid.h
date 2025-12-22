

#pragma once

/**
 * @file rfid.h
 * @brief RFID tag type definitions and driver API
 *
 * Bitflags for supported tag types and the small driver API used by
 * board-level RFID drivers.
 */

#define RFID_TAG_MIFARE_CLASSIC_EV1     BIT(0)
#define RFID_TAG_MIFARE_PLUS_EV1        BIT(1)
#define RFID_TAG_MIFARE_PLUS_X          BIT(2)
#define RFID_TAG_MIFARE_PLUS_EV2        BIT(3)
#define RFID_TAG_MIFARE_ULTRALIGHT_C    BIT(4)
#define RFID_TAG_MIFARE_ULTRALIGHT_EV1  BIT(5)
#define RFID_TAG_MIFARE_ULTRALIGHT_NANO BIT(6)
#define RFID_TAG_MIFARE_ULTRALIGHT_AES  BIT(7)
#define RFID_TAG_MIFARE_DESFIR_EV1      BIT(8)
#define RFID_TAG_MIFARE_DESFIR_EV2      BIT(9)
#define RFID_TAG_MIFARE_DESFIR_EV3      BIT(10)
#define RFID_TAG_MIFARE_DUOX            BIT(11)

typedef bool (*rfid_poll_t)(const struct device * dev);

  
typedef struct rfid_driver_api {
  rfid_poll_t poll;
} rfid_api_t;
