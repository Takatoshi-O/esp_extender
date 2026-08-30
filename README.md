# esp_extender

`esp_extender` is a SPIKE RT extension component that provides a communication and sensor abstraction layer for an ESP32-connected device. It exposes camera and color-sensor access, command transmission/reception, command dispatch, button input handling, and calibration task entry points.

## Main features

- ESP32 device communication through the SPIKE/PUP device interface.
- Periodic command send and receive processing.
- Sensor-specific command dispatch through registered handlers.
- Camera access for color IDs, YUV values, and 12-position color data.
- Color-sensor access for color IDs, RGBC data, notification colors, and calibration requests.
- Button polling with per-caller short/long press detection.
- Camera and color-sensor calibration task entry points.

## Directory structure

```text
esp_extender/
├── include/
│   ├── lump_calib.h
│   ├── lump_camera.h
│   ├── lump_color.h
│   ├── lump_comm.h
│   ├── lump_comm_tsk.h
│   └── lump_sensors.h
├── src/
│   ├── header/
│   │   ├── lump_button_input.h
│   │   ├── lump_dispatch.h
│   │   └── lump_receive.h
│   │   └── ...
│   └── *.c
├── include/lump_comm.cfg
├── esp_extender.cfg
└── Makefile
```

## Integration

The component is intended to be built as part of a SPIKE RT project. The supplied `Makefile` adds the source directory to the kernel source paths, exports the public include directory, and adds the implementation object files to `SYSSVC_COBJS`.

`include/lump_comm.cfg` creates the communication, dispatch, button, and calibration tasks. Its periodic send and receive cycles are configured for 10 ms activation.

## Public headers

| Header | Purpose |
|---|---|
| `lump_comm.h` | ESP32 device initialization and command transmission |
| `lump_comm_tsk.h` | Task priorities, stack size, and task entry points |
| `lump_receive.h` | Received-command queue interface |
| `lump_dispatch.h` | Command-handler registration |
| `lump_camera.h` | Camera setup and data access |
| `lump_color.h` | Color-sensor setup and data access |
| `lump_sensors.h` | Common color IDs and instance limits |
| `lump_calib.h` | Calibration-mode control |
| `src/header/lump_button_input.h` | Button state and press-event detection |

## Configuration notes

The default communication port and mode are controlled by `ESP32_PORT` and `ESP32_MODE` in `lump_comm.h`. The command payload length is defined by `LUMP_PAYLOAD_LEN`. Task priorities and the default stack size are defined in `lump_comm_tsk.h`.

See the source and configuration files for the exact protocol values and application-specific behavior.
