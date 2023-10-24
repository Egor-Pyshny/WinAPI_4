#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <iostream>
#include <aclapi.h>

using namespace std;

void SetNewValue(HKEY hKey, const wchar_t* name, const wchar_t* data) {
    if (RegSetValueExW(hKey, name, 0, REG_SZ, (BYTE*)data, sizeof(wchar_t) * (wcslen(data) + 1)) == ERROR_SUCCESS) {
        cout << "Data is recorded" << endl;
    }
    else {
        cout << "Error" << endl;
    }
}


void AddValue(HKEY hKey, const char* name, const char* extra_data) {
    DWORD Tdata = 0;
    DWORD size = 0;
    char* dataBuffer = new char[size];
    if (RegQueryValueExA(hKey, name, NULL, &Tdata, NULL, &size) == ERROR_SUCCESS) {
        if (Tdata == REG_SZ) {
            RegQueryValueExA(hKey, name, NULL, &Tdata, (BYTE*)(dataBuffer), &size);
            char* temp = strcat(dataBuffer, " ");
            const char* newdata = strcat(temp, extra_data);

            int requiredSize = MultiByteToWideChar(CP_ACP, 0, name, -1, NULL, 0);
            wchar_t* wname = new wchar_t[requiredSize];
            MultiByteToWideChar(CP_ACP, 0, name, -1, wname, requiredSize);

            requiredSize = MultiByteToWideChar(CP_ACP, 0, newdata, -1, NULL, 0);
            wchar_t* wdate = new wchar_t[requiredSize];
            MultiByteToWideChar(CP_ACP, 0, newdata, -1, wdate, requiredSize);

            SetNewValue(hKey, wname, wdate);

            cout << "Data updated" << endl;
        }
    }
}


void DeleteKeyByHKEY(HKEY hKey) {
    if (RegDeleteKeyEx(hKey, NULL, KEY_WOW64_64KEY, 0) == ERROR_SUCCESS) {
        cout << "Key deleted" << endl;
    }
    else {
        cout << "Error" << endl;
    }
}


void DeleteKeyByPath(HKEY section, LPCWSTR keyPath) {
    if (RegDeleteKeyEx(section, keyPath, KEY_WOW64_64KEY, 0) == ERROR_SUCCESS) {
        cout << "Key deleted" << endl;
    }
    else {
        cout << "Error" << endl;
    }
}


void FindKey(HKEY section, LPCWSTR keyPath) {
    HKEY hKey;
    if (RegOpenKeyEx(section, keyPath, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        cout << "Key found" << endl;
        RegCloseKey(hKey);
    }
    else {
        cout << "Key not found" << endl;
    }
}


HKEY OpenOrCreateKey(HKEY section,LPCSTR keyPath) {
    HKEY hKey;
    DWORD res;

    if (RegCreateKeyExA(section, keyPath, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE | KEY_READ, NULL, &hKey, &res) == ERROR_SUCCESS) {
        if (res == REG_CREATED_NEW_KEY) {
            cout << "Key created" << endl;
        }
        else if (res == REG_OPENED_EXISTING_KEY) {
            cout << "Key is already created" << endl;
        }
    }
    else {
        cout << "Error" << endl;
    }
    return hKey;
}


void Notifications(HKEY hKey) {
    HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    DWORD filterFlags = REG_NOTIFY_CHANGE_NAME | REG_NOTIFY_CHANGE_LAST_SET;
    DWORD result = RegNotifyChangeKeyValue(hKey, TRUE, filterFlags, hEvent, TRUE);
    while (true) {
        if (result == ERROR_SUCCESS) {
            WaitForSingleObject(hEvent, INFINITE);
            cout << "Changed";
        }
    }

}


DWORD WINAPI MyThreadFunction(LPVOID lpParam) {
    HKEY* myData = reinterpret_cast<HKEY*>(lpParam);
    HKEY hKey = *myData;
    HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    DWORD filterFlags = REG_NOTIFY_CHANGE_NAME | REG_NOTIFY_CHANGE_LAST_SET;
    while (true) {
        DWORD result = RegNotifyChangeKeyValue(hKey, TRUE, filterFlags, hEvent, TRUE);
        if (result == ERROR_SUCCESS) {
            WaitForSingleObject(hEvent, INFINITE);
            cout << "Key was modified";
        }
        Sleep(200);
    }
    return 0;
}


void CheckFlag(HKEY hKey, LONG flag) {
    DWORD psdsize = 1;
    RegGetKeySecurity(hKey, DACL_SECURITY_INFORMATION, NULL, &psdsize);
    PSECURITY_DESCRIPTOR pSecurityDescriptor = LocalAlloc(LMEM_FIXED, psdsize);
    RegGetKeySecurity(hKey, DACL_SECURITY_INFORMATION, pSecurityDescriptor, &psdsize);
    PACL pDacl;
    BOOL bDaclPresent;
    BOOL bDaclDefaulted = FALSE;
    if (GetSecurityDescriptorDacl(pSecurityDescriptor, &bDaclPresent, &pDacl, &bDaclDefaulted)) {
        if (bDaclPresent) {
            for (DWORD i = 0; i < pDacl->AceCount; i++) {
                ACCESS_ALLOWED_ACE* pAce;
                if (GetAce(pDacl, i, reinterpret_cast<void**>(&pAce))) {
                    if ((pAce->Mask & flag) == flag) {
                        cout << "Флаг KEY_READ установлен." << endl;
                    }
                }
            }
        }
    }
}


int main() {
    HKEY hKey = NULL;
    HANDLE hThread = NULL;
    DWORD dwThreadId;
    DWORD psdsize = 1;
    try {
        hKey = OpenOrCreateKey(HKEY_LOCAL_MACHINE, "SOFTWARE\\MyCompany\\MyApp");
        //hThread = CreateThread(NULL, 0, MyThreadFunction, &hKey, 0, &dwThreadId);
       /* if (hThread == NULL) {
            return 1;
        }*/
        //CheckFlag(hKey, KEY_ALL_ACCESS);
        //Notifications(hKey);
        //SetNewValue(hKey, L"Value", L"NewValue1");
        //AddValue(hKey, "Value", "plused");
        //while (true) {
        //}

        CloseHandle(hThread);
        RegCloseKey(hKey);
    }
    catch (...) {
        CloseHandle(hThread);
        RegCloseKey(hKey);
        cout << "Error" << endl;
    }
    return 0;
}
