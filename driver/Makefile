obj-y := driver.o  
  
KDIR := /usr/src/linux-5.15.10/  
  
PWD := $(shell pwd)  
  
all:  
	make -C $(KDIR) M=$(PWD) modules
