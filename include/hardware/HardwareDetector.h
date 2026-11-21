
#pragma once

#include <string>
#include <vector>

struct DeviceInfo {
    std::string description;
    std::string hardwareId;
};

struct HardwareReport {
    std::string manufacturer;
    std::string model;

    bool acpiDeviceFound = false;
    bool dchuDriverFound = false;
    bool dchuDllFound = false;

    std::string dllPath;
    std::vector<DeviceInfo> devices;
};

class HardwareDetector {
public:
    static HardwareReport scan();

private:
    static void detectAcpiDevices(HardwareReport& report);
    static void detectDchuDll(HardwareReport& report);
    static void detectSystemInfo(HardwareReport& report);
};
