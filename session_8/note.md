# Tài liệu tham khảo chính thức

Bạn nên đọc các tài liệu sau trong quá trình thực hành:

1. Linux Kernel — GPIO Descriptor Consumer Interface  
   Giải thích cách driver lấy và sử dụng GPIO descriptor, cấu hình input/output và đọc/ghi GPIO.  
   ***https://docs.kernel.org/driver-api/gpio/consumer.html***

2. Linux Kernel — GPIO Mappings  
   Giải thích cách ánh xạ GPIO vào thiết bị thông qua Device Tree.  
   ***https://docs.kernel.org/driver-api/gpio/board.html***

3. Linux Kernel — GPIO Character Device Userspace API  
   Giải thích giao diện GPIO dành cho user space. Tài liệu này giúp bạn phân biệt giao diện GPIO trực tiếp từ user space với character device do driver của mình cung cấp.  
   ***https://docs.kernel.org/userspace-api/gpio/chardev.html***

4. Raspberry Pi — The Linux Kernel  
   Hướng dẫn chuẩn bị môi trường, kernel headers và biên dịch kernel trên Raspberry Pi.  
   ***https://www.raspberrypi.com/documentation/computers/linux_kernel.html***

Tài liệu tham khảo:

- Linux Kernel: Platform Devices and Drivers  
***https://docs.kernel.org/driver-api/driver-model/platform.html***
- Raspberry Pi: The Linux Kernel  
***https://www.raspberrypi.com/documentation/computers/linux_kernel.html?utm_source=chatgpt.com***

| Hàm | Chức năng |
|---|---|
| `devm_gpiod_get()` | Lấy GPIO descriptor và tự quản lý vòng đời tài nguyên theo device |
| `gpiod_direction_output()` | Cấu hình GPIO làm đầu ra |
| `gpiod_direction_input()` | Cấu hình GPIO làm đầu vào |
| `gpiod_set_value()` | Thiết lập giá trị logic của GPIO |
| `gpiod_get_value()` | Đọc giá trị logic của GPIO |

Các hàm này được khai báo trong header:

```c
#include <linux/gpio/consumer.h>
```

## gpio_demo.c

| Thành phần | Vai trò |
|---|---|
| `gpio_demo_probe()` | Được gọi khi kernel tìm thấy platform device phù hợp với driver |
| `gpio_demo_remove()` | Được gọi khi thiết bị được gỡ khỏi driver |
| `of_device_id` | Khai báo các chuỗi `compatible` mà driver hỗ trợ |
| `MODULE_DEVICE_TABLE()` | Xuất thông tin nhận diện module cho cơ chế hỗ trợ nạp module |
| `module_platform_driver()` | Tạo cơ chế đăng ký và hủy đăng ký platform driver |
| `MODULE_LICENSE()` | Khai báo giấy phép của module |

## Nạp overlay vào hệ thống đang chạy

Trước tiên, gỡ overlay cũ để tránh nhầm lẫn khi thay đổi cấu hình:

```bash
sudo dtoverlay -r gpio-demo-overlay
```

Biên dịch overlay:

```bash
cd ~/gpio_driver

dtc -@ -I dts -O dtb -o gpio-demo-overlay.dtbo gpio-demo-overlay.dts
```

Raspberry Pi OS hỗ trợ nạp Device Tree Overlay trong lúc hệ thống đang chạy bằng lệnh dtoverlay. Raspberry Pi

Trước tiên, kiểm tra lệnh có sẵn:

```bash
which dtoverlay
```

Nếu có, nạp overlay từ thư mục hiện tại:

```bash
sudo dtoverlay -d "$PWD" gpio-demo-overlay
```

Lệnh trên giả định file nhị phân mang tên gpio-demo-overlay.dtbo đang nằm trong thư mục hiện tại.
Sau đó, kiểm tra thông báo kernel:

```bash
sudo dmesg | tail -n 30
```

Giải thích hai thuộc tính GPIO mới:

```dts
led-gpios = <&gpio 17 0>;
button-gpios = <&gpio 27 1>;
```

Cấu trúc của một GPIO specifier thường là:
> <&gpio GPIO_NUMBER GPIO_FLAGS>

| Thành phần | Ý nghĩa |
|---|---|
| `&gpio` | Tham chiếu GPIO controller trong Device Tree |
| `17` hoặc `27` | Số GPIO trên controller được tham chiếu |
| `0` | `GPIO_ACTIVE_HIGH` |
| `1` | `GPIO_ACTIVE_LOW` |

Hiểu các thành phần cần bổ sung
Để tạo character device, driver cần đăng ký thêm các thành phần sau:

| Thành phần | API | Chức năng |
|---|---|---|
| Device number | `alloc_chrdev_region()` | Cấp phát major/minor number |
| Character device | `cdev_init()` | Liên kết `file_operations` với character device |
| Đăng ký `cdev` | `cdev_add()` | Đưa character device vào kernel |
| Device class | `class_create()` | Tạo class để quản lý device |
| Device node | `device_create()` | Công bố device qua sysfs và tạo sự kiện cho udev |
| Giải phóng | `cdev_del()`, `device_destroy()`... | Dọn dẹp khi driver bị gỡ |