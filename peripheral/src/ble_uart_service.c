#include "ble_uart_service.h"
#include "case_converter.h"

static ble_uart_service_rx_callback rx_callback = NULL;
#define BLE_UART_SERVICE_TX_CHAR_OFFSET 3

#define CHRC_SIZE 100
static uint8_t chrc_data[CHRC_SIZE];

#define CFLAG(flag) static atomic_t flag = (atomic_t) false
#define SFLAG(flag) (void)atomic_set(&flag, (atomic_t) true)

CFLAG(flag_long_subscribe);

static struct bt_uuid_128 ble_uart_uppercase = BT_UUID_INIT_128(
    0x01, 0x23, 0x45, 0x67, 0x89, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x00, 0x00
);
static struct bt_uuid_128 ble_uart_receive_data = BT_UUID_INIT_128(
    0x01, 0x23, 0x45, 0x67, 0x89, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0xFF, 0x00
);
static struct bt_uuid_128 ble_uart_notify = BT_UUID_INIT_128(
    0x01, 0x23, 0x45, 0x67, 0x89, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0xFF, 0x11
);

#define isBufferOverflow(len) ((len) > sizeof(chrc_data))
#define offsetOverflow(len, offset) (((len) + (offset)) > sizeof(chrc_data))

void convertBufferToString(const void* buf, char str[], uint16_t len) {
    for (int i = 0; i < len; i++) {
        str[i] = *((char *)buf + i);
    }
    str[len] = '\0';
}

ssize_t uart_rx_callback( // called when data is received from write characteristc
    struct bt_conn *conn,
    const struct bt_gatt_attr *attr, 
    const void *buf, 
    uint16_t len, 
    uint16_t offset, 
    uint8_t flags) 
{
    static uint8_t prepare_count;

    char string[len + 1];
    convertBufferToString(buf, string, len);

    printk("\nReceived data: %s\n", string);

    if (isBufferOverflow(len)) {
        printk("Buffer overflow\n");
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
    } else if (offsetOverflow(len, offset)) {
        printk("Invalid offset and length\n");
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
    }

    if (flags & BT_GATT_WRITE_FLAG_PREPARE) {
        printk("prepare_count %u\n", prepare_count++);
        return BT_GATT_ERR(BT_ATT_ERR_SUCCESS);
    }

    (void)memcpy(chrc_data + offset, buf, len);
    prepare_count = 0;

    if (rx_callback) {
        rx_callback(buf, len);
    }

    return len;
}

static struct bt_gatt_attr attrs[] = {
    BT_GATT_PRIMARY_SERVICE(&ble_uart_uppercase),
    BT_GATT_CHARACTERISTIC(&ble_uart_receive_data.uuid, BT_GATT_CHRC_WRITE_WITHOUT_RESP, BT_GATT_PERM_WRITE, NULL, uart_rx_callback, NULL),
    BT_GATT_CHARACTERISTIC(&ble_uart_notify.uuid, BT_GATT_CHRC_NOTIFY, BT_GATT_PERM_NONE, NULL, NULL, NULL),
    BT_GATT_CCC(ble_uart_ccc_changed, BT_GATT_PERM_WRITE),
};

static struct bt_gatt_service peripheral = BT_GATT_SERVICE(attrs);

void ble_uart_ccc_changed(const struct bt_gatt_attr *attr, uint16_t value) {
    const bool areNotificationsEnabled = (value == BT_GATT_CCC_NOTIFY);
    if (areNotificationsEnabled)
        SFLAG(flag_long_subscribe);
    printk("Notifications %s\n", areNotificationsEnabled ? "enabled" : "disabled");
}

int ble_uart_service_register(const ble_uart_service_rx_callback callback) {
    rx_callback = callback;
    return bt_gatt_service_register(&peripheral);
}

bool bufferEmpty(const uint8_t *buffer, size_t len) {
    return !buffer || !len;
}

int uartTransmit(const uint8_t *buffer, size_t len) {
    printk("\nTransmitting...\n");

    if (bufferEmpty(buffer, len))
        return -1;

    struct bt_conn *conn = getConnection();

    uint8_t string[len + 1];
    convert_to_uppercase(buffer, string, len);

    if (conn)
        return (bt_gatt_notify(conn, &attrs[2], string, len));
    else
        return -1;
}