# So sánh và lựa chọn

| Tiêu chí | System Call | IOCTL | procfs | Netlink |
|---|---|---|---|---|
| Mục đích chính | Yêu cầu kernel thực hiện một thao tác | Điều khiển hoặc truy vấn một đối tượng thông qua file descriptor | Truy cập dữ liệu kernel qua filesystem ảo | Trao đổi thông điệp có cấu trúc với subsystem |
| Interface điển hình | `syscall()` | `ioctl()` | `open()`, `read()`, `write()` | `socket()`, `sendmsg()`, `recvmsg()` |
| Cách truyền dữ liệu | Tham số system call | Request code và tham số | Nội dung file ảo | Netlink message |
| Hai chiều | Tùy system call | Có thể đọc và ghi dữ liệu | Có thể đọc và ghi, tùy entry | Hỗ trợ request/reply và notification |
| Ví dụ | `getpid`, `openat` | Điều khiển LED qua driver | Đọc thông tin kernel | Truy vấn địa chỉ IP, route |
| Khi nào chọn? | Khi cần thao tác kernel đã cung cấp hoặc xây dựng system call mới trong trường hợp đặc biệt | Khi cần lệnh điều khiển chuyên biệt cho device hoặc subsystem | Khi cần giao diện file đơn giản cho thông tin hoặc tham số kernel | Khi cần giao thức thông điệp có cấu trúc, đặc biệt cho networking |

Quy tắc lựa chọn:

- System Call: dùng các system call có sẵn trước khi nghĩ đến việc bổ sung system call mới vào kernel.

- IOCTL: phù hợp khi xây dựng device driver cần các lệnh như bật/tắt thiết bị, thiết lập chế độ hoạt động hoặc truy vấn trạng thái.

- procfs: phù hợp với các thông tin và tham số kernel được cung cấp qua filesystem ảo. Với giao diện mới, cần cân nhắc thêm sysfs hoặc các cơ chế khác tùy loại dữ liệu.

- Netlink: ưu tiên khi cần giao tiếp có cấu trúc với Linux networking subsystem hoặc subsystem hỗ trợ Netlink.

**IOCTL** được thiết kế cho các thao tác điều khiển đặc thù, procfs cung cấp giao diện filesystem ảo, còn Netlink hỗ trợ giao tiếp dạng thông điệp và có thể mở rộng bằng các thuộc tính dữ liệu.
