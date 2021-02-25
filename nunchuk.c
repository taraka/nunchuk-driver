// SPDX-License-Identifier: GPL-2.0
#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/input-polldev.h>

struct nunchuk_dev {
    struct i2c_client *i2c_client;
};

static int nunchuk_read_registers(struct i2c_client *client, char *buff, int size) {
    int err;
    char cmd[] = {0x00};

    msleep(20);

    err = i2c_master_send(client, cmd, sizeof(cmd));
    if (err != sizeof(cmd)) {
        return err;
    }

    msleep(20);

    err = i2c_master_recv(client, buff, size);
    return err;
}

static void nunchuk_i2c_poll(struct input_polled_dev *polled_input)
{
	struct nunchuk_dev *nunchuk;
	struct i2c_client *client;
    char reg[6];

    nunchuk = (struct nunchuk_dev *)polled_input->private;
	client = nunchuk->i2c_client;

    nunchuk_read_registers(client, reg, sizeof(reg));
    

    input_event(polled_input->input, EV_ABS, ABS_X, reg[0]);
	input_event(polled_input->input, EV_ABS, ABS_Y, reg[1]);
	input_event(polled_input->input, EV_REL, REL_X, (int)(reg[2] << 2) | ((reg[5]>>2) & 3));
	input_event(polled_input->input, EV_REL, REL_Y, (int)(reg[3] << 2) | ((reg[5]>>4) & 3));
	input_event(polled_input->input, EV_REL, REL_Z, (int)(reg[4] << 2) | ((reg[5]>>6) & 3));
    input_event(polled_input->input, EV_KEY, BTN_Z, !((reg[5] >> 0) & 1));
    input_event(polled_input->input, EV_KEY, BTN_C, !((reg[5] >> 1) & 1));
    input_sync(polled_input->input);
}


static int nunchuk_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    struct nunchuk_dev *nunchuk;
    struct input_polled_dev *polled_input;
    struct input_dev *input;
    int i, err;
        
    char init[][2] = {
        {0xf0, 0x55},
        {0xfb, 0x00}
    };

    for (i = 0; i < sizeof(init)/2; i++) {
        err = i2c_master_send(client, init[i], sizeof(init[i]));
        if (err == sizeof(init[i])) {
            pr_info("Init command #%d Success: (%d)", i, err);
        }
        else {
            pr_info("Init command #%d Error (%d)", i, err);
        }
        msleep(1);
    }

    polled_input = devm_input_allocate_polled_device(&client->dev);
    if (!polled_input)
		return -ENOMEM;

    nunchuk = devm_kzalloc(&client->dev,
			       sizeof(struct nunchuk_dev), GFP_KERNEL);
	if (!nunchuk)
		return -ENOMEM;

    nunchuk->i2c_client = client;
    polled_input->private = nunchuk;
    polled_input->poll = &nunchuk_i2c_poll;
	polled_input->poll_interval = 50;

    input = polled_input->input;
    input->name = "Wii Nunchuk";
    input->id.bustype = BUS_I2C;
    
	set_bit(EV_KEY, input->evbit);
    input_set_abs_params(input, ABS_X, 0, 255, 4, 8);
	input_set_abs_params(input, ABS_Y, 0, 255, 4, 8);
	set_bit(EV_ABS, input->evbit);
	set_bit(EV_REL, input->evbit);
	set_bit(BTN_C, input->keybit);
	set_bit(BTN_Z, input->keybit);
	set_bit(REL_X, input->relbit);
	set_bit(REL_Y, input->relbit);
	set_bit(REL_Z, input->relbit);
	set_bit(INPUT_PROP_ACCELEROMETER, input->propbit);


    err = input_register_polled_device(polled_input);
    if (err != 0)
		return -EIO;

    return 0;
}

static int nunchuk_i2c_remove(struct i2c_client *client)
{
	return 0;
}

static const struct i2c_device_id nunchuk_i2c_id[] = {
    { "nunchuk", 0 },
    { }
};
MODULE_DEVICE_TABLE(i2c, nunchuk_i2c_id);

static const struct of_device_id nunchuk_of_match[] = {
    { .compatible = "nintendo,nunchuk" },
    { },
};
MODULE_DEVICE_TABLE(of, nunchuk_of_match);

static struct i2c_driver nunchuk_i2c_driver = {
.driver = {
        .name = "nunchuk",
        .of_match_table = nunchuk_of_match,
    },
    .probe = nunchuk_i2c_probe,
    .remove = nunchuk_i2c_remove,
    .id_table = nunchuk_i2c_id,
};
module_i2c_driver(nunchuk_i2c_driver);

MODULE_LICENSE("GPL");

