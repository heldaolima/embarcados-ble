#include "gatt_operations.h"
#include "ble_connection.h"

CFLAG(subscribed_f);
CFLAG(discover_f);
CFLAG(cwrite_f);

static struct bt_uuid_128 ble_upper = BT_UUID_INIT_128(0x01, 0x23, 0x45, 0x67, 0x89, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x00, 0x00);
static struct bt_uuid_128 ble_receive = BT_UUID_INIT_128(0x01, 0x23, 0x45, 0x67, 0x89, 0x01, 0x02, 0x03,0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0xFF, 0x00);
static struct bt_uuid_128 ble_notify = BT_UUID_INIT_128(0x01, 0x23, 0x45, 0x67, 0x89, 0x01, 0x02, 0x03,0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0xFF, 0x11);

uint16_t chrc_h;
static uint16_t notify_h;
static volatile size_t num_not;
static struct bt_gatt_discover_params disc_params;
static struct bt_uuid *primary_uuid = &ble_upper.uuid;

static struct bt_gatt_subscribe_params sub_params = {
	.notify = notify,
	.write = subscribe,
	.ccc_handle = 0, // Descoberta automática CCC
	.disc_params = &disc_params, // Descoberta automática CCC
	.end_handle = BT_ATT_LAST_ATTRIBUTE_HANDLE,
	.value = BT_GATT_CCC_NOTIFY,
};

uint8_t discover(struct bt_conn *conn, const struct bt_gatt_attr *attr, struct bt_gatt_discover_params *params)
{
    int err;

    if (attr == NULL) {
        if (!chrc_h) {
            printk("Did not discover long_chrc (%x)\n", chrc_h);
        }
        memset(params, 0, sizeof(*params));

        SFLAG(discover_f);
        return BT_GATT_ITER_STOP;
    }

    printk("[ATTRIBUTE] handle %u\n", attr->handle);
    if (params->type == BT_GATT_DISCOVER_PRIMARY && bt_uuid_cmp(params->uuid, &ble_upper.uuid) == 0) {
        printk("Found service\n");
        params->uuid = NULL;
        params->start_handle = attr->handle + 1;
        params->type = BT_GATT_DISCOVER_CHARACTERISTIC;

        err = bt_gatt_discover(conn, params);
        if (err != 0) {
            printk("Discover failed (err %d)\n", err);
        }
        return BT_GATT_ITER_STOP;
    } else if (params->type == BT_GATT_DISCOVER_CHARACTERISTIC) {
        struct bt_gatt_chrc *chrc = (struct bt_gatt_chrc *)attr->user_data;

        if (bt_uuid_cmp(chrc->uuid, &ble_receive.uuid) == 0) {
            printk("Found rvd_chrc\n");
            chrc_h = chrc->value_handle;
        } else if (bt_uuid_cmp(chrc->uuid, &ble_notify.uuid) == 0) {
            printk("Found notify_chrc\n");
            notify_h = chrc->value_handle;
        }
    }
    return BT_GATT_ITER_CONTINUE;
}

void subscribe(struct bt_conn *conn, uint8_t err, struct bt_gatt_write_params *params)
{
    if (err)
    {
        printk("Subscribe failed (err %d)\n", err);
    }

    SFLAG(subscribed_f);

    if (!params) 
    {
        printk("params NULL\n");
        return;
    }

    if (params->handle == notify_h)
    {
        printk("Subscribed to characteristic\n");
    }
    else
    {
        printk("Unknown handle %d\n", params->handle);
    }
}

uint8_t notify(struct bt_conn *conn, struct bt_gatt_subscribe_params *params, const void *data, uint16_t len) 
{
    printk("\nReceived notification #%u with length %d\n", num_not++, len);

    uint8_t string[len+1];
    memcpy(string, data, len);
    string[len] = '\0';

    printk("\nReceived data from peripheral: %s\n\n", string);

    memset((void*)data, 0, len);

    return BT_GATT_ITER_CONTINUE;
}

void gatt_subscribe(void)
{
	int err;
	UFLAG(subscribed_f);

	sub_params.value_handle = chrc_h;
	err = bt_gatt_subscribe(default_conn, &sub_params);

	if (err < 0) {
	    printk("Failed to subscribe\n");
	} else {
	    printk("Subscribe request sent\n");
	}

	WFLAG(subscribed_f);
}


void gatt_discover(void)
{
	printk("Discovering services and characteristics\n");
	static struct bt_gatt_discover_params discover_params;

	discover_params.uuid = primary_uuid;
	discover_params.func = discover;
	discover_params.start_handle = BT_ATT_FIRST_ATTRIBUTE_HANDLE;
	discover_params.end_handle = BT_ATT_LAST_ATTRIBUTE_HANDLE;
	discover_params.type = BT_GATT_DISCOVER_PRIMARY;

    if (bt_gatt_discover(default_conn, &discover_params)) {
        printk("Discover failed\n");
    } else {
        WFLAG(discover_f);
        printk("Discover complete\n");
    }
}

void gatt_write_cb(struct bt_conn *conn, uint8_t err, struct bt_gatt_write_params *params)
{
    if (err != BT_ATT_ERR_SUCCESS) {
        printk("Write failed: 0x%02X\n", err);
    }

    memset(params, 0, sizeof(*params));
    SFLAG(cwrite_f);
}

void gatt_write(uint16_t handle, char* chrc_data)
{
    static struct bt_gatt_write_params write_params;
    int err;

    if (handle == chrc_h)
    {
        printk("Writing to chrc\n");

        write_params.data = chrc_data;
        write_params.length = strlen(chrc_data);
    }

    write_params.func = gatt_write_cb;
    write_params.handle = handle;

    UFLAG(cwrite_f);

    err = bt_gatt_write(default_conn, &write_params);
    if (err != 0)
        printk("bt_gatt_write failed: %d\n", err);

    WFLAG(cwrite_f);
    printk("write success\n");
}