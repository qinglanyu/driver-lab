#include <linux/gpio/consumer.h>
#include <linux/leds.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>

struct qemu_gpio_led {
    struct led_classdev cdev;
    struct gpio_desc *gpiod;
};

static int qemu_gpio_led_set(struct led_classdev *cdev,
                             enum led_brightness brightness)
{
    struct qemu_gpio_led *led = container_of(cdev, struct qemu_gpio_led, cdev);
    gpiod_set_value_cansleep(led->gpiod, brightness != LED_OFF);
    return 0;
}

static int qemu_gpio_led_probe(struct platform_device *pdev)
{
    struct qemu_gpio_led *led = NULL;
    const char *label = "qemu-platform";
    int ret;

    printk("ready to %s ...\n", __FUNCTION__);

    led = devm_kzalloc(&pdev->dev, sizeof(*led), GFP_KERNEL);
    if (!led)
        return -ENOMEM;

    of_property_read_string(pdev->dev.of_node, "label", &label);
    led->gpiod = devm_gpiod_get(&pdev->dev, NULL, GPIOD_OUT_LOW);
    if (IS_ERR(led->gpiod))
        return PTR_ERR(led->gpiod);

    led->cdev.name = label;
    led->cdev.max_brightness = LED_FULL;
    led->cdev.brightness_set_blocking = qemu_gpio_led_set;
    ret = devm_led_classdev_register(&pdev->dev, &led->cdev);
    if (ret)
        return ret;

    dev_info(&pdev->dev, "bould GPIO LED: %s\n", label);
    return 0;
}

static const struct of_device_id qemu_gpio_led_of_match[] = {
    { .compatible = "linux-study,qemu-led" },
    {}
};
MODULE_DEVICE_TABLE(of, qemu_gpio_led_of_match);  // 让系统找到并加载驱动模块

static struct platform_driver qemu_gpio_led_driver = {
    .probe = qemu_gpio_led_probe,
    .driver = {
        .name = "qemu_gpio_led",
        .of_match_table = qemu_gpio_led_of_match,
    },
};
module_platform_driver(qemu_gpio_led_driver);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("QEMU GPIO LED platform driver");
