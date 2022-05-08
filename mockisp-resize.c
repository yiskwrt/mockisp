#include <media/v4l2-common.h>
#include <media/v4l2-subdev.h>

#include "mockisp.h"

/* **************************
 * V4L2 media controller 
 */
static const struct media_entity_operations mockisp_resize_media_ops = {
	.link_validate = v4l2_subdev_link_validate,
};

/* **************************
 * V4L2 Subdev ops
 */
static int mockisp_resize_s_stream(struct v4l2_subdev *sd, int enable)
{
	return -EINVAL;
}

static const struct v4l2_subdev_pad_ops mockisp_resize_pad_ops = {
	.link_validate = v4l2_subdev_link_validate_default,
};

static const struct v4l2_subdev_video_ops mockisp_resize_video_ops = {
	.s_stream = mockisp_resize_s_stream,
};

static const struct v4l2_subdev_ops mockisp_resize_ops = {
	.video = &mockisp_resize_video_ops,
	.pad   = &mockisp_resize_pad_ops,
};

static int mockisp_resize_init_config(struct v4l2_subdev *sd, struct v4l2_subdev_state *sd_state)
{
	struct v4l2_mbus_framefmt *sink_fmt, *src_fmt;
	struct v4l2_rect *sink_crop;

	sink_fmt = v4l2_subdev_get_try_format(sd, sd_state, MOCKISP_RESIZE_PAD_SOURCE);
	sink_fmt->width  = MOCKISP_DEFAULT_WIDTH;
	sink_fmt->height = MOCKISP_DEFAULT_HEIGHT;
	sink_fmt->field  = V4L2_FIELD_NONE;
	sink_fmt->code   = MOCKISP_DEFAULT_FMT;
	
	sink_crop = v4l2_subdev_get_try_crop(sd, sd_state, MOCKISP_RESIZE_PAD_SINK);
	sink_crop->width = MOCKISP_DEFAULT_WIDTH;
	sink_crop->height = MOCKISP_DEFAULT_HEIGHT;
	sink_crop->left = 0;
	sink_crop->top = 0;

	src_fmt = v4l2_subdev_get_try_format(sd, sd_state, MOCKISP_RESIZE_PAD_SINK);
	*src_fmt = *sink_fmt;

	return 0;
}

static int mockisp_resize_register(struct mockisp_resize *rsz)
{
	struct v4l2_subdev_state state = {
		.pads = rsz->pad_cfg
	};
	struct media_pad *pads = rsz->pads;
	struct v4l2_subdev *sd = &rsz->sd;
	int ret;

	v4l2_subdev_init(sd, &mockisp_resize_ops);
	sd->flags |= V4L2_SUBDEV_FL_HAS_DEVNODE;
	sd->entity.ops = &mockisp_resize_media_ops;
	sd->entity.function = MEDIA_ENT_F_PROC_VIDEO_SCALER;
	sd->owner = THIS_MODULE;
	strscpy(sd->name, MOCKISP_RESIZE_DEV_NAME, sizeof(sd->name));

	pads[MOCKISP_RESIZE_PAD_SINK].flags = MEDIA_PAD_FL_SINK | MEDIA_PAD_FL_MUST_CONNECT;
	pads[MOCKISP_RESIZE_PAD_SOURCE].flags = MEDIA_PAD_FL_SOURCE | MEDIA_PAD_FL_MUST_CONNECT;

	mutex_init(&rsz->ops_lock);
	
	ret = media_entity_pads_init(&sd->entity, MOCKISP_RESIZE_PADS_NUM, pads);
	if (ret)
		return ret;

	ret = v4l2_device_register_subdev(&rsz->mockisp->v4l2_dev, sd);
	if (ret) {
		dev_err(sd->dev, "Failed to register resize subdev\n");
		goto err_cleanup_media_entity;
	}

	mockisp_resize_init_config(sd, &state);

	return 0;

err_cleanup_media_entity:
	media_entity_cleanup(&sd->entity);

	return ret;
}

int mockisp_resize_dev_register(struct mockisp_device *mockisp)
{
	struct mockisp_resize *rsz = &mockisp->resize_dev;
	
	rsz->mockisp = mockisp;
	return mockisp_resize_register(rsz);
}

void mockisp_resize_dev_unregister(struct mockisp_device *mockisp)
{
	struct mockisp_resize *rsz = &mockisp->resize_dev;

	v4l2_device_unregister_subdev(&rsz->sd);
	media_entity_cleanup(&rsz->sd.entity);
}
