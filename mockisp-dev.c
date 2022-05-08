#include <linux/module.h>
#include <linux/of.h>
#include <linux/types.h>
#include <linux/platform_device.h>

#include "mockisp.h"

static int mockisp_entities_register(struct mockisp_device *mockisp)
{
	int ret;

	/* device subdevs */
	ret = mockisp_capture_dev_register(mockisp);
	if (ret)
		goto err_capture_dev;
	
	ret = mockisp_resize_dev_register(mockisp);
	if (ret)
		goto err_resize_dev;

	/* sensor subdevs from fwnode */

	return 0;

err_resize_dev:
	mockisp_capture_dev_unregister(mockisp);
err_capture_dev:
	return ret;
}

static void mockisp_entities_unregister(struct mockisp_device *mockisp)
{
	mockisp_resize_dev_unregister(mockisp);
	mockisp_capture_dev_unregister(mockisp);
}

static int mockisp_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct mockisp_device *mockisp;
	struct v4l2_device *v4l2_dev;
	int ret;

	/* greetings */
	pr_info("mockisp_probe called");

	mockisp = devm_kzalloc(dev, sizeof(*mockisp), GFP_KERNEL);
	if (!mockisp)
		return -ENOMEM;

	mockisp->dev = dev;

	/* build media_dev */
	mockisp->media_dev.hw_revision = 0;
	strscpy(mockisp->media_dev.model, MOCKISP_DRIVER_NAME, sizeof(mockisp->media_dev.model));
	mockisp->media_dev.dev = dev;
	strscpy(mockisp->media_dev.bus_info, MOCKISP_BUS_INFO, sizeof(mockisp->media_dev.bus_info));
	media_device_init(&mockisp->media_dev);

	/* build v4l2_dev */
	v4l2_dev = &mockisp->v4l2_dev;
	v4l2_dev->mdev = &mockisp->media_dev;
	strscpy(v4l2_dev->name, MOCKISP_DRIVER_NAME, sizeof(v4l2_dev->name));

	/* register */
	ret = v4l2_device_register(mockisp->dev, &mockisp->v4l2_dev);
	if (ret) {
		dev_err(dev, "Failed to register v4l2 device: %d\n", ret);
		return ret;
	}

	ret = media_device_register(&mockisp->media_dev);
	if (ret) {
		dev_err(dev, "Failed to register media device: %d\n", ret);
		goto err_unreg_v4l2_dev;
	}

	ret = mockisp_entities_register(mockisp);
	if (ret)
		goto err_unreg_media_dev;

	return 0;

	/* error handling */
err_unreg_media_dev:
	media_device_unregister(&mockisp->media_dev);

err_unreg_v4l2_dev:
	v4l2_device_unregister(&mockisp->v4l2_dev);
	return ret;
}

static int mockisp_remove(struct platform_device *pdev)
{
	struct mockisp_device *mockisp = platform_get_drvdata(pdev);

	pr_info("mockisp_remove called");
	mockisp_entities_unregister(mockisp);
	media_device_unregister(&mockisp->media_dev);
	v4l2_device_unregister(&mockisp->v4l2_dev);

	return 0;
}

static const struct of_device_id mockisp_of_match[] = {
	{
		.compatible = "mockisp,mockisp",
	},
	{},
};

static struct platform_driver mockisp_drv = {
	.driver = {
		.name = MOCKISP_DRIVER_NAME,
		.of_match_table = of_match_ptr(mockisp_of_match),
	},
	.probe = mockisp_probe,
	.remove = mockisp_remove,
};

module_platform_driver(mockisp_drv);
MODULE_DESCRIPTION("Mock Video platform driver");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("Yuji Ishikawa");
