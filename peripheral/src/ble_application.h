#ifndef _ble_application_h
#define _ble_application_h

#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <sys/printk.h>
#include <sys/byteorder.h>
#include <sys/util.h>
#include <sys_clock.h>
#include <zephyr.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>
#include <kernel.h>

typedef enum {
    kBleConnected,
    kBleDisconnected,
} ble_status;

typedef void (*ble_ready_callback) (struct bt_conn *connection);

uint32_t startBluetooth(ble_ready_callback callback);
struct bt_conn *getConnection();

#endif