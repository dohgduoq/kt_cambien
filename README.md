# Kỹ thuật cảm biến

Hệ thống ESP32 sử dụng DHT11 và cảm biến độ ẩm đất, hiển thị OLED, điều khiển bơm và quạt bằng nút bấm hoặc giao diện web.

Mở `kt_cambien.ino` bằng Arduino IDE, chọn bo ESP32. Cài thư viện DHT sensor library, Adafruit Unified Sensor, Adafruit GFX Library và Adafruit SSD1306.

Giữ cấu trúc thư mục `src` và `lib`. GPIO và các ngưỡng đặt trong `lib/config.h`. Điền tên và mật khẩu Wi-Fi nếu cần kết nối mạng có sẵn. Có thể kết nối Wi-Fi `esp32` để mở trang web tại http://192.168.4.1.

Serial Monitor dùng tốc độ 115200 baud, hiển thị nhiệt độ, độ ẩm không khí, độ ẩm đất và ADC mỗi 2 giây. Chương trình giữ các tác vụ FreeRTOS và logic điều khiển của bản gốc.
