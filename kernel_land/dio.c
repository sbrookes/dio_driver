/*
  
  This is the main file for a driver that will 
  control the ACCESS card PCI-DIO-120
  specifically for the super-darn radar system
  running on a Linux machine.

  Author: Scott Brookes
  Date: created 1.18.13

  Note: Driver developed with close reference to 
  "Linux Device Drivers" Third Edition by
  Corbet, Alessandro, Rubini, and Kroah-Hartman

  ** DESIGN
     This driver registers itself with the linux
     kernel as both a PCI device and a set of 
     "char" devices. The PCI registration allows
     the driver to access vital hardware information
     such as the base address of the device assigned
     at boot time by the system. The 5 char devices
     will be used to address each group of outputs
     on the card individually. This will enable
     an abstraction in user space in which macros
     will allow a user to call open(DIO_RXFE, "r+") 
     and subsequently will not need to concern 
     himself with the hardware path from the card 
     to the RXFE.
   ***********

 */

/* */
/* *** */
/* ******* */
/* ************** */
/* **************************** */
#define DEBUG 0 /* debug option */
/* non-zero to debug else zero  */
/* **************************** */
/* ************** */
/* ******* */
/* *** */
/* */

/*
   ATTN: note that some of these may
   be unneccesary... check later
*/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>     /* error codes */
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/fcntl.h>
#include <linux/ioport.h>    /* mapping io */
#include <linux/kdev_t.h>    /* kernel device type */
#include <linux/cdev.h>      /* char device type */
#include <asm/system.h>
#include <asm/uaccess.h>     /* user access */
#include <linux/semaphore.h> /* synchronization */
#include "dio_kernel_defs.h" /* driver specific header */

/* module information */
MODULE_LICENSE("Dual BSD/GPL"); /* ??? */
MODULE_AUTHOR("Scott Brookes");

/* register initialization and exit routines */
module_init(dio_dev_init);
module_exit(dio_dev_exit);

/* global device data */
dio_dev_data dio_card[DIO_DEV_COUNT];
int          dio_maj_num;

/* pci struct to register with kernel */
/*     so kernel can pair with device */
static struct pci_device_id dio_id[] = {
  { PCI_DEVICE(DIO_VENDOR_ID, DIO_DEVICE_ID) },
  {   /*zero field needed by definition */   }
}; /* and actual registration */
MODULE_DEVICE_TABLE(pci, dio_id);

/* for full PCI driver registration with kernel */
static struct pci_driver dio_driver = {
  .name = "dio",
  .id_table = dio_id,
  .probe = dio_probe,
  .remove = dio_remove,
};

/* register as char device to help read/write */
struct file_operations dio_fops = {
  .owner = THIS_MODULE,
  .read = dio_read,
  .write = dio_write,
  .open = dio_open,
  .release = dio_release,
};

/* 

   INIT - Module Initialization

   called by the system when this module is 
   loaded into the kernel. ( man insmod )

 */
static int __init dio_dev_init(void) {

  int i, j, rc;
  dev_t dev_num;

 #if DEBUG != 0
  printk(KERN_DEBUG "dio_dev_init entry\n");
 #endif

  /* dynamically assign device number */
  rc = alloc_chrdev_region(&dev_num, MINOR0,
			   DIO_DEV_COUNT, dio_driver.name);
  if (rc) {
    printk(KERN_ALERT "Error allocating dev numbers - dio.c\n");
    return rc;
  }
  dio_maj_num = MAJOR(dev_num); /* record major number */

  /* set up individual data for each char dev */
  for ( i = 0; i < DIO_DEV_COUNT; i++ ) {

    dio_card[i].group = i;

    dio_card[i].driver = &dio_driver;
    dio_card[i].fops   = &dio_fops  ;

    /* setup cdev */
    cdev_init(&dio_card[i].cdev, &dio_fops);
    dio_card[i].cdev.owner = THIS_MODULE;
    dio_card[i].cdev.ops = &dio_fops;

    /* register cdev with kernel */
    rc = cdev_add(&dio_card[i].cdev, 
		  MKDEV(dio_maj_num, MINOR0 + i), 1);
    if (rc < 0) {
      printk(KERN_ALERT "Error adding dio cdev %d to sys\n", i);
      goto del_cdev;
    }
  } /* end data initialization for loop */

 #if DEBUG != 0
  printk(KERN_DEBUG "dio_dev_init() exit success\n");
 #endif

  /* finally, register as pci dev */
  /* END FLOW OF SUCCESSFUL OPERATION */
  return pci_register_driver(&dio_driver);

  /* ERROR HANDLING */
 del_cdev:
  /* delete the cdevs that succedded (up to i) */
  for ( j = 0; j < i; j++ )
    cdev_del(&dio_card[j].cdev);

  /* unregister char drivers */
  unregister_chrdev_region(dio_card[0].num, DIO_DEV_COUNT);

  return rc;

} /* end dio_dev_init() function */

/*

  EXIT - module exit routine

  called by the kernel when the module is 
  removed. ( man rmmod )

 */
static void __exit dio_dev_exit(void) {

  int i;

 #if DEBUG != 0
  printk(KERN_DEBUG "dio_dev_exit() entry\n");
 #endif

  /* unregister char drivers */
  unregister_chrdev_region(dio_card[0].num, DIO_DEV_COUNT);

  /* remove char drivers from system */
  for ( i = 0; i < DIO_DEV_COUNT; i++ )
    cdev_del(&dio_card[i].cdev);

  /* unregister PCI driver */
  pci_unregister_driver(&dio_driver);

 #if DEBUG != 0
  printk(KERN_DEBUG "dio_dev_exit() exit normal\n");
 #endif

  return;
} /* end dio_dev_exit() func */

/*

  PROBE - called when device is first assigned to driver
          that is, when the system recognizes that this
	  driver is likely a match to xyz device.

 */
static int dio_probe(struct pci_dev *dev,
		     const struct pci_device_id *id) {

  int rc, i;

 #if DEBUG != 0
  printk(KERN_DEBUG "dio_probe() entry\n");
 #endif

  /* enable the card */
  rc = pci_enable_device(dev);
  if (rc) {
    printk(KERN_ALERT "Failed to enable dio (%d)\n", rc);
    return rc;
  }
 
  /* must claim proprietary access to memory region mapped */
  /*      by the device                                    */
  rc = pci_request_regions(dev, dio_driver.name);
  if (rc) {
    printk(KERN_ALERT "Memory Region Collision for DIO card\n");
    goto request_fail;
  }
  
  /* retrieve base address of mmapped region */
  dio_card[0].len = pci_resource_len(dev, DIO_BAR);
  dio_card[0].base = pci_iomap(dev, DIO_BAR, dio_card[0].len+1);
  if (!dio_card[0].base) {
    printk(KERN_ALERT "Failed to find dio base address\n");
    rc = -ENODEV; /* no device error */
    goto no_base;
  }

  /* set base for other regions */
  for ( i = 0; i < DIO_DEV_COUNT; i++ ) 
    dio_card[i].base = dio_card[0].base + i*DIO_DEV_SIZE ;
  
  /* basic configuration for outputs */
  for ( i = 0; i < DIO_DEV_COUNT; i++ )
    iowrite8(0x80, dio_card[i].base+3);

 #if DEBUG != 0
  printk(KERN_DEBUG "dio_probe() exit success\n");
 #endif

  /* END NATURAL CONTROL FLOW */
  return 0; /* success */

  /* ERROR HANDLING */
 no_base:
  pci_release_regions(dev);

 request_fail:
  pci_disable_device(dev);

  return rc;

} /* end of dio_probe() method */

/*

  REMOVE - Called on removal of driver.. must undo
           all actions taken in probe method

 */
static void dio_remove(struct pci_dev *dev) {

 #if DEBUG != 0
  printk(KERN_DEBUG "dio_remove() entry\n");
 #endif

  pci_iounmap(dev, dio_card[0].base);
  pci_release_regions(dev);
  pci_disable_device(dev);

 #if DEBUG != 0
  printk(KERN_DEBUG "dio_remove() exit success\n");
 #endif

  return;

} /* end of remove method */

/*
  OPEN - called when char device is opened
 */
static int dio_open(struct inode *inode, struct file *filp) {

  dio_dev_data *dev;

 #if DEBUG != 0
  printk(KERN_DEBUG "dio_open() entry\n");
 #endif

  /* need attach the specific device to the filp for */
  /*    other methods to access private data (group) */
  dev = container_of(inode->i_cdev, dio_dev_data, cdev);
  filp->private_data = dev; 
  
 #if DEBUG != 0
  printk(KERN_DEBUG "dio_open() exit success\n");
 #endif

  return 0;

} /* end of open method */

/*

  RELEASE - called when char device is closed

 */
static int dio_release(struct inode *inode, struct file *filp) {

 #if DEBUG != 0
  printk(KERN_DEBUG "dio_release() entry\n");
 #endif

  /* nothing important to do here */

 #if DEBUG != 0
  printk(KERN_DEBUG "dio_release() exit success\n");
 #endif

  return 0;
} /* end of release method */

/*
  READ - used to read from device via char driver

   Note: Char devices have been registered for each group, but 
         not for each port. I will return 2 bytes - the first
	 being the port and the second being the data. Note
	 that I expect the user to place the desired port
	 in the buffer where he wants me to place the data.

   ** IMPT ** calling this from user-land should use the read()
         syscall directly rather than the fread() wrapper because
	 fread() (seems to) change some of the parameters sent
	 (mainly count)...
	            	 
	 port A      = 0x00
	 port B      = 0x01
	 port C(lo)  = 0x02
	 port C(hi)  = 0x12
	 port cntrl  = 0x03
 */
static ssize_t dio_read(struct file *filp, char __user *buf,
			size_t count, loff_t *f_pos) {

  int rc;
  dio_dev_data *dev;
  uint16_t raw;
  uint8_t port, data;

 #if DEBUG != 0
  printk(KERN_DEBUG "dio_read() entry\n");
 #endif

  /* to find the right group */
  dev = filp->private_data;

  /* check arguments */
  if ( count != 2 ) { /* port byte and data byte */
    printk(KERN_ALERT "dio_read() got bad count (%d)", (int)count);
    return -EINVAL;
  }
      
  /* retrieve user data */ 
  rc = copy_from_user(&raw, buf, count);
  if (rc) {
    printk(KERN_ALERT "dio_read() bad copy_from_user\n");
    return -EFAULT;
  }

  /* data comes in backwards */
  raw = SWAP_BYTES(raw);

  /* process the command */
  switch (GET_PORT(raw)) { /* isolate port offset */
  
  case DIO_CNTRL: 
    port = DIO_CNTRL;
    break;
  case DIO_PORTA:
    port = DIO_PORTA;
    break;
  case DIO_PORTB:
    port = DIO_PORTB;
    break;
  case DIO_PORTC:
    port = DIO_PORTC;
    break;
  default:
    return -EINVAL;

  } /* end switch statement */

  /* clear space for and read data */
  raw &= 0xff00;
  raw |= ioread8(dev->base + port);

  /* isolate data to be returned */
  if (DIO_PORTC == port) {
    data = PORTC_READ_DATA(raw);
    raw = ADD_DATA(raw, data);  
  }

  /* return data (it will reverse it again) */
  raw = SWAP_BYTES(raw);
  rc = copy_to_user(buf, &raw, 2);
  if (rc) {
    printk(KERN_ALERT "dio_read() bad copy_to_user\n");
    return -EFAULT;
  }

 #if DEBUG != 0
  printk(KERN_DEBUG "dio_read() exit success\n");
 #endif

  return count;
} /* end of read method */


/*

  WRITE - used to write to device via char driver

   Note: Char devices have been registered for each group, but 
         not for each port. I will accept 2 bytes of input to 
	 the write function with the first being the offset
	 necessary to address the requested port.

	 port A      = 0x00{data}
	 port B      = 0x01{data}
	 port C(lo)  = 0x02{data}
	 port C(hi)  = 0x12{data}
	 port cntrl  = 0x03{data}

*/
static ssize_t dio_write(struct file *filp, const char __user *buf,
			 size_t count, loff_t *f_pos) {

  int rc;
  dio_dev_data *dev;
  uint16_t raw;
  uint8_t port, data, temp;

 #if DEBUG != 0
  printk(KERN_DEBUG "dio_write() entry\n");
 #endif

  /* to find the right group */
  dev = filp->private_data;

  /* check arguments */
  if ( count != 2 ) { /* port byte and data byte */
    printk(KERN_ALERT "dio_write() got bad count (%d)\n", (int)count);
    return -EINVAL;
   }

  /* retrieve user data */ 
  rc = copy_from_user(&raw, buf, count);
  if (rc) {
    printk(KERN_ALERT "dio_write() bad copy_from_user\n");
    return -EFAULT;
  }

  /* data comes in backwards */
  raw = SWAP_BYTES(raw);

  /* process the command */
  switch (GET_PORT(raw)) { /* isolate port offset */
  
  case DIO_CNTRL: 
    port = DIO_CNTRL;
    break;
  case DIO_PORTA:
    port = DIO_PORTA;
    break;
  case DIO_PORTB:
    port = DIO_PORTB;
    break;
  case DIO_PORTC:
    port = DIO_PORTC;
    break;
  default:
    return -EINVAL;
  
  } /* end switch statement */

  /* isolate data to be written */
  if (DIO_PORTC == port) {
    /* retrieve existing portC data */
    temp = ioread8(dev->base + port);
    data = PORTC_WRITE_DATA(raw, temp);
  }
  else /* data for A, B, or CNTRL */
    data = ABGET_DATA(raw);

  /* now write the data */
  iowrite8(data, dev->base + port);

 #if DEBUG != 0
  printk(KERN_DEBUG "dio_write exit success\n");
 #endif

  return count;
 
} /* end of write method */


