#include <media/v4l2-common.h>
#include <media/v4l2-event.h>
#include <media/v4l2-fh.h>
#include <media/v4l2-ioctl.h>
#include <media/v4l2-mc.h>
#include <media/v4l2-subdev.h>
#include <media/videobuf2-dma-contig.h>

#include "mockisp.h"

static inline struct mockisp_vdev_node *
mockisp_vdev_to_node(struct video_device *vdev)
{
	return container_of(vdev, struct mockisp_vdev_node, vdev);
}

/* ******************
 * File Handle
 */
static int mockisp_capture_link_validate(struct media_link *link)
{
	return 0;
};

static const struct media_entity_operations mockisp_media_ops = {
	.link_validate = mockisp_capture_link_validate,
};

/* ******************
 * Media Controller
 */
static const struct v4l2_file_operations mockisp_fops = {
	.open = v4l2_fh_open,
	.release = vb2_fop_release,
	.unlocked_ioctl = video_ioctl2,
	.poll = vb2_fop_poll,
	.mmap = vb2_fop_mmap,
};

/* ******************
 * VideoBuf2
 */
static int mockisp_vb2_queue_setup(struct vb2_queue *queue,
		unsigned int *num_buffers,
		unsigned int *num_planes,
		unsigned int sizes[],
		struct device *alloc_devs[])
{
	
	if (*num_planes) {
		;;
	} else {
		;;
	}

	return 0;
}

static void mockisp_vb2_buf_queue(struct vb2_buffer *vb)
{
}

static int mockisp_vb2_buf_prepare(struct vb2_buffer *vb)
{
	return 0;
}

static void mockisp_vb2_stop_streaming(struct vb2_queue *queue)
{
}

static int mockisp_vb2_start_streaming(struct vb2_queue *queue, unsigned int count)
{
	return -EINVAL;
}

static const struct vb2_ops mockisp_vb2_ops = {
	.queue_setup = mockisp_vb2_queue_setup,
	.buf_queue = mockisp_vb2_buf_queue,
	.buf_prepare = mockisp_vb2_buf_prepare,
	.wait_prepare = vb2_ops_wait_prepare,
	.wait_finish = vb2_ops_wait_finish,
	.stop_streaming = mockisp_vb2_stop_streaming,
	.start_streaming = mockisp_vb2_start_streaming,
};

/* ******************
 * IOCTL
 */
static const struct v4l2_ioctl_ops mockisp_v4l2_ioctl_ops = {
};


/* ******************
 * Register Device
 */
static void mockisp_capture_init(struct mockisp_device *mockisp)
{
	struct mockisp_capture *cap = &mockisp->capture_dev;

	memset(cap, 0, sizeof(*cap));
	cap->mockisp = mockisp;
}

static int mockisp_register_capture(struct mockisp_capture *cap)
{
	struct v4l2_device *v4l2_dev = &cap->mockisp->v4l2_dev;
	struct video_device *vdev = &cap->vnode.vdev;
	struct mockisp_vdev_node *node;
	struct vb2_queue *q;
	int ret;

	strscpy(vdev->name, MOCKISP_CAPTURE_DEV_NAME, sizeof(vdev->name));
	node = mockisp_vdev_to_node(vdev);
	mutex_init(&node->vlock);

	vdev->ioctl_ops = &mockisp_v4l2_ioctl_ops;
	vdev->release = video_device_release_empty;
	vdev->fops = &mockisp_fops;
	vdev->minor = -1; /* give me new minor id */
	vdev->v4l2_dev = v4l2_dev;
	vdev->lock = &node->vlock;
	vdev->device_caps = V4L2_CAP_VIDEO_CAPTURE_MPLANE | V4L2_CAP_STREAMING;
	vdev->device_caps |= V4L2_CAP_IO_MC;
	vdev->entity.ops = &mockisp_media_ops;
	video_set_drvdata(vdev, cap);
	vdev->vfl_dir = VFL_DIR_RX;
	node->pad.flags = MEDIA_PAD_FL_SINK;

	q = &node->buf_queue;
	q->type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
	q->io_modes = VB2_DMABUF;
	q->drv_priv = cap;
	q->ops = &mockisp_vb2_ops;
	q->mem_ops = &vb2_dma_contig_memops;
	q->buf_struct_size = sizeof(struct mockisp_buffer);
	q->min_buffers_needed = 2;
	q->timestamp_flags = V4L2_BUF_FLAG_TIMESTAMP_MONOTONIC;
	q->lock = &node->vlock;
	q->dev = cap->mockisp->dev;
	ret = vb2_queue_init(q);
	if (ret) {
		dev_err(cap->mockisp->dev, "vb2_queue_init failed: %d\n", ret);
		return ret;
	}
	
	vdev->queue = q;

	ret = video_register_device(vdev, VFL_TYPE_VIDEO, -1);
	if (ret) {
		dev_err(cap->mockisp->dev, "video_register_device failed: %d\n", ret);
		return ret;
	}

	v4l2_info(v4l2_dev, "registered %s as /dev/video%d\n", vdev->name, vdev->num);

	ret = media_entity_pads_init(&vdev->entity, 1, &node->pad);
	if (ret) {
		video_unregister_device(vdev);
		return ret;
	}

	return 0;
}

int mockisp_capture_dev_register(struct mockisp_device *mockisp)
{
	mockisp_capture_init(mockisp);
	return mockisp_register_capture(&mockisp->capture_dev);
}

void mockisp_capture_dev_unregister(struct mockisp_device *mockisp)
{
	struct mockisp_capture *cap = &mockisp->capture_dev;
	media_entity_cleanup(&cap->vnode.vdev.entity);
	vb2_video_unregister_device(&cap->vnode.vdev);
}
