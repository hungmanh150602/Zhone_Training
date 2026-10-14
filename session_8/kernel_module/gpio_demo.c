#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/gpio/consumer.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/err.h>

#define DRIVER_NAME "gpio-demo"
#define DEVICE_NAME "gpio_demo"

struct gpio_demo_data
{
    struct gpio_desc *led;
    struct gpio_desc *button;

    dev_t devt;
    struct cdev cdev;
    struct class *class;
    struct device *chardev;
};

/* Open: associate the file with our device data */
static int gpio_demo_open(struct inode *inode, struct file *file)
{
    struct gpio_demo_data *data;

    data = container_of(inode->i_cdev, struct gpio_demo_data, cdev);

    file->private_data = data;

    return 0;
}

/* Write: '1' turns LED on; '0' turns LED off */
static ssize_t gpio_demo_write(struct file *file,
                               const char __user *buf,
                               size_t count,
                               loff_t *ppos)
{
    struct gpio_demo_data *data = file->private_data;
    char cmd;

    if (count == 0)
        return 0;

    if (copy_from_user(&cmd, buf, 1))
        return -EFAULT;

    if (cmd == '1')
    {
        gpiod_set_value_cansleep(data->led, 1);
        pr_info("gpio_demo: LED ON\n");
    }
    else if (cmd == '0')
    {
        gpiod_set_value_cansleep(data->led, 0);
        pr_info("gpio_demo: LED OFF\n");
    }
    else
    {
        return -EINVAL;
    }

    return count;
}

/* Read: return current logical button state as '0' or '1' */
static ssize_t gpio_demo_read(struct file *file,
                              char __user *buf,
                              size_t count,
                              loff_t *ppos)
{
    struct gpio_demo_data *data = file->private_data;
    int value;
    char result;

    if (count == 0)
        return 0;

    value = gpiod_get_value_cansleep(data->button);
    if (value < 0)
        return value;

    result = value ? '1' : '0';

    if (copy_to_user(buf, &result, 1))
        return -EFAULT;

    return 1;
}

static const struct file_operations gpio_demo_fops = {
    .owner = THIS_MODULE,
    .open = gpio_demo_open,
    .read = gpio_demo_read,
    .write = gpio_demo_write,
    .llseek = noop_llseek,
};

static int gpio_demo_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    struct gpio_demo_data *data;
    int ret;

    dev_info(dev, "GPIO demo driver probe\n");

    data = devm_kzalloc(dev, sizeof(*data), GFP_KERNEL);
    if (!data)
        return -ENOMEM;

    /* Get LED GPIO and initialize it to logical OFF */
    data->led = devm_gpiod_get(dev, "led", GPIOD_OUT_LOW);
    if (IS_ERR(data->led))
    {
        ret = PTR_ERR(data->led);
        dev_err(dev, "Failed to get LED GPIO: %d\n", ret);
        return ret;
    }

    /* Get button GPIO as input */
    data->button = devm_gpiod_get(dev, "button", GPIOD_IN);
    if (IS_ERR(data->button))
    {
        ret = PTR_ERR(data->button);
        dev_err(dev, "Failed to get button GPIO: %d\n", ret);
        return ret;
    }

    /* Allocate a device number */
    ret = alloc_chrdev_region(&data->devt, 0, 1, DEVICE_NAME);
    if (ret)
    {
        dev_err(dev, "Failed to allocate device number: %d\n", ret);
        return ret;
    }

    /* Create a device class */
    data->class = class_create(DRIVER_NAME);
    if (IS_ERR(data->class))
    {
        ret = PTR_ERR(data->class);
        goto err_unregister_region;
    }

    /* Initialize and register the character device */
    cdev_init(&data->cdev, &gpio_demo_fops);

    ret = cdev_add(&data->cdev, data->devt, 1);
    if (ret)
    {
        dev_err(dev, "Failed to add cdev: %d\n", ret);
        goto err_destroy_class;
    }

    /* Create /dev/gpio_demo through the device model */
    data->chardev = device_create(data->class, dev,
                                  data->devt, data,
                                  DEVICE_NAME);
    if (IS_ERR(data->chardev))
    {
        ret = PTR_ERR(data->chardev);
        dev_err(dev, "Failed to create device node: %d\n", ret);
        goto err_del_cdev;
    }

    platform_set_drvdata(pdev, data);

    dev_info(dev, "Character device /dev/%s created\n", DEVICE_NAME);

    return 0;

err_del_cdev:
    cdev_del(&data->cdev);

err_destroy_class:
    class_destroy(data->class);

err_unregister_region:
    unregister_chrdev_region(data->devt, 1);

    return ret;
}

static void gpio_demo_remove(struct platform_device *pdev)
{
    struct gpio_demo_data *data = platform_get_drvdata(pdev);

    device_destroy(data->class, data->devt);
    cdev_del(&data->cdev);
    class_destroy(data->class);
    unregister_chrdev_region(data->devt, 1);

    dev_info(&pdev->dev, "GPIO demo driver remove\n");
}

static const struct of_device_id gpio_demo_of_match[] = {
    {.compatible = "training,gpio-demo"},
    {}};

MODULE_DEVICE_TABLE(of, gpio_demo_of_match);

static struct platform_driver gpio_demo_driver = {
    .probe = gpio_demo_probe,
    .remove = gpio_demo_remove,
    .driver = {
        .name = DRIVER_NAME,
        .of_match_table = gpio_demo_of_match,
    },
};

module_platform_driver(gpio_demo_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Hung Linux Driver Training");
MODULE_DESCRIPTION("Raspberry Pi GPIO LED and button character driver");