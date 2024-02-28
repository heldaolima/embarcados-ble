#ifndef BLE_CONNECTION_H
#define BLE_CONNECTION_H

#include <bluetooth/att.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>
#include <bluetooth/gatt.h>
#include <bluetooth/hci.h>
#include <bluetooth/uuid.h>
#include <console/console.h>
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/byteorder.h>
#include <sys/printk.h>
#include <sys/slist.h>
#include <zephyr.h>
#include <zephyr/types.h>

extern struct bt_conn *default_conn;
void start_scan(void);

#endif