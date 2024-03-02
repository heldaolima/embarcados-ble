#include "gatt_operations.h"
#include "ble_connection.h"

void read_input(void) {
	console_getline_init();
	bool isGattInitialized = true;
	while (true) {
		printk("Entry ->: ");
		char *s = console_getline();

		printk("Received entry -> : %s\n", s);
		if (isGattInitialized) {
			gatt_discover();
			gatt_subscribe();
		}

		gatt_write(chrc_h, s);
		isGattInitialized = false;
	}
}

void main(void) {
	int err = bt_enable(NULL); // initializes bluetooth
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
		return;
	}

	printk("Bluetooth initialized\n");

	start_scan();
	read_input();
}
