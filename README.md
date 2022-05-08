# Mock video device driver to understand media-ctl framework

## Caution

This video device driver is NOT FOR ANY PRACTICAL USE.
The driver has no underlying hardware.
The driver does not provide any service via /dev/video, /dev/media.

The driver is only for studying how media-ctl framework works.

The base idea is from Rockchip rkisp1 driver.

## Driver Topology

```
/dev/media3:

+-----------+         +------------------+
+ subdevice | >-----> | node /dev/video0 |
+ Resize    |         | Capture          |
+-----------+         +------------------+
```

## Target SBC

- Raspberry Pi 4B
  - with latest 64bit Raspberry Pi OS (Apr 4th 2022)
  - (CAUTION: not with ubuntu on Raspberry Pi)

## Cross Building

- Install aarch64-linux-gnu-gcc
- Download Kernel Source Code
  ```sh
  $ git clone https://github.com/raspberrypi/linux.git -b rpi-5.15.y --depth 1
  ```
- Rebuild Kernel (is this really needed?)
  - follow official operation manual: https://www.raspberrypi.com/documentation/computers/linux_kernel.html
  - install /boot/kernel8.img, /boot/*.dtb /boot/overlays/*.dtbo
- Build mockisp kernel-module and devicetree overlay
  ```sh
  $ make
  ## to get mockisp.ko and mockisp.dtbo
  ```
- Install files to your Raspberry Pi's storage
  - copy to /home/username/mockisp.ko
  - copy to /boot/overlays/mockisp.dtbo
  - edit /boot/config.txt
    ```
    [all]
    dtoverlay=mockisp
    ```
- Reboot RaspberryPi
- Check if dtbo works
  ```sh
  $ cat /sys/firmware/devicetree/base/mockisp@0/mockisp
  mockisp
  ```
- install module
  ```sh
  $ sudo insmod mockisp.ko
  $ dmesg | tail -10
  mockisp: loading out-of-tree module taints kernel.
  mockisp_probe called
  mockisp: registered mockisp:capture as /dev/video0
  ```

## Check if it works

```all
## /dev/video0 for capture node
$ ls /dev/video0
$ cat cat /sys/class/video4linux/video0/name
mockisp:capture

## V4L2 file interface is not implemented yet
$ sudo v4l2-ctl -d 0 --all
Failed to open /dev/video0: Inappropriate ioctl for device

$ sudo v4l2-ctl --list-devices
bcm2835-codec-decode (platform:bcm2835-codec):
        /dev/video10
        /dev/video11
        /dev/video12
        /dev/video18
        /dev/video31
        /dev/media2

bcm2835-isp (platform:bcm2835-isp):
        /dev/video13
        /dev/video14
        /dev/video15
        /dev/video16
        /dev/video20
        /dev/video21
        /dev/video22
        /dev/video23
        /dev/media0
        /dev/media1

mockisp (platform:mockisp):
        /dev/media3

Failed to open /dev/video0: Inappropriate ioctl for device

## Show detail with media-ctl
$ sudo media-ctl -p -d 3
Media controller API version 5.15.36

Media device information
------------------------
driver          mockisp
model           mockisp
serial
bus info        platform:mockisp
hw revision     0x0
driver version  5.15.36

Device topology
- entity 1: mockisp:capture (1 pad, 0 link)
            type Node subtype V4L flags 0
            device node name /dev/video0
        pad0: Sink

- entity 5: mockisp:resize (2 pads, 0 link)
            type V4L2 subdev subtype Unknown flags 0
        pad0: Sink
        pad1: Source

```
