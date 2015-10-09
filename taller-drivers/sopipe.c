#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h> /* for memory management */
#include <asm/uaccess.h> /* for accessing user-space memory */
#include <linux/semaphore.h> /* for synchronization */
#include <linux/errno.h>

#include "sopipe.h"

MODULE_LICENSE("GPL");

static struct cdev dev;

// Should be a struct
static struct semaphore s_hay_datos;
static spinlock_t m_data_buffer;
static unsigned char * data;
static ssize_t data_size = 0;
static loff_t readptr = 0;
static loff_t writeptr = 0;

static dev_t devno = MKDEV(0,0);

static __init int sopipe_init(void) {
  int err;

  cdev_init(&dev, &sopipe_fops);
  dev.owner = THIS_MODULE;
  dev.ops = &sopipe_fops;

  // Inicializo semáforo, spinlock y buffer
  sema_init(&s_hay_datos, 0);
  spin_lock_init(&m_data_buffer);
  data_size = 32;
  data = vmalloc(data_size);

  /* dynamically get a MAJOR and MINOR */
  err = alloc_chrdev_region(&devno, 0, 1, "sopipe");
  if (err) {
    printk(KERN_NOTICE "Error %d allocating major and minor numbers for sopipe\n", err);
    goto out_noregion;
  }
  printk(KERN_ALERT "SOPIPE MODULE MAJOR = %d | MINOR = %d\n", MAJOR(devno), MINOR(devno));

  err = cdev_add(&dev, devno, 1);
  /* Fail gracefully if need be */
  if (err) {
    printk(KERN_NOTICE "Error %d adding sopipe\n", err);
    goto out_noadd;
  }
  return 0;

out_noadd:
  unregister_chrdev_region(devno, 1);
out_noregion:
  // Si algo salió mal, devuelvo la memoria que me dieron
  return -EIO;
}

static __exit void sopipe_exit(void) {
  cdev_del(&dev);
  unregister_chrdev_region(devno, 1);
  // Libero la memoria del módulo
  vfree(data);
}

int sopipe_open(struct inode * inode, struct file * filp) {
  return 0;
}

int sopipe_release(struct inode * inode, struct file * filp) {
  return 0;
}

ssize_t sopipe_read(struct file * filp, char __user *buff, size_t count, loff_t * offp) {
    // printk(KERN_ALERT "Están leyendo %d bytes\n", count);
    if (count == 0) {
        return 0;
    }
    ssize_t to_read = 1;
    down(&s_hay_datos);
    spin_lock(&m_data_buffer);
    if (writeptr - readptr < to_read) {
        // la cantidad a leer es mayor de lo que hay escrito
        // printk(KERN_ALERT "la cantidad a leer es mayor de lo que hay escrito\n");
        to_read = writeptr - readptr;
    }
    if (to_read == 0) {
        return 0;
    }
    copy_to_user(buff, data+readptr, to_read);
    readptr += to_read;
    spin_unlock(&m_data_buffer);
    return to_read;
}

ssize_t sopipe_write(struct file * filp, const char __user *buff, size_t count, loff_t * offp) {
    spin_lock(&m_data_buffer);
    if (data_size - writeptr < count) {
        // no hay espacio para escribir lo que me piden:
        printk(KERN_ALERT "no hay espacio para escribir lo que me piden\n");
        // 1) Pido un buffer nuevo más grande que el que tengo
        ssize_t new_size = data_size*2;
        char * aux = vmalloc(new_size);
        // 2) Copio el buffer viejo al buffer nuevo
        int i;
        for (i = 0; i < data_size; i++) {
            aux[i] = data[i];
        }
        // 3) Libero el buffer viejo
        vfree(data);
        // 4) Seteo el nuevo buffer como buffer del módulo
        data = aux;
        data_size = new_size;
    }
    spin_unlock(&m_data_buffer);
    // hay espacio para escribir
    spin_lock(&m_data_buffer);
    copy_from_user(data+writeptr, buff, count);
    writeptr += count;
    spin_unlock(&m_data_buffer);
    up(&s_hay_datos);
    return count;
}

module_init(sopipe_init);
module_exit(sopipe_exit);
