#include <stdio.h>
#include <usb.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

//sudo gcc -Wall temp.c -lusb -otemp

struct usb_dev_handle * open_port()
{
	struct usb_bus *busses, *bus;

	usb_init();
	usb_find_busses();
	usb_find_devices();

	busses = usb_get_busses();
	for(bus=busses;bus;bus=bus->next)
	{
		struct usb_device *dev;
		for(dev=bus->devices;dev;dev=dev->next)
		{
			if( 0x1235==dev->descriptor.idVendor
			&& 0x4321==dev->descriptor.idProduct)
			{
				printf("Temp device found!\n");
				struct usb_dev_handle *hdev = usb_open(dev);
				if(!hdev)
				{
					perror("Cannot open device");
				}
				else
				{
					usb_detach_kernel_driver_np(hdev, 0);
					usb_set_configuration(hdev, 1);
					if(0!=usb_claim_interface(hdev, 0))
					{
						perror("Cannot claim interface");
						usb_close(hdev);
						hdev = NULL;
					}
				}
				return hdev;
			}
		}
	}
	printf("Temp device not found!\n");
	return NULL;
}

int main(int argc, char *argv[])
{
	int j, i = 0;
	double v;
	struct usb_dev_handle *hdev = open_port();
	if(!hdev)
	{
		return 1;
	}

	unsigned char buf[40];
	usb_clear_halt(hdev,0x81);
	while(1)
	{
		j = usb_bulk_read(hdev, 0x81, (char *)buf, sizeof(buf), 0);
		if(j < 40)
		{
			break;
		}
		if(buf[2]*256+buf[3] != 0x7FFF)
		{
			if(buf[2]*256+buf[3] == 850)
				if(i < 3)
				{
					i++;
					continue;
				}
			v = (buf[2]*256+buf[3]);
			printf("Temp: %.1f\n", v / 10);
		}
	}
	printf("Done!\n");
	return 0;
}
