#include "shellassoc.h"
#include <Windows.h>
#include <Shlobj.h>
#include <shlwapi.h>
#include <sddl.h>
#include <wincrypt.h>
#include <cstdint>
#include <string>

#define MD5_HASH_LEN  16
#define TIMESTAMP_LEN 16
#define STRING_BUFFER 1024


static const char REG_URL_IE_ASSOC[]   = "SOFTWARE\\Microsoft\\Internet Explorer\\Capabilities\\UrlAssociations";
static const char REG_EXT_USER_ASSOC[] = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\%s\\UserChoice";
static const char REG_URL_USER_ASSOC[] = "SOFTWARE\\Microsoft\\Windows\\Shell\\Associations\\UrlAssociations\\%s\\UserChoice";
static const char REG_STARTMENU_INET[] = "SOFTWARE\\Clients\\StartMenuInternet";
static const char EDGE_UWP_LEGACY_ID[] = "AppXq0fevzme2pys62n3e0fbqa7peapykr8v";
static const char EDGE_UWP_MODERN_ID[] = "AppX90nv6nhay5n6a98fnetv7tpk64pp35es";
static const char EDGE_UWP_EXE_PATH[]  = "C:\\Windows\\SystemApps\\Microsoft.MicrosoftEdge_8wekyb3d8bbwe\\MicrosoftEdge.exe";

static const uint32_t HASH_V1[] = {
    0x69FB0000, 0x13DB0000, 0x10FA9605, 0x79F8A395,
    0x689B6B9F, 0xEA970001, 0x3C101569, 0x3CE8EC25,
    0x59C3AF2D, 0x2232E0F1, 0x1EC90001, 0x35BD1EC9
};

static const uint32_t HASH_V2[] = {
    0xB1110000, 0x30674EEF, 0x5B9F0000, 0x78F7A461,
    0x12CEB96D, 0x46930000, 0x1D830000, 0x257E1D83,
    0x16F50000, 0x5D8BE90B, 0x96FF0000, 0x2C7C6901,
    0x2B890000, 0x7C932B89, 0x9F690000, 0x405B6097
};

static const uint8_t ASSOC_DESCRIPTOR[] = {
    0x75, 0x73, 0x65, 0x72, 0x20, 0x63, 0x68, 0x6F,
    0x69, 0x63, 0x65, 0x20, 0x73, 0x65, 0x74, 0x20,
    0x76, 0x69, 0x61, 0x20, 0x77, 0x69, 0x6E, 0x64,
    0x6F, 0x77, 0x73, 0x20, 0x75, 0x73, 0x65, 0x72,
    0x20, 0x65, 0x78, 0x70, 0x65, 0x72, 0x69, 0x65,
    0x6E, 0x63, 0x65, 0x20, 0x7B, 0x64, 0x31, 0x38,
    0x62, 0x36, 0x64, 0x64, 0x35, 0x2D, 0x36, 0x31,
    0x32, 0x34, 0x2D, 0x34, 0x33, 0x34, 0x31, 0x2D,
    0x39, 0x33, 0x31, 0x38, 0x2D, 0x38, 0x30, 0x34,
    0x30, 0x30, 0x33, 0x62, 0x61, 0x66, 0x61, 0x30,
    0x62, 0x7D
};

static const char HTA_BEGIN[] = R"(
<!DOCTYPE html><html><head><title>ShellAssoc</title>
<HTA:APPLICATION ID="oHTA"
    APPLICATIONNAME="%s"
    BORDER="none"
    CAPTION="no"
    SHOWINTASKBAR="no"
    SINGLEINSTANCE="yes"
    SYSMENU="no"
    WINDOWSTATE="minimize">
<script language="VBScript">
Sub Window_OnLoad
    Set WshShell = CreateObject("WScript.Shell")
    On Error Resume Next
)";
static const char HTA_WRITE[] = R"(
    WshShell.RegWrite "HKCU\%s\Hash", "%s", "REG_SZ"
    WshShell.RegWrite "HKCU\%s\ProgId", "%s", "REG_SZ"
)";
static const char HTA_DELETE[] = R"(
    WshShell.RegDelete "HKCU\%s\"
)";
static const char HTA_END[] = R"(
    Set WshShell = Nothing
    self.close
End Sub
</script>
</head><body></body></html>
)";

enum WinVer {
    Undef, WinXP, WinVista, Win7, Win8, Win8_1,
    Win10UpTo1607, Win10Above1607, Win11, WinFuture
};

int getWindowsVersion()
{
    static int winVer = Undef;
    if (winVer != Undef)
        return winVer;

    if (HMODULE module = GetModuleHandleA("ntdll")) {
        NTSTATUS(WINAPI *_RtlGetVersion)(LPOSVERSIONINFOEXW);
        *(FARPROC*)&_RtlGetVersion = GetProcAddress(module, "RtlGetVersion");
        OSVERSIONINFOEXW os = {0};
        os.dwOSVersionInfoSize = sizeof(os);
        if (_RtlGetVersion && _RtlGetVersion(&os) == 0) {
            const DWORD major = os.dwMajorVersion;
            const DWORD minor = os.dwMinorVersion;
            const DWORD build = os.dwBuildNumber;
            winVer =
                major == 5L && (minor == 1L || minor == 2L) ? WinXP :
                major == 6L && minor == 0L ? WinVista :
                major == 6L && minor == 1L ? Win7 :
                major == 6L && minor == 2L ? Win8 :
                major == 6L && minor == 3L ? Win8_1 :
                major == 10L && minor == 0L && build <= 14393 ? Win10UpTo1607 :
                major == 10L && minor == 0L && build < 22000 ? Win10Above1607 :
                major == 10L && minor == 0L && build >= 22000 ? Win11 :
                major == 10L && minor > 0L ? WinFuture :
                major > 10L ? WinFuture : Undef;
        }
    }
    return winVer;
}

std::string GetCurrentUserSidString()
{
    static std::string user_sid;
    if (!user_sid.empty())
        return user_sid;
    HANDLE hToken = NULL;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
        return user_sid;
    DWORD tokenLen = 0, tokenInfoLen;
    GetTokenInformation(hToken, TokenUser, NULL, 0, &tokenLen);
    if (GetLastError() != ERROR_INSUFFICIENT_BUFFER || tokenLen == 0) {
        CloseHandle(hToken);
        return user_sid;
    }
    PTOKEN_USER pTokenUser = (PTOKEN_USER)LocalAlloc(LMEM_ZEROINIT, tokenLen);
    if (!pTokenUser) {
        CloseHandle(hToken);
        return user_sid;
    }
    tokenInfoLen = tokenLen;
    if (GetTokenInformation(hToken, TokenUser, pTokenUser, tokenInfoLen, &tokenLen)) {
        LPSTR sidStr = NULL;
        if (ConvertSidToStringSidA(pTokenUser->User.Sid, &sidStr)) {
            user_sid.assign(sidStr);
            LocalFree(sidStr);
        }
    }
    LocalFree(pTokenUser);
    CloseHandle(hToken);
    return user_sid;
}

std::string GetRegistryKeyTimestampHex(LPCSTR lpSubKey)
{
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, lpSubKey, 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
        return "";
    }
    FILETIME ftLastWriteTime;
    if (RegQueryInfoKeyA(hKey, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, &ftLastWriteTime) != ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return "";
    }
    SYSTEMTIME sysTime;
    FileTimeToSystemTime(&ftLastWriteTime, &sysTime);
    sysTime.wMilliseconds = 0;
    sysTime.wSecond = 0;
    SystemTimeToFileTime(&sysTime, &ftLastWriteTime);

    std::string timestamp(TIMESTAMP_LEN, '\0');
    snprintf(&timestamp[0], TIMESTAMP_LEN + 1, "%08x%08x", (uint32_t)ftLastWriteTime.dwHighDateTime, (uint32_t)ftLastWriteTime.dwLowDateTime);
    RegCloseKey(hKey);
    return timestamp;
}

bool FindBrowserCommandByProtocol(HKEY hKey, LPCSTR protocol, LPCSTR progId, DWORD nBufferLen, LPSTR lpBuffer)
{
    HKEY phkResult;
    if (RegOpenKeyExA(hKey, REG_STARTMENU_INET, 0, KEY_READ, &phkResult) != ERROR_SUCCESS)
        return false;

    DWORD cSubKeys = 0;
    if (RegQueryInfoKeyA(phkResult, NULL, NULL, NULL, &cSubKeys, NULL, NULL, NULL, NULL, NULL, NULL, NULL) != ERROR_SUCCESS) {
        RegCloseKey(phkResult);
        return false;
    }

    for (DWORD i = 0; i < cSubKeys; ++i) {
        char pName[MAX_PATH];
        DWORD cchName = _countof(pName);
        if (RegEnumKeyExA(phkResult, i, pName, &cchName, NULL, NULL, NULL, NULL) != ERROR_SUCCESS)
            continue;

        char assocKeyPath[MAX_PATH];
        if (_stricmp(pName, "IEXPLORE.EXE") == 0) {
            strcpy_s(assocKeyPath, REG_URL_IE_ASSOC);
        } else {
            snprintf(assocKeyPath, MAX_PATH, "%s\\%s\\Capabilities\\URLAssociations", REG_STARTMENU_INET, pName);
        }

        HKEY assocKey;
        if (RegOpenKeyExA(hKey, assocKeyPath, 0, KEY_READ, &assocKey) != ERROR_SUCCESS)
            continue;

        BYTE pData[MAX_PATH];
        DWORD cbData = sizeof(pData);
        if (RegQueryValueExA(assocKey, protocol, NULL, NULL, pData, &cbData) == ERROR_SUCCESS) {
            if (_stricmp((const char*)pData, progId) == 0) {
                char cmdKeyPath[MAX_PATH];
                snprintf(cmdKeyPath, MAX_PATH, "%s\\%s\\shell\\open\\command", REG_STARTMENU_INET, pName);
                HKEY cmdKey;
                if (RegOpenKeyExA(hKey, cmdKeyPath, 0, KEY_READ, &cmdKey) == ERROR_SUCCESS) {
                    cbData = sizeof(pData);
                    if (RegQueryValueExA(cmdKey, NULL, NULL, NULL, pData, &cbData) == ERROR_SUCCESS) {
                        const char* command = (const char*)pData;
                        const char* quoted = strchr(command, '"');
                        if (quoted)
                            command = strtok((char*)pData, "\"");
                        strcpy_s(lpBuffer, nBufferLen, command);
                        RegCloseKey(cmdKey);
                        RegCloseKey(assocKey);
                        RegCloseKey(phkResult);
                        return true;
                    }
                    RegCloseKey(cmdKey);
                }
            }
        }
        RegCloseKey(assocKey);
    }
    RegCloseKey(phkResult);
    return false;
}

bool IsPdfOrHttpScheme(const char *str)
{
    static const char *exts[] = {".pdf", "http", "https"};
    for (size_t i = 0; i < sizeof(exts)/sizeof(exts[0]); ++i) {
        if (_stricmp(str, exts[i]) == 0)
            return 1;
    }
    return 0;
}

bool CalcMD5Hash(const void *src, size_t size, uint32_t md5[2])
{
    HCRYPTPROV hProv = 0;
    HCRYPTHASH hHash = 0;
    DWORD hashLen = MD5_HASH_LEN;
    BYTE hash[MD5_HASH_LEN];
    if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
        return false;

    if (!CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash)) {
        CryptReleaseContext(hProv, 0);
        return false;
    }
    if (!CryptHashData(hHash, (const BYTE*)src, (DWORD)size, 0)) {
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        return false;
    }
    if (!CryptGetHashParam(hHash, HP_HASHVAL, hash, &hashLen, 0)) {
        CryptDestroyHash(hHash);
        CryptReleaseContext(hProv, 0);
        return false;
    }
    CryptDestroyHash(hHash);
    CryptReleaseContext(hProv, 0);
    memcpy(md5, hash, MD5_HASH_LEN);
    return true;
}

bool CalcCustomHash_V1(uint32_t *data, uint32_t count, const uint32_t key[2], uint32_t out[2])
{
    if (count < 2 || count % 2 != 0)
        return false;

    count /= 2;
    const uint32_t k0 = (key[0] | 1) + HASH_V1[0];
    const uint32_t k1 = (key[1] | 1) + HASH_V1[1];
    uint32_t res1 = 0;
    uint32_t res2 = 0;
    uint32_t index = 0;

    while (count--) {
        uint32_t r1 = res1 + data[index++];
        uint32_t r2_0 = k0 * r1 - HASH_V1[2] * HIWORD(r1);
        uint32_t r2_1 = HASH_V1[3] * r2_0 + HASH_V1[4] * HIWORD(r2_0);
        uint32_t r3 = HASH_V1[5] * r2_1 - HASH_V1[6] * HIWORD(r2_1);
        uint32_t r4_0 = r3 + data[index++];
        uint32_t r5_0 = k1 * r4_0 - HASH_V1[7] * HIWORD(r4_0);
        uint32_t r5_1 = HASH_V1[8] * r5_0 - HASH_V1[9] * HIWORD(r5_0);
        res1 = HASH_V1[10] * r5_1 + HASH_V1[11] * HIWORD(r5_1);
        res2 += res1 + r3;
    }
    out[0] = res1;
    out[1] = res2;
    return true;
}

bool CalcCustomHash_V2(uint32_t *data, uint32_t count, const uint32_t key[2], uint32_t out[2])
{
    if (count < 2 || count % 2 != 0)
        return false;

    count /= 2;
    const uint32_t k0 = key[0] | 1;
    const uint32_t k1 = key[1] | 1;
    uint32_t res1 = 0;
    uint32_t res2 = 0;
    uint32_t index = 0;

    while (count--) {
        uint32_t r1_0 = k0 * (res1 + data[index++]);
        uint32_t r1_1 = HASH_V2[0] * r1_0 - HASH_V2[1] * HIWORD(r1_0);
        uint32_t r2_0 = HASH_V2[2] * r1_1 - HASH_V2[3] * HIWORD(r1_1);
        uint32_t r2_1 = HASH_V2[4] * HIWORD(r2_0) - HASH_V2[5] * r2_0;
        uint32_t r3 = HASH_V2[6] * r2_1 + HASH_V2[7] * HIWORD(r2_1);
        uint32_t r4_0 = k1 * (r3 + data[index++]);
        uint32_t r4_1 = HASH_V2[8] * r4_0 - HASH_V2[9] * HIWORD(r4_0);
        uint32_t r5_0 = HASH_V2[10] * r4_1 - HASH_V2[11] * HIWORD(r4_1);
        uint32_t r5_1 = HASH_V2[12] * r5_0 + HASH_V2[13] * HIWORD(r5_0);
        res1 = HASH_V2[14] * r5_1 - HASH_V2[15] * HIWORD(r5_1);
        res2 += res1 + r3;
    }
    out[0] = res1;
    out[1] = res2;
    return true;
}

bool Base64Encode(const BYTE *pData, DWORD cbData, std::string &base64Str)
{
    DWORD cchSize = 0;
    if (!CryptBinaryToStringA(pData, cbData, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, NULL, &cchSize))
        return false;
    base64Str.resize(cchSize - 1, '\0');
    return CryptBinaryToStringA(pData, cbData, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, &base64Str[0], &cchSize);
}

bool CreateRegWriteHTA(const char *fileName, const char *regPath, const char *hash, const char *progId)
{
    FILE *f;
    errno_t err = fopen_s(&f, fileName, "w");
    if (err != 0) {
        return false;
    }
    fprintf(f, HTA_BEGIN, "WriteToRegistry");
    fprintf(f, HTA_WRITE, regPath, hash, regPath, progId);
    fputs(HTA_END, f);
    return fclose(f) == 0;
}

bool CreateRegDeleteHTA(const char *fileName, const char *regPath)
{    
    FILE *f;
    errno_t err = fopen_s(&f, fileName, "w");
    if (err != 0) {
        return false;
    }
    fprintf(f, HTA_BEGIN, "DeleteRegistryKey");
    fprintf(f, HTA_DELETE, regPath);
    fputs(HTA_END, f);
    return fclose(f) == 0;
}

bool RunHTAScript(const char *filePath)
{
    char pCmd[MAX_PATH];
    PROCESS_INFORMATION pi;
    STARTUPINFOA si;
    memset(&pi, 0, sizeof(pi));
    memset(&si, 0, sizeof(si));
    si.cb = sizeof(STARTUPINFOA);
    si.dwFlags = STARTF_USESHOWWINDOW;
    snprintf(pCmd, MAX_PATH, "mshta.exe \"%s\"", filePath);
    if (!CreateProcessA(NULL, pCmd, NULL, NULL, 0, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        return false;
    }
    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    return CloseHandle(pi.hThread);
}

bool GenerateFilePathHTA(DWORD nBufferLen, LPSTR lpBuffer)
{
    char tempPath[MAX_PATH];
    DWORD ret = GetTempPathA(MAX_PATH, tempPath);
    if (ret == 0 || ret > MAX_PATH)
        return false;

    UUID uuid;
    RPC_CSTR wszUuid = NULL;
    if (UuidCreate(&uuid) != RPC_S_OK || UuidToStringA(&uuid, &wszUuid) != RPC_S_OK)
        return false;

    const char *uid = (const char*)wszUuid;
    snprintf(lpBuffer, nBufferLen, "%sfta_%s.hta", tempPath, uid);
    RpcStringFreeA(&wszUuid);
    return true;
}

bool RunWriteAssocWithHTA(const char *regPath, const char *hash, const char *progId)
{
    char tempFile[MAX_PATH];
    if (!GenerateFilePathHTA(MAX_PATH, tempFile))
        return false;
    CreateRegWriteHTA(tempFile, regPath, hash, progId);
    RunHTAScript(tempFile);
    return DeleteFileA(tempFile);
}

bool RunDeleteAssocWithHTA(const char *regPath)
{
    char tempFile[MAX_PATH];
    if (!GenerateFilePathHTA(MAX_PATH, tempFile))
        return false;
    CreateRegDeleteHTA(tempFile, regPath);
    RunHTAScript(tempFile);
    return DeleteFileA(tempFile);
}

bool SetAssocWithHash(const char *ext, const char *progID, const char *subkey)
{
    int winVer = getWindowsVersion();
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, subkey, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        RegCloseKey(hKey);
        if (!IsPdfOrHttpScheme(ext))
            goto LABEL_DEL_ORDIN;

        if (winVer > Win8_1) {
            RunDeleteAssocWithHTA(subkey);
            goto LABEL_CREATE;
        }
    LABEL_DEL_ORDIN:
        if (RegDeleteKeyA(HKEY_CURRENT_USER, subkey) != ERROR_SUCCESS) {
            printf("Could not delete registry key %s\n", subkey);
            return false;
        }
    }
LABEL_CREATE:
    if (RegCreateKeyExA(HKEY_CURRENT_USER, subkey, 0, NULL, 0, KEY_WRITE, 0, &hKey, 0) != ERROR_SUCCESS) {
        printf("Could not create registry key %s\n", subkey);
        return false;
    }
    RegCloseKey(hKey);

    if (winVer < Win8) {
        if (RegOpenKeyExA(HKEY_CURRENT_USER, subkey, 0, KEY_SET_VALUE, &hKey) != ERROR_SUCCESS) {
            return false;
        }
        if (RegSetValueExA(hKey, "ProgId", 0, REG_SZ, (const BYTE*)progID, strlen(progID) + 1) != ERROR_SUCCESS) {
            RegCloseKey(hKey);
            return false;
        }
        return RegCloseKey(hKey) == ERROR_SUCCESS;
    }

    char pBrowsrCmd[MAX_PATH] = {0};
    if (_stricmp(ext, "http") == 0 || _stricmp(ext, "https") == 0) {
        if (_stricmp(progID, EDGE_UWP_LEGACY_ID) == 0 || _stricmp(progID, EDGE_UWP_MODERN_ID) == 0) {
            strcpy_s(pBrowsrCmd, _countof(pBrowsrCmd), EDGE_UWP_EXE_PATH);
        } else {
            if (!FindBrowserCommandByProtocol(HKEY_CURRENT_USER, ext, progID, sizeof(pBrowsrCmd), pBrowsrCmd))
                FindBrowserCommandByProtocol(HKEY_LOCAL_MACHINE, ext, progID, sizeof(pBrowsrCmd), pBrowsrCmd);
        }
    }
    char pSource[STRING_BUFFER];
    if (winVer > Win8_1) {
        std::string ts = GetRegistryKeyTimestampHex(subkey);
        std::string sid = GetCurrentUserSidString();
        if (winVer < Win10Above1607) {
            snprintf(pSource, _countof(pSource), "%s%s%s%s%s%s", ext, sid.c_str(), progID, pBrowsrCmd, ts.c_str(), (const char*)ASSOC_DESCRIPTOR);
        } else {
            snprintf(pSource, _countof(pSource), "%s%s%s%s%s", ext, sid.c_str(), progID, ts.c_str(), (const char*)ASSOC_DESCRIPTOR);
        }
    } else
    if (winVer > Win7) {
        std::string sid = GetCurrentUserSidString();
        snprintf(pSource, _countof(pSource), "%s%s%s%s", ext, sid.c_str(), progID, pBrowsrCmd);
    }

    size_t cbDest;
    wchar_t pDest[STRING_BUFFER] = {0};
    mbstowcs_s(&cbDest, pDest, sizeof(pDest)/sizeof(pDest[0]), pSource, strlen(pSource));
    cbDest = (wcslen(pDest) + 1) * sizeof(pDest[0]);
    _wcslwr_s(pDest);

    uint32_t md5Hash[2] = {0};
    CalcMD5Hash(pDest, cbDest, md5Hash);

    unsigned int len = ((cbDest & 4) == 0) + (cbDest >> 2) - 1;
    uint32_t outHash[4] = {0};
    CalcCustomHash_V1((uint32_t*)pDest, len, md5Hash, outHash);
    CalcCustomHash_V2((uint32_t*)pDest, len, md5Hash, outHash + 2);
    uint64_t hashVal1 = *(uint64_t*)(outHash + 2) ^ *(uint64_t*)outHash;
    uint64_t hashVal2 = *(uint64_t*)(outHash + 3) ^ *(uint64_t*)(outHash + 1);
    uint8_t outHashBase[8] = {0};
    memcpy(outHashBase, &hashVal1, sizeof(outHashBase)/2);
    memcpy(outHashBase + 4, &hashVal2, sizeof(outHashBase)/2);

    std::string base64Enc;
    if (!Base64Encode((const BYTE*)outHashBase, sizeof(outHashBase), base64Enc)) {
        return false;
    }

    char resHash[20];
    snprintf(resHash, 20, "%s", base64Enc.c_str());
    if (_stricmp(ext, ".pdf") == 0 || _stricmp(ext, "http") == 0 || _stricmp(ext, "https") == 0) {
        return RunWriteAssocWithHTA(subkey, resHash, progID);

    } else {
        if (RegOpenKeyExA(HKEY_CURRENT_USER, subkey, 0, KEY_SET_VALUE, &hKey) != ERROR_SUCCESS) {
            return false;
        }
        if (RegSetValueExA(hKey, "Hash", 0, REG_SZ, (const BYTE*)resHash, strlen(resHash) + 1) != ERROR_SUCCESS) {
            RegCloseKey(hKey);
            return false;
        }
        if (RegSetValueExA(hKey, "ProgId", 0, REG_SZ, (const BYTE*)progID, strlen(progID) + 1) != ERROR_SUCCESS) {
            RegCloseKey(hKey);
            return false;
        }
        return RegCloseKey(hKey) == ERROR_SUCCESS;
    }
}

bool SetUserFileAssoc(const wchar_t* ext, const wchar_t* progId, bool notifySystem)
{
    if (getWindowsVersion() < WinVista)
        return false;

    char lpExt[MAX_PATH];
    size_t cbDest;
    wcstombs_s(&cbDest, lpExt, sizeof(lpExt), ext, sizeof(lpExt) - 1);

    char lpProgId[MAX_PATH];
    wcstombs_s(&cbDest, lpProgId, sizeof(lpProgId), progId, sizeof(lpProgId) - 1);

    char lpSubkey[MAX_PATH];
    snprintf(lpSubkey, _countof(lpSubkey), lpExt[0] == L'.' ? REG_EXT_USER_ASSOC : REG_URL_USER_ASSOC, lpExt);

    if (!SetAssocWithHash(lpExt, lpProgId, lpSubkey))
        return false;

    if (notifySystem)
        SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
    return true;
}

bool SetUserFileAssoc(const AssocPair* assocArray, size_t count)
{
    for (int i = 0; i < count; ++i) {
        if (!SetUserFileAssoc(assocArray[i].extension, assocArray[i].progId, false))
            return false;
    }
    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
    return true;
}

// bool ResetUserFileAssoc(const wchar_t* ext, bool notifySystem)
// {
//     int winVer = getWindowsVersion();
//     if (winVer < WinVista)
//         return false;

//     char lpExt[MAX_PATH];
//     size_t cbDest;
//     wcstombs_s(&cbDest, lpExt, sizeof(lpExt), ext, sizeof(lpExt) - 1);

//     char lpSubkey[MAX_PATH];
//     snprintf(lpSubkey, _countof(lpSubkey), lpExt[0] == L'.' ? REG_EXT_USER_ASSOC : REG_URL_USER_ASSOC, lpExt);

//     HKEY hKey;
//     if (RegOpenKeyExA(HKEY_CURRENT_USER, lpSubkey, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
//         RegCloseKey(hKey);
//         if (IsPdfOrHttpScheme(lpExt) && winVer > Win8_1) {
//             if (!RunDeleteAssocWithHTA(lpSubkey))
//                 return false;
//         } else {
//             if (RegDeleteKeyA(HKEY_CURRENT_USER, lpSubkey) != ERROR_SUCCESS)
//                 return false;
//         }
//         if (notifySystem)
//             SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
//     }
//     return true;
// }

// bool ResetUserFileAssoc(const wchar_t** extArray, size_t count)
// {
//     for (int i = 0; i < count; ++i) {
//         if (!ResetUserFileAssoc(extArray[i], false))
//             return false;
//     }
//     SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
//     return true;
// }
