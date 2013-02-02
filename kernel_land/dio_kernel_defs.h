#ifndef DEF_GUARD_DIO_KERNEL_DEFS_H
#define DEF_GUARD_DIO_KERNEL_DEFS_H

/* 

   Header file whos purpose will be to contain
   definitions relevant to the driver written 
   for ACCESS card PCI-DIO-120.

   Author: Scott Brookes
   Date: created 1.18.13

*/

#include <linux/cdev.h>      /* char device type */
#include <linux/semaphore.h> /* synchronization  */
#include <linux/pci.h>

/* 
   Vendor and Device ID used by the PCI protocol 
   to register the card with the system. These
   are defined in the device's documentation
 */
#define DIO_VENDOR_ID 0x494f
#define DIO_DEVICE_ID 0x0c78

#define DIO_BAR 2       /* Base address region */
/* QQQ why is this 2? 2 works but idk why */
#define DIO_DEV_COUNT 5 /* 120 card has 5 groups */
#define DIO_DEV_SIZE  4 /* ports A,B,C & control */
#define MINOR0        0 /* first minor number */

/* address offsets */
#define DIO_PORTA   0x00
#define DIO_PORTB   0x01
#define DIO_PORTC   0x02
#define DIO_CNTRL   0x03

/* userland command processing masks */
#define SWAP_BYTES(x) ((x << 8) | (x >> 8))
#define GET_PORT(x) ((x & 0x0f00) >> 8)
#define ABGET_DATA(x) (x & 0x00ff)
#define PORTC_WRITE_DATA(x, y) (((x & 0xf000)>>12) ? ((x&0x000f)<<4)|(y&0x000f) : ((x&0x000f)|(y&0x00f0)))
#define PORTC_READ_DATA(x) (((x & 0xf000)>>12) ? ((x&0x00f0)>>4) : (x&0x000f))
/* '--> based on the fact that as defined in the driver, user sends */
/*      x02 for cLO and x12 for cHI so by isolating that MSnibble I */
/*      can apply the appropriate mask.                             */
#define ADD_DATA(r, d) ((r & 0xff00) | (d & 0x00ff))

/* 

   this structure will be used in the driver to package
   all of the relevant data for a device

 */
typedef struct _sdarn_dio_driver_data {

  struct pci_driver *driver;     /* kernel PCI driver data struct */ 
  struct cdev cdev;              /* kernel char driver struct */         
  void __iomem *base;            /* base address */
  unsigned long len;             /* resource length */
  struct file_operations *fops;  /* char device file operations */
  dev_t num;                     /* device number */
  int   group;                   /* group 0-4 */

} dio_dev_data;

/* prototypes for "methods" that will be implemented. */
static int __init dio_dev_init(void);
static void __exit dio_dev_exit(void);

static int dio_probe(struct pci_dev *dev, 
	      const struct pci_device_id *id);
static void dio_remove(struct pci_dev *dev);

static int dio_open(struct inode *inode, struct file *filp);
static int dio_release(struct inode *inode, struct file *filp);
static ssize_t dio_read(struct file *filp, char __user *buf,
			size_t count, loff_t *f_pos);
static ssize_t dio_write(struct file *filp, const char __user *buf,
			 size_t count, loff_t *f_pos);

#endif
