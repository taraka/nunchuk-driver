// SPDX-License-Identifier: GPL-2.0
#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>


static int nunchuk_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    pr_info("nunchuk loaded");
	return 0;
}

static int nunchuk_i2c_remove(struct i2c_client *client)
{
    pr_info("nunchuk unloaded");
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

