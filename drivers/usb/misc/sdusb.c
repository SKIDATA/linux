/*
 * USB Skeleton driver - 2.0
 *
 * Copyright (C) 2001-2004 Greg Kroah-Hartman (greg@kroah.com)
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License as
 *	published by the Free Software Foundation, version 2.
 *
 * This driver is based on the 2.6.3 version of drivers/usb/usb-skeleton.c 
 * but has been rewritten to be easy to read and use, as no locks are now
 * needed anymore.
 *
 */

/* #include <linux/config.h> */
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/kref.h>
#include <asm/uaccess.h>
#include <linux/usb.h>


/* Define these values to match your devices */

/* table of devices that work with this driver */
static struct usb_device_id sdusb_table [] = {
	{ USB_DEVICE(0x1411, 0x0001) },	/* co460 unlimited */
	{ USB_DEVICE(0x1411, 0x0002) },	/* sd582 schnittstellenprint */
	{ USB_DEVICE(0x1411, 0x0003) },	/* sd704 display */
	{ USB_DEVICE(0x1411, 0x0006) },	/* sd674 turnstile */
	{ USB_DEVICE(0x1411, 0x0007) },	/* sd692 barcode unit */
	{ USB_DEVICE(0x1411, 0x0009) },	/* co460 easy */
	{ USB_DEVICE(0x1411, 0x000E) }, /* sd737 operator diplay */
	{ USB_DEVICE(0x1411, 0x0010) }, /* sd769 neocore freemotion device controller */
	{ USB_DEVICE(0x1411, 0x0011) }, /* sd773 neocore barcode module */
	{ USB_DEVICE(0x1411, 0x0012) }, /* sd776 neocore display module */
	{ USB_DEVICE(0x1411, 0x0014) }, /* sd792 neocore interface board */
	{ USB_DEVICE(0x1411, 0x001E) }, /* sd832 barrier 2010 */
	{ USB_DEVICE(0x1411, 0x0023) }, /* sd849 IO Controller Power.Gate Basic */
	{ USB_DEVICE(0x1411, 0x0027) }, /* sd866 IO Controller Power.Gate Full */
	{ USB_DEVICE(0x1411, 0x0028) }, /* sd871 Plug-In Card Reader */
	{ }					/* Terminating entry */
};
MODULE_DEVICE_TABLE (usb, sdusb_table);


/* Get a minor range for your devices from the usb maintainer */
#define USB_sdusb_MINOR_BASE	192

/* Structure to hold all of our device specific stuff */
struct sdusb {
	struct usb_device *	udev;			/* the usb device for this device */
	struct usb_interface *	interface;		/* the interface for this device */
	__u8			bulk_in_endpointAddr;	/* the address of the bulk in endpoint */
	__u8			bulk_out_endpointAddr;	/* the address of the bulk out endpoint */
	wait_queue_head_t	read_queue;
	struct urb		*rx_urb[8];
	struct kref		kref;
};
#define to_sdusb_dev(d) container_of(d, struct sdusb, kref)

static struct usb_driver sdusb_driver;

static void sdusb_add_urb(struct sdusb *dev, struct urb *urb)
{
	int i;
	for (i=0; i<8; i++){
		if (dev->rx_urb[i] == 0 || dev->rx_urb[i] == urb){
			dev->rx_urb[i] = urb;
			return;
		}
	}
	err("more than 8 rx urbs");
}

static void sdusb_del_urb(struct sdusb *dev, struct urb *urb)
{
	int i;
	for (i=0; i<8; i++){
		if (dev->rx_urb[i] == urb){
			dev->rx_urb[i] = NULL;
			return;
		}
	}
}

static void sdusb_kill_urb(struct sdusb *dev)
{
	int i;
	for (i=0; i<8; i++){
		if (dev->rx_urb[i]);
			usb_kill_urb(dev->rx_urb[i]);
	}
}

static void sdusb_delete(struct kref *kref)
{	
	struct sdusb *dev = to_sdusb_dev(kref);

	/* info("delete"); */
	usb_put_dev(dev->udev);
	kfree (dev);
}

static int sdusb_open(struct inode *inode, struct file *file)
{
	struct sdusb *dev;
	struct usb_interface *interface;
	int subminor;
	int retval = 0;

	subminor = iminor(inode);

	interface = usb_find_interface(&sdusb_driver, subminor);
	if (!interface) {
		err ("%s - error, can't find device for minor %d",
		     __FUNCTION__, subminor);
		retval = -ENODEV;
		goto exit;
	}

	dev = usb_get_intfdata(interface);
	if (!dev) {
		retval = -ENODEV;
		goto exit;
	}

	/* increment our usage count for the device */
	kref_get(&dev->kref);

	/* save our object in the file's private structure */
	file->private_data = dev;

exit:
	return retval;
}

static int sdusb_release(struct inode *inode, struct file *file)
{
	struct sdusb *dev;

	/* info("release"); */
	dev = (struct sdusb *)file->private_data;
	if (dev == NULL)
		return -ENODEV;

	sdusb_kill_urb(dev);

	/* decrement the count on our device */
	kref_put(&dev->kref, sdusb_delete);
	return 0;
}

static void sdusb_read_bulk_callback(struct urb *urb)
{
	struct sdusb *dev;

	dev = (struct sdusb *)urb->context;

	/* sync/async unlink faults aren't errors 
	if (urb->status && 
	    !(urb->status == -ENOENT || 
	      urb->status == -ECONNRESET ||
	      urb->status == -ESHUTDOWN)) {
		dbg("%s - nonzero write bulk status received: %d",
		    __FUNCTION__, urb->status);
		return;
	}*/
	/* info("read status %d count %d",urb->status,urb->actual_length); */
	wake_up_interruptible(&dev->read_queue);
}

static ssize_t sdusb_read(struct file *file, char *buffer, size_t count, loff_t *ppos)
{
	struct sdusb *dev;
	int retval = 0;
	struct urb *urb = NULL;
	char *buf = NULL;

	dev = (struct sdusb *)file->private_data;
	
	/* create a urb, and a buffer for it, and copy the data to the urb */
	urb = usb_alloc_urb(0, GFP_KERNEL);
	if (!urb) {
		retval = -ENOMEM;
		goto error;
	}

	buf = usb_alloc_coherent(dev->udev, count, GFP_KERNEL, &urb->transfer_dma);
	if (!buf) {
		retval = -ENOMEM;
		goto error;
	}

	/* initialize the urb properly */
	usb_fill_bulk_urb(urb, dev->udev,
			  usb_rcvbulkpipe(dev->udev, dev->bulk_in_endpointAddr),
			  buf, count, sdusb_read_bulk_callback, dev);
	
	urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;

	sdusb_add_urb(dev, urb);
	retval = usb_submit_urb(urb, GFP_KERNEL);
	/* info("submit %d\n",retval); */
	
	/* if the read was successful, copy the data to userspace */
	if (retval == 0)
	{
		/* info("before wait %d %d\n",retval,urb->status); */
		retval = wait_event_interruptible(dev->read_queue,urb->status != -EINPROGRESS);
		/* info("after wait %d %d\n",retval,urb->status); */
		if (urb->status ==-EINPROGRESS) /* this urb hangs in the usb-core - kill it */
			usb_kill_urb(urb);

		sdusb_del_urb(dev, urb);
		if (urb->status == 0)
		{
			if (copy_to_user (buffer, buf, urb->actual_length)) {
				retval = -EFAULT;
				goto error;
			}
			retval = urb->actual_length;
		}
		else
		{
			retval = urb->status;
		}
	}
	else
		sdusb_del_urb(dev, urb);
	
error:
	usb_free_coherent(dev->udev, count, buf, urb->transfer_dma);
	usb_free_urb(urb);

	return retval;
}

static void sdusb_write_bulk_callback(struct urb *urb)
{
	struct sdusb *dev;

	dev = (struct sdusb *)urb->context;

	/* sync/async unlink faults aren't errors */
	if (urb->status && 
	    !(urb->status == -ENOENT || 
	      urb->status == -ECONNRESET ||
	      urb->status == -ESHUTDOWN)) {
		dbg("%s - nonzero write bulk status received: %d",
		    __FUNCTION__, urb->status);
	}

	/* free up our allocated buffer */
	usb_free_coherent(urb->dev, urb->transfer_buffer_length, 
			urb->transfer_buffer, urb->transfer_dma);
}

static ssize_t sdusb_write(struct file *file, const char *user_buffer, size_t count, loff_t *ppos)
{
	struct sdusb *dev;
	int retval = 0;
	struct urb *urb = NULL;
	char *buf = NULL;

	dev = (struct sdusb *)file->private_data;

	/* create a urb, and a buffer for it, and copy the data to the urb */
	urb = usb_alloc_urb(0, GFP_KERNEL);
	if (!urb) {
		retval = -ENOMEM;
		goto error;
	}

	buf = usb_alloc_coherent(dev->udev, count, GFP_KERNEL, &urb->transfer_dma);
	if (!buf) {
		retval = -ENOMEM;
		goto error;
	}

	if (copy_from_user(buf, user_buffer, count)) {
		retval = -EFAULT;
		goto error;
	}

	/* initialize the urb properly */
	usb_fill_bulk_urb(urb, dev->udev,
			  usb_sndbulkpipe(dev->udev, dev->bulk_out_endpointAddr),
			  buf, count, sdusb_write_bulk_callback, dev);
	
	urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;
	urb->transfer_flags |= URB_ZERO_PACKET;	
	
	/* send the data out the bulk port */
	retval = usb_submit_urb(urb, GFP_KERNEL);
	if (retval) {
		err("%s - failed submitting write urb, error %d", __FUNCTION__, retval);
		goto error;
	}

	/* release our reference to this urb, the USB core will eventually free it entirely */
	usb_free_urb(urb);

	return count;

error:
	usb_free_coherent(dev->udev, count, buf, urb->transfer_dma);
	usb_free_urb(urb);
	return retval;
}

static struct file_operations sdusb_fops = {
	.owner =	THIS_MODULE,
	.read =		sdusb_read,
	.write =	sdusb_write,
	.open =		sdusb_open,
	.release =	sdusb_release,
};

/* 
 * usb class driver info in order to get a minor number from the usb core,
 * and to have the device registered with devfs and the driver core
 */
static struct usb_class_driver sdusb_class = {
	.name =		"sdusb%d",
	.fops =		&sdusb_fops,
	.minor_base =	USB_sdusb_MINOR_BASE,
};

static int sdusb_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
	struct sdusb *dev = NULL;
	struct usb_host_interface *iface_desc;
	struct usb_endpoint_descriptor *endpoint;
	int i;
	int retval = -ENOMEM;

	/* allocate memory for our device state and initialize it */
	dev = kmalloc(sizeof(*dev), GFP_KERNEL);
	if (dev == NULL) {
		err("Out of memory");
		goto error;
	}
	memset(dev, 0x00, sizeof(*dev));
	kref_init(&dev->kref);

	dev->udev = usb_get_dev(interface_to_usbdev(interface));
	dev->interface = interface;

	iface_desc = interface->cur_altsetting;

	/* current we take the if with 2 ep only 
	if (iface_desc->desc.bNumEndpoints != 2)
	{
		err("interface with %d endpoints ignored",iface_desc->desc.bNumEndpoints);
		goto error;
	}*/
	
	/* set up the endpoint information */
	/* use only the first bulk-in and bulk-out endpoints */
	for (i = 0; i < iface_desc->desc.bNumEndpoints; ++i) {
		endpoint = &iface_desc->endpoint[i].desc;

		if (!dev->bulk_in_endpointAddr &&
		    ((endpoint->bEndpointAddress & USB_ENDPOINT_DIR_MASK)
					== USB_DIR_IN) &&
		    ((endpoint->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK)
					== USB_ENDPOINT_XFER_BULK)) {
			/* we found a bulk in endpoint */
			dev->bulk_in_endpointAddr = endpoint->bEndpointAddress;
		}

		if (!dev->bulk_out_endpointAddr &&
		    ((endpoint->bEndpointAddress & USB_ENDPOINT_DIR_MASK)
					== USB_DIR_OUT) &&
		    ((endpoint->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK)
					== USB_ENDPOINT_XFER_BULK)) {
			/* we found a bulk out endpoint */
			dev->bulk_out_endpointAddr = endpoint->bEndpointAddress;

		}
	}
	/*
	if (!(dev->bulk_in_endpointAddr && dev->bulk_out_endpointAddr)) {
		err("Could not find both bulk-in and bulk-out endpoints");
		goto error;
	}
	*/
	init_waitqueue_head(&dev->read_queue);

	/* save our data pointer in this interface device */
	usb_set_intfdata(interface, dev);

	/* we can register the device now, as it is ready */
	retval = usb_register_dev(interface, &sdusb_class);
	if (retval) {
		/* something prevented us from registering this driver */
		err("Not able to get a minor for this device.");
		usb_set_intfdata(interface, NULL);
		goto error;
	}
	/* increment our usage count for the device */
	kref_get(&dev->kref);
	
	/* let the user know what node this device is now attached to */
	pr_info("sdusb device now attached to sdusb-%d", interface->minor);
	return 0;

error:
	if (dev)
		kref_put(&dev->kref, sdusb_delete);
	return retval;
}

static void sdusb_disconnect(struct usb_interface *interface)
{
	struct sdusb *dev;
	int minor = interface->minor;

	dev = usb_get_intfdata(interface);
	usb_set_intfdata(interface, NULL);

	/* give back our minor */
	usb_deregister_dev(interface, &sdusb_class);

	/* decrement our usage count */
	kref_put(&dev->kref, sdusb_delete);

	pr_info("sdusb #%d now disconnected", minor);
}

static struct usb_driver sdusb_driver = {
	.name =		"sdusb",
	.probe =	sdusb_probe,
	.disconnect =	sdusb_disconnect,
	.id_table =	sdusb_table,
};

static int __init usb_sdusb_init(void)
{
	int result;

	pr_info("sdusb V 5.1.5");
	/* register this driver with the USB subsystem */
	result = usb_register(&sdusb_driver);
	if (result)
		err("usb_register failed. Error number %d", result);
	return result;
}

static void __exit usb_sdusb_exit(void)
{
	/* deregister this driver with the USB subsystem */
	usb_deregister(&sdusb_driver);
}

module_init (usb_sdusb_init);
module_exit (usb_sdusb_exit);

MODULE_LICENSE("GPL");
