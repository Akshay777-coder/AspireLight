
#include "hardware/HardwareDetector.h"

#include <windows.h>
#include <setupapi.h>

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <vector>

#pragma comment(lib, "Setupapi.lib")

namespace fs = std::filesystem;

static std::string toString(const wchar_t* value) {
    if (!value) return {};

    int size = WideCharToMultiByte(
        CP_UTF8, 0, value, -1, nullptr, 0, nullptr, nullptr
    );

    if (size <= 1) return {};

    std::string result(size - 1, '\0');

    WideCharToMultiByte(
        CP_UTF8, 0, value, -1,
        result.data(), size, nullptr, nullptr
    );

    return result;
}

void HardwareDetector::detectSystemInfo(
    HardwareReport& report
) {
    HKEY key;

    if (RegOpenKeyExW(
        HKEY_LOCAL_MACHINE,
        L"HARDWARE\\DESCRIPTION\\System\\BIOS",
        0,
        KEY_READ,
        &key
    ) != ERROR_SUCCESS) {
        return;
    }

    auto readValue = [&](const wchar_t* name) {
        wchar_t buffer[256]{};
        DWORD size = sizeof(buffer);

        if (RegQueryValueExW(
            key, name, nullptr, nullptr,
            reinterpret_cast<BYTE*>(buffer), &size
        ) == ERROR_SUCCESS) {
            return toString(buffer);
        }

        return std::string{};
    };

    report.manufacturer = readValue(L"SystemManufacturer");
    report.model = readValue(L"SystemProductName");

    RegCloseKey(key);
}

void HardwareDetector::detectAcpiDevices(
    HardwareReport& report
) {
    HDEVINFO devices = SetupDiGetClassDevsW(
        nullptr,
        nullptr,
        nullptr,
        DIGCF_ALLCLASSES | DIGCF_PRESENT
    );

    if (devices == INVALID_HANDLE_VALUE)
        return;

    SP_DEVINFO_DATA deviceData{};
    deviceData.cbSize = sizeof(deviceData);

    for (DWORD i = 0;
         SetupDiEnumDeviceInfo(devices, i, &deviceData);
         ++i) {

        wchar_t description[512]{};
        wchar_t hardwareIds[2048]{};

        SetupDiGetDeviceRegistryPropertyW(
            devices,
            &deviceData,
            SPDRP_DEVICEDESC,
            nullptr,
            reinterpret_cast<PBYTE>(description),
            sizeof(description),
            nullptr
        );

        SetupDiGetDeviceRegistryPropertyW(
            devices,
            &deviceData,
            SPDRP_HARDWAREID,
            nullptr,
            reinterpret_cast<PBYTE>(hardwareIds),
            sizeof(hardwareIds),
            nullptr
        );

        std::string desc = toString(description);
        std::string ids = toString(hardwareIds);

        std::string searchable = ids;
        std::transform(
            searchable.begin(),
            searchable.end(),
            searchable.begin(),
            ::toupper
        );

        // Record only relevant ACPI devices.
        if (searchable.find("ACPI\\") != std::string::npos) {
            report.devices.push_back({desc, ids});
        }

        if (
            searchable.find("CLV0001") != std::string::npos ||
            searchable.find("DCHU") != std::string::npos
        ) {
            report.acpiDeviceFound = true;
        }
    }

    SetupDiDestroyDeviceInfoList(devices);
}

void HardwareDetector::detectDchuDll(
    HardwareReport& report
) {
    const fs::path driverStore =
        L"C:\\Windows\\System32\\DriverStore\\FileRepository";

    std::error_code ec;

    if (!fs::exists(driverStore, ec))
        return;

    for (const auto& entry :
         fs::directory_iterator(driverStore, ec)) {

        if (ec) break;

        if (!entry.is_directory())
            continue;

        std::wstring folder =
            entry.path().filename().wstring();

        if (folder.find(L"acpibridge") ==
            std::wstring::npos) {
            continue;
        }

        fs::path dll = entry.path() / L"InsydeDCHU.dll";

        if (fs::is_regular_file(dll, ec)) {
            report.dchuDllFound = true;
            report.dllPath = dll.string();
            break;
        }
    }

    // Check for the driver file without loading it.
    const fs::path driver =
        L"C:\\Windows\\System32\\drivers\\AcpiBridge.sys";

    if (fs::exists(driver, ec)) {
        report.dchuDriverFound = true;
    }
}

HardwareReport HardwareDetector::scan() {
    HardwareReport report;

    detectSystemInfo(report);
    detectAcpiDevices(report);
    detectDchuDll(report);

    return report;
}
