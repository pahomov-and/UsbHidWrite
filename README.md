# UsbHidWrite

### ./UsbHidWrite -?
Writing data from file to USB:   
> Example: ./UsbHidWrite -v 0x1d50 -p 0x607a -i 0 -e 1 -f Makefile  
> -v - Vendor ID  
> -p - Product ID  
> -i - Interface number  
> -e - Endpoint  
> -f - File  
> -P - Pattern bytes  
> -N - Patterns count  
> -l - Pipe file  
> -W - display width   
> -H - display height  
> -b - Pixel format. Byte on pixel  
> For linux: echo 2702700 > /proc/sys/fs/pipe-max-size  
> For linux: echo 1000 > /sys/module/usbcore/parameters/usbfs_memory_mb  

### Change pipe max size 
> Display width = 1092  
> Display height = 825  
> Display bytes on pixel = 3  
> Total byte 1092 * 825 * 3 = 2702700  
> 
> echo "2702700" > /proc/sys/fs/pipe-max-size   

### Change usb buffer size
> echo 1000 > /sys/module/usbcore/parameters/usbfs_memory_mb

### Add usb rules in udev
> cp 99-fx3_usb_display.rules /etc/udev/rules.d/99-fx3_usb_display.rules

### Example pipe from gstreamer to usb:
> gst-launch-1.0 videotestsrc pattern=13 num-buffers=1 ! video/x-raw,width=1092,height=825,format=RGB ! \
> filesink location=pipe1 | sudo ./UsbHidWrite -v 0x04b4 -p 0x00f1 -i 0 -e 1 -l pipe1 -W 1092 -H 825 -b 3   
