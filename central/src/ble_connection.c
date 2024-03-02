#include "ble_connection.h"

struct bt_conn *default_conn;

bool isThereAConnectionAlreadyEstablished() {
    return default_conn != NULL;
}

// checks if device is close enough to establish a connection
bool isSignalStrongEnough(int8_t rssi) { 
    return rssi < -70;
}

void device_found( // what to do when a new device is found
    const bt_addr_le_t *deviceAddr, 
    int8_t rssi, 
    uint8_t type, 
    struct net_buf_simple *ad
    ) {
    char addr_str[BT_ADDR_LE_STR_LEN];
    int err;

    if (!isThereAConnectionAlreadyEstablished()) {
        // We're only interested in connectable events
        if (type == BT_GAP_ADV_TYPE_ADV_IND ||
            type == BT_GAP_ADV_TYPE_ADV_DIRECT_IND)
        {

            bt_addr_le_to_str(deviceAddr, addr_str, sizeof(addr_str));
            printk("Device found: %s (RSSI %d)\n", addr_str, rssi);

            if (isSignalStrongEnough(rssi) || bt_le_scan_stop()) {
                return;
            }

            err = bt_conn_le_create(
                deviceAddr,
                BT_CONN_LE_CREATE_CONN,   // create connection
                BT_LE_CONN_PARAM_DEFAULT, // default configuration
                &default_conn);

            if (err) {
                printk("Create conn to %s failed (%d)\n", addr_str, err);
                start_scan();
            }
        }
    }
}

void start_scan(void) {
    int err;

    err = bt_le_scan_start(BT_LE_SCAN_PASSIVE, device_found);
    if (err) {
        printk("Scanning failed to start (err %d)\n", err);
    } else {
        printk("Scanning successfully started\n");
    }
}

void unrefConnection(struct bt_conn *conn) {
    bt_conn_unref(default_conn);
    default_conn = NULL;
}

bool isConnectionEstablished(struct bt_conn *conn) {
    return conn == default_conn;
}

// what will happen when the device is disconnected
void connected(struct bt_conn *conn, uint8_t err) { 
    char addr[BT_ADDR_LE_STR_LEN];

    bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

    if (err) {
        printk("Failed to connect to %s (%u)\n", addr, err);
        unrefConnection(conn);
        start_scan();
    } else if (isConnectionEstablished(conn)) {
        printk("Connected: %s\n", addr);
    }
}


void disconnected(struct bt_conn *conn, uint8_t reason) {
    char addr[BT_ADDR_LE_STR_LEN];

    if (isConnectionEstablished(conn)) {
        bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

        printk("Disconnected: %s (reason 0x%02x)\n", addr, reason);

        int err = bt_conn_disconnect(conn, BT_HCI_ERR_REMOTE_USER_TERM_CONN);
        if (err) {
            printk("Failed to disconnect: %d\n", err);
        }

        unrefConnection(conn);
        start_scan();
    }
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
	.connected = connected,
	.disconnected = disconnected,
};
