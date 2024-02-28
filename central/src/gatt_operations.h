#ifndef GATT_OPERATIONS_H
#define GATT_OPERATIONS_H

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

#define CFLAG(flag) static atomic_t flag = ATOMIC_INIT(false)
#define SFLAG(flag) atomic_set(&flag, true)
#define UFLAG(flag) atomic_set(&flag, false)
#define WFLAG(flag) \
    while (!atomic_get(&flag)) { \
        k_sleep(K_MSEC(1)); \
    }
#define BT_ATT_LAST_ATTRIBUTE_HANDLE  0xffff
#define BT_ATT_FIRST_ATTRIBUTE_HANDLE 0x0001

void subscribe(struct bt_conn *conn, uint8_t err, struct bt_gatt_write_params *params);
uint8_t notify(struct bt_conn *conn, struct bt_gatt_subscribe_params *params, const void *data, uint16_t len);
uint8_t discover(struct bt_conn *conn, const struct bt_gatt_attr *attr, struct bt_gatt_discover_params *params);
void gatt_subscribe(void);
void gatt_discover(void);
void gatt_write_cb(struct bt_conn *conn, uint8_t err, struct bt_gatt_write_params *params);
void gatt_write(uint16_t handle, char* chrc_data);

extern uint16_t chrc_h;

#endif