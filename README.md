# KT Cam Bien

Project phục vụ đọc dữ liệu cảm biến và điều khiển các thiết bị ngoại vi.
Hệ thống sử dụng ESP32 DHT11 và cảm biến độ ẩm đất, hiển thị OLED, điều khiển bơm và quạt bằng nút bấm hoặc giao diện web.

## Cấu trúc project

```text
kt_cambien/
├── kt_cambien.ino
├── src/
│   ├── actuators.cpp
│   ├── comm.cpp
│   ├── display.cpp
│   └── sensors.cpp
├── lib/
│   ├── actuators.h
│   ├── comm.h
│   ├── config.h
│   ├── display.h
│   └── sensors.h
├── .gitignore
└── README.md
```

## Chức năng

- Đọc dữ liệu từ cảm biến.
- Điều khiển thiết bị ngoại vi.
- Giao tiếp và trao đổi dữ liệu.
- Hiển thị thông tin qua giao diện hiển thị.

