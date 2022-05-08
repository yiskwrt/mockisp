#ifndef MOCKISP_H
#define MOCKISP_H

#include <media/media-device.h>
#include <media/v4l2-device.h>
#include <media/videobuf2-v4l2.h>

/* mockisp-dev.c */
#define MOCKISP_DRIVER_NAME "mockisp"
#define MOCKISP_BUS_INFO    "platform:" MOCKISP_DRIVER_NAME
#define MOCKISP_CAPTURE_DEV_NAME "mockisp:capture"
#define MOCKISP_RESIZE_DEV_NAME "mockisp:resize"

#define MOCKISP_DEFAULT_WIDTH 800
#define MOCKISP_DEFAULT_HEIGHT 600
#define MOCKISP_DEFAULT_FMT MEDIA_BUS_FMT_YUYV8_2X8

/* forward declaration */
struct mockisp_device;

/* mockisp-capture.c */
struct mockisp_buffer {
	struct vb2_v4l2_buffer vb;
	struct list_head queue;
};

struct mockisp_vdev_node {
	struct vb2_queue buf_queue;
	struct mutex vlock; /* serialize ioctl calls */
	struct video_device vdev;
	struct media_pad pad;
};

struct mockisp_capture {
	struct mockisp_vdev_node vnode;
	struct mockisp_device *mockisp;
};

int mockisp_capture_dev_register(struct mockisp_device *mockisp);
void mockisp_capture_dev_unregister(struct mockisp_device *mockisp);

/* mockisp-resize.c */
enum mockisp_resize_pad_id {
	MOCKISP_RESIZE_PAD_SINK,
	MOCKISP_RESIZE_PAD_SOURCE,
	MOCKISP_RESIZE_PADS_NUM
};

struct mockisp_resize {
	struct v4l2_subdev sd;
	struct mockisp_device *mockisp;
	struct media_pad pads[MOCKISP_RESIZE_PADS_NUM];
	struct v4l2_subdev_pad_config pad_cfg[MOCKISP_RESIZE_PADS_NUM];
	struct mutex ops_lock;
};

int mockisp_resize_dev_register(struct mockisp_device *mockisp);
void mockisp_resize_dev_unregister(struct mockisp_device *mockisp);

/* mockisp-dev.c */
struct mockisp_device {
	struct device *dev;
	struct v4l2_device v4l2_dev;
	struct media_device media_dev;
	struct media_pipeline pipe;

	struct mockisp_capture capture_dev;
	struct mockisp_resize resize_dev;
};


#endif
