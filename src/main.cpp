
#include "hardware/HardwareDetector.h"

#include <iostream>
#include <windows.h>

int main() {
    SetConsoleOutputCP(CP_UTF8);

    std::cout << "\n";
    std::cout << "====================================\n";
    std::cout << "          ASPIRE LIGHT\n";
    std::cout << "       Hardware Detection\n";
    std::cout << "====================================\n\n";

    HardwareReport report = HardwareDetector::scan();

    std::cout << "[SYSTEM]\n";
    std::cout << "Manufacturer: " << report.manufacturer << '\n';
    std::cout << "Model:        " << report.model << "\n\n";

    std::cout << "[HARDWARE]\n";
    std::cout << "ACPI device:  "
              << (report.acpiDeviceFound ? "Found" : "Not found")
              << '\n';

    std::cout << "DCHU driver:  "
              << (report.dchuDriverFound ? "Found" : "Not found")
              << '\n';

    std::cout << "DCHU DLL:     "
              << (report.dchuDllFound ? "Found" : "Not found")
              << '\n';

    if (report.dchuDllFound)
        std::cout << "DLL path:     " << report.dllPath << '\n';

    std::cout << "\n[ACPI DEVICES]\n";

    for (const auto& device : report.devices) {
        std::cout << "Device: " << device.description << '\n';
        std::cout << "ID:     " << device.hardwareId << "\n\n";
    }

    std::cout << "====================================\n";
    std::cout << "Detection completed. No hardware writes.\n";

    return 0;
}
