/*
 * Velora Language — Windows Installer/Runtime
 * ==============================================
 * Single self-contained executable that:
 *   1. Requests administrator rights (UAC manifest)
 *   2. Installs Velora to C:\Program Files\Velora\
 *   3. Registers .vel file extension in Windows
 *   4. Adds 'velora' command to system PATH
 *   5. Installs itself as a background tray app
 *   6. Adds startup registry entry (auto-starts with Windows)
 *   7. Provides: velora run, velora build, velora check, velora new
 *
 * Compile with:
 *   g++ velora_installer.cpp -o VeloraSetup.exe -mwindows -lshlwapi -lshell32
 *   windres velora_installer.rc velora_installer.res
 *   g++ velora_installer.cpp velora_installer.res -o VeloraSetup.exe -mwindows -lshlwapi -lshell32
 */

#define WIN32_LEAN_AND_MEAN
#define UNICODE
#define _UNICODE

#include <windows.h>
#include <shlwapi.h>
#include <shellapi.h>
#include <shlobj.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "shell32.lib")

// ─── Constants ───

#define VELORA_VERSION      L"0.1.0"
#define VELORA_APP_NAME     L"Velora"
#define VELORA_INSTALL_DIR  L"C:\\Program Files\\Velora"
#define VELORA_EXE_NAME     L"velora.exe"
#define VELORA_TRAY_NAME    L"VeloraTray"
#define WM_TRAY_ICON        (WM_USER + 1)
#define IDI_TRAY_ICON       1
#define ID_TRAY_OPEN        1001
#define ID_TRAY_UNINSTALL   1002
#define ID_TRAY_EXIT        1003

// ─── Global State ───

HWND g_tray_hwnd = NULL;
NOTIFYICONDATAW g_nid = {};
bool g_installed_mode = false;

// ─── Utility: Wide string helpers ───

std::wstring exe_path() {
    wchar_t buf[MAX_PATH] = {};
    GetModuleFileNameW(NULL, buf, MAX_PATH);
    return buf;
}

std::wstring exe_dir() {
    std::wstring path = exe_path();
    size_t pos = path.rfind(L'\\');
    return (pos != std::wstring::npos) ? path.substr(0, pos) : L".";
}

std::wstring install_dir() {
    return std::wstring(VELORA_INSTALL_DIR);
}

std::wstring installed_exe() {
    return install_dir() + L"\\" + VELORA_EXE_NAME;
}

// ─── Registry Helpers ───

bool reg_set_sz(HKEY root, const std::wstring& key_path,
                const std::wstring& value_name, const std::wstring& value) {
    HKEY hk;
    LONG res = RegCreateKeyExW(root, key_path.c_str(), 0, NULL,
                                REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hk, NULL);
    if (res != ERROR_SUCCESS) return false;
    res = RegSetValueExW(hk, value_name.empty() ? NULL : value_name.c_str(),
                         0, REG_SZ, (BYTE*)value.c_str(),
                         (DWORD)((value.size() + 1) * sizeof(wchar_t)));
    RegCloseKey(hk);
    return res == ERROR_SUCCESS;
}

bool reg_delete_key(HKEY root, const std::wstring& key_path) {
    return RegDeleteTreeW(root, key_path.c_str()) == ERROR_SUCCESS;
}

std::wstring reg_get_sz(HKEY root, const std::wstring& key_path,
                         const std::wstring& value_name) {
    HKEY hk;
    if (RegOpenKeyExW(root, key_path.c_str(), 0, KEY_READ, &hk) != ERROR_SUCCESS)
        return L"";
    wchar_t buf[32768] = {};
    DWORD size = sizeof(buf);
    DWORD type = 0;
    RegQueryValueExW(hk, value_name.empty() ? NULL : value_name.c_str(),
                     NULL, &type, (BYTE*)buf, &size);
    RegCloseKey(hk);
    return buf;
}

// ─── PATH Management ───

bool add_to_system_path(const std::wstring& dir) {
    const std::wstring key = L"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment";
    std::wstring current = reg_get_sz(HKEY_LOCAL_MACHINE, key, L"Path");

    // Check if already in PATH
    std::wstring lower_cur = current;
    std::wstring lower_dir = dir;
    std::transform(lower_cur.begin(), lower_cur.end(), lower_cur.begin(), ::towlower);
    std::transform(lower_dir.begin(), lower_dir.end(), lower_dir.begin(), ::towlower);
    if (lower_cur.find(lower_dir) != std::wstring::npos) return true;

    // Append
    if (!current.empty() && current.back() != L';') current += L';';
    current += dir;

    bool ok = reg_set_sz(HKEY_LOCAL_MACHINE, key, L"Path", current);
    if (ok) {
        // Broadcast the change so open terminals pick it up
        SendMessageTimeoutW(HWND_BROADCAST, WM_SETTINGCHANGE, 0,
                            (LPARAM)L"Environment", SMTO_ABORTIFHUNG, 2000, NULL);
    }
    return ok;
}

bool remove_from_system_path(const std::wstring& dir) {
    const std::wstring key = L"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment";
    std::wstring current = reg_get_sz(HKEY_LOCAL_MACHINE, key, L"Path");

    // Remove the dir
    std::wstring search = dir + L";";
    size_t pos = current.find(search);
    if (pos == std::wstring::npos) {
        search = dir;
        pos = current.find(search);
    }
    if (pos != std::wstring::npos) {
        current.erase(pos, search.size());
    }

    return reg_set_sz(HKEY_LOCAL_MACHINE, key, L"Path", current);
}

// ─── File Extension Registration ───

bool register_vel_extension() {
    std::wstring velora_exe = installed_exe();

    // .vel → VeloraSourceFile
    reg_set_sz(HKEY_CLASSES_ROOT, L".vel", L"", L"VeloraSourceFile");
    reg_set_sz(HKEY_CLASSES_ROOT, L".vel", L"Content Type", L"text/x-velora");
    reg_set_sz(HKEY_CLASSES_ROOT, L"VeloraSourceFile", L"",
               L"Velora Source File");
    reg_set_sz(HKEY_CLASSES_ROOT, L"VeloraSourceFile\\DefaultIcon", L"",
               velora_exe + L",0");

    // Open command
    reg_set_sz(HKEY_CLASSES_ROOT,
               L"VeloraSourceFile\\shell\\open\\command", L"",
               L"\"" + velora_exe + L"\" run \"%1\"");

    // Run in terminal
    reg_set_sz(HKEY_CLASSES_ROOT,
               L"VeloraSourceFile\\shell\\run", L"",
               L"Run with Velora");
    reg_set_sz(HKEY_CLASSES_ROOT,
               L"VeloraSourceFile\\shell\\run\\command", L"",
               L"cmd.exe /k \"\"" + velora_exe + L"\" run \"%1\"\"");

    // Build
    reg_set_sz(HKEY_CLASSES_ROOT,
               L"VeloraSourceFile\\shell\\build", L"",
               L"Build with Velora");
    reg_set_sz(HKEY_CLASSES_ROOT,
               L"VeloraSourceFile\\shell\\build\\command", L"",
               L"cmd.exe /k \"\"" + velora_exe + L"\" build \"%1\"\"");

    // Check
    reg_set_sz(HKEY_CLASSES_ROOT,
               L"VeloraSourceFile\\shell\\check", L"",
               L"Check Velora Syntax");
    reg_set_sz(HKEY_CLASSES_ROOT,
               L"VeloraSourceFile\\shell\\check\\command", L"",
               L"cmd.exe /k \"\"" + velora_exe + L"\" check \"%1\"\"");

    // Refresh shell
    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
    return true;
}

bool unregister_vel_extension() {
    reg_delete_key(HKEY_CLASSES_ROOT, L".vel");
    reg_delete_key(HKEY_CLASSES_ROOT, L"VeloraSourceFile");
    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
    return true;
}

// ─── App Registration (Add/Remove Programs) ───

bool register_uninstall_entry() {
    const std::wstring key = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\VeloraLang";
    std::wstring velora_exe = installed_exe();

    reg_set_sz(HKEY_LOCAL_MACHINE, key, L"DisplayName", L"Velora Programming Language");
    reg_set_sz(HKEY_LOCAL_MACHINE, key, L"DisplayVersion", VELORA_VERSION);
    reg_set_sz(HKEY_LOCAL_MACHINE, key, L"Publisher", L"Velora Contributors");
    reg_set_sz(HKEY_LOCAL_MACHINE, key, L"InstallLocation", install_dir());
    reg_set_sz(HKEY_LOCAL_MACHINE, key, L"UninstallString",
               L"\"" + velora_exe + L"\" --uninstall");
    reg_set_sz(HKEY_LOCAL_MACHINE, key, L"DisplayIcon", velora_exe + L",0");
    reg_set_sz(HKEY_LOCAL_MACHINE, key, L"URLInfoAbout", L"https://velora.dev");
    reg_set_sz(HKEY_LOCAL_MACHINE, key, L"NoModify", L"1");
    return true;
}

// ─── Startup Entry ───

bool add_startup_entry() {
    const std::wstring key = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";
    std::wstring cmd = L"\"" + installed_exe() + L"\" --tray";
    return reg_set_sz(HKEY_LOCAL_MACHINE, key, L"VeloraLang", cmd);
}

bool remove_startup_entry() {
    HKEY hk;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                      L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
                      0, KEY_WRITE, &hk) != ERROR_SUCCESS) return false;
    RegDeleteValueW(hk, L"VeloraLang");
    RegCloseKey(hk);
    return true;
}

// ─── Install / Uninstall ───

void ensure_dir(const std::wstring& path) {
    SHCreateDirectoryExW(NULL, path.c_str(), NULL);
}

bool do_install() {
    std::wstring src = exe_path();
    std::wstring dst = installed_exe();
    std::wstring dir = install_dir();

    ensure_dir(dir);
    ensure_dir(dir + L"\\examples");
    ensure_dir(dir + L"\\stdlib");

    // Copy this EXE as velora.exe into Program Files
    if (!CopyFileW(src.c_str(), dst.c_str(), FALSE)) {
        return false;
    }

    // Write a small velora.cmd wrapper so both `velora` and `velora.exe` work
    std::wstring cmd_path = dir + L"\\velora.cmd";
    std::string cmd_content = "@echo off\r\n\"" ;
    std::string dst_a(dst.begin(), dst.end());
    cmd_content += dst_a + "\" %*\r\n";
    std::ofstream cmd_file(cmd_path.c_str(), std::ios::binary);
    if (cmd_file.is_open()) {
        cmd_file << cmd_content;
        cmd_file.close();
    }

    // Write a hello world example
    std::wstring ex_path = dir + L"\\examples\\hello.vel";
    std::string ex_content = "// Hello World — Velora\r\n\r\nmain:\r\n    print(\"Hello from Velora!\")\r\n";
    std::ofstream ex_file(ex_path.c_str(), std::ios::binary);
    if (ex_file.is_open()) { ex_file << ex_content; ex_file.close(); }

    add_to_system_path(dir);
    register_vel_extension();
    register_uninstall_entry();
    add_startup_entry();

    return true;
}

bool do_uninstall() {
    remove_from_system_path(install_dir());
    unregister_vel_extension();
    reg_delete_key(HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\VeloraLang");
    remove_startup_entry();

    // Schedule deletion of the install dir after reboot
    wchar_t empty[] = L"";
    MoveFileExW(install_dir().c_str(), NULL, MOVEFILE_DELAY_UNTIL_REBOOT);

    return true;
}

// ─── System Tray App ───

HMENU create_tray_menu() {
    HMENU menu = CreatePopupMenu();
    InsertMenuW(menu, 0, MF_BYPOSITION | MF_STRING, ID_TRAY_OPEN,
                L"Open Velora Folder");
    InsertMenuW(menu, 1, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
    InsertMenuW(menu, 2, MF_BYPOSITION | MF_STRING, ID_TRAY_UNINSTALL,
                L"Uninstall Velora");
    InsertMenuW(menu, 3, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
    InsertMenuW(menu, 4, MF_BYPOSITION | MF_STRING, ID_TRAY_EXIT,
                L"Exit Tray (Velora stays installed)");
    return menu;
}

void show_tray_balloon(const wchar_t* title, const wchar_t* msg) {
    g_nid.uFlags = NIF_INFO;
    g_nid.dwInfoFlags = NIIF_INFO;
    wcscpy_s(g_nid.szInfoTitle, title);
    wcscpy_s(g_nid.szInfo, msg);
    g_nid.uTimeout = 3000;
    Shell_NotifyIconW(NIM_MODIFY, &g_nid);
}

LRESULT CALLBACK tray_wnd_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_TRAY_ICON:
            if (lp == WM_RBUTTONUP || lp == WM_LBUTTONUP) {
                POINT pt;
                GetCursorPos(&pt);
                SetForegroundWindow(hwnd);
                HMENU menu = create_tray_menu();
                TrackPopupMenu(menu, TPM_RIGHTALIGN | TPM_BOTTOMALIGN,
                               pt.x, pt.y, 0, hwnd, NULL);
                DestroyMenu(menu);
            }
            return 0;

        case WM_COMMAND:
            switch (LOWORD(wp)) {
                case ID_TRAY_OPEN:
                    ShellExecuteW(NULL, L"explore", install_dir().c_str(), NULL, NULL, SW_SHOW);
                    break;
                case ID_TRAY_UNINSTALL:
                    if (MessageBoxW(NULL,
                        L"Are you sure you want to uninstall Velora?",
                        L"Velora Uninstall", MB_YESNO | MB_ICONQUESTION) == IDYES) {
                        Shell_NotifyIconW(NIM_DELETE, &g_nid);
                        do_uninstall();
                        MessageBoxW(NULL,
                            L"Velora has been uninstalled.\nThe folder will be removed on next restart.",
                            L"Velora Uninstalled", MB_OK | MB_ICONINFORMATION);
                        PostQuitMessage(0);
                    }
                    break;
                case ID_TRAY_EXIT:
                    Shell_NotifyIconW(NIM_DELETE, &g_nid);
                    PostQuitMessage(0);
                    break;
            }
            return 0;

        case WM_DESTROY:
            Shell_NotifyIconW(NIM_DELETE, &g_nid);
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

int run_tray_mode() {
    WNDCLASSW wc = {};
    wc.lpfnWndProc = tray_wnd_proc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = VELORA_TRAY_NAME;
    RegisterClassW(&wc);

    g_tray_hwnd = CreateWindowW(VELORA_TRAY_NAME, L"Velora Tray",
                                 0, 0, 0, 0, 0, HWND_MESSAGE, NULL,
                                 GetModuleHandle(NULL), NULL);

    // Add system tray icon
    g_nid.cbSize = sizeof(g_nid);
    g_nid.hWnd = g_tray_hwnd;
    g_nid.uID = 1;
    g_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    g_nid.uCallbackMessage = WM_TRAY_ICON;
    g_nid.hIcon = LoadIconW(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_TRAY_ICON));
    if (!g_nid.hIcon) g_nid.hIcon = LoadIconW(NULL, IDI_APPLICATION);
    wcscpy_s(g_nid.szTip, L"Velora Programming Language");
    Shell_NotifyIconW(NIM_ADD, &g_nid);

    // Show balloon on start
    show_tray_balloon(L"Velora is running",
                      L"Type 'velora run main.vel' in any terminal.");

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return 0;
}

// ─── Install UI ───

void show_install_ui() {
    int choice = MessageBoxW(NULL,
        L"Velora Programming Language v0.1.0\n\n"
        L"This will install Velora on your system:\n\n"
        L"  \u2022 Installs to: C:\\Program Files\\Velora\\\n"
        L"  \u2022 Adds 'velora' command to system PATH\n"
        L"  \u2022 Registers .vel file extension\n"
        L"  \u2022 Adds 'Run/Build/Check' to right-click on .vel files\n"
        L"  \u2022 Starts background tray helper on Windows startup\n\n"
        L"Click OK to install, Cancel to exit.",
        L"Velora Language Installer",
        MB_OKCANCEL | MB_ICONINFORMATION);

    if (choice != IDOK) {
        return;
    }

    // Show progress
    HWND progress_hwnd = CreateWindowExW(0, L"STATIC", NULL,
        WS_POPUP | WS_VISIBLE | SS_CENTER,
        500, 400, 300, 60, NULL, NULL, GetModuleHandle(NULL), NULL);

    SetWindowTextW(progress_hwnd, L"Installing...");
    UpdateWindow(progress_hwnd);

    bool ok = do_install();
    DestroyWindow(progress_hwnd);

    if (ok) {
        MessageBoxW(NULL,
            L"Velora installed successfully!\n\n"
            L"You can now:\n\n"
            L"  Open a new terminal and type:\n"
            L"    velora run main.vel\n"
            L"    velora build main.vel\n"
            L"    velora check main.vel\n"
            L"    velora new my_project\n"
            L"    velora version\n\n"
            L"  Right-click any .vel file to run or build it.\n\n"
            L"Velora will start automatically on next Windows startup.\n"
            L"Starting tray helper now...",
            L"Velora Installed!",
            MB_OK | MB_ICONINFORMATION);

        // Launch tray helper (this exe with --tray)
        std::wstring cmd = L"\"" + installed_exe() + L"\" --tray";
        STARTUPINFOW si = {sizeof(si)};
        PROCESS_INFORMATION pi = {};
        CreateProcessW(NULL, (LPWSTR)cmd.c_str(), NULL, NULL, FALSE,
                       CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
        if (pi.hProcess) CloseHandle(pi.hProcess);
        if (pi.hThread) CloseHandle(pi.hThread);
    } else {
        DWORD err = GetLastError();
        wchar_t err_buf[256];
        swprintf_s(err_buf, L"Installation failed.\nError code: %lu\n\n"
                             L"Make sure you ran as Administrator.", err);
        MessageBoxW(NULL, err_buf, L"Velora Install Error",
                    MB_OK | MB_ICONERROR);
    }
}

// ─── Compiler Mode (the actual velora compiler) ───

// The full compiler is embedded in this binary.
// When run as `velora run file.vel`, it compiles and runs the file.

#include <iostream>
#include <fstream>
#include <iterator>

// Forward declarations (full compiler from compiler/*.cpp)
// In the build, all compiler sources are compiled together with this file.
// So lexer.cpp, parser.cpp, analyzer.cpp, codegen.cpp are all linked in.

// velora_main() is provided by compiler_bridge.cpp

// ─── WinMain Entry Point ───

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR lpCmdLine, int nCmdShow) {
    (void)hInstance; (void)nCmdShow;

    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);

    // Check for command-line modes
    if (argc >= 2) {
        std::wstring cmd1 = argv[1];

        // Tray mode: started by Windows startup
        if (cmd1 == L"--tray") {
            LocalFree(argv);
            return run_tray_mode();
        }

        // Uninstall mode
        if (cmd1 == L"--uninstall") {
            int choice = MessageBoxW(NULL,
                L"Are you sure you want to uninstall Velora?",
                L"Velora Uninstall", MB_YESNO | MB_ICONQUESTION);
            if (choice == IDYES) {
                do_uninstall();
                Shell_NotifyIconW(NIM_DELETE, &g_nid);
                MessageBoxW(NULL,
                    L"Velora has been uninstalled.\n"
                    L"The installation folder will be removed on next restart.",
                    L"Uninstalled", MB_OK);
            }
            LocalFree(argv);
            return 0;
        }

        // Install mode
        if (cmd1 == L"--install") {
            show_install_ui();
            LocalFree(argv);
            return 0;
        }

        // Compiler commands: run, build, check, new, version, tokens, help
        // Redirect to a console window so output is visible
        AttachConsole(ATTACH_PARENT_PROCESS);
        if (!AttachConsole(ATTACH_PARENT_PROCESS)) {
            AllocConsole();
        }

        LocalFree(argv);

        // Re-parse as multibyte for compiler
        int mb_argc = 0;
        LPWSTR* mb_argv_w = CommandLineToArgvW(GetCommandLineW(), &mb_argc);

        // Convert wchar args to char** for compiler
        std::vector<std::string> args_str;
        std::vector<const char*> args_ptr;
        for (int i = 0; i < mb_argc; i++) {
            int len = WideCharToMultiByte(CP_UTF8, 0, mb_argv_w[i], -1, NULL, 0, NULL, NULL);
            std::string s(len, '\0');
            WideCharToMultiByte(CP_UTF8, 0, mb_argv_w[i], -1, &s[0], len, NULL, NULL);
            if (!s.empty() && s.back() == '\0') s.pop_back();
            args_str.push_back(s);
            args_ptr.push_back(args_str.back().c_str());
        }
        LocalFree(mb_argv_w);

        // Reopen stdout/stderr to console
        FILE* dummy;
        freopen_s(&dummy, "CONOUT$", "w", stdout);
        freopen_s(&dummy, "CONOUT$", "w", stderr);
        freopen_s(&dummy, "CONIN$", "r", stdin);

        // Include the compiler's main logic inline
        // (The externally linked velora_compiler_main handles run/build/check/etc.)
        // For the self-contained build, main.cpp's logic is called directly.
        extern int velora_main(int argc, char** argv);
        int result = velora_main((int)args_ptr.size(), (char**)args_ptr.data());
        FreeConsole();
        return result;
    }

    // No arguments: show install UI (first run behavior)
    LocalFree(argv);

    // Check if already installed
    if (PathFileExistsW(installed_exe().c_str())) {
        int choice = MessageBoxW(NULL,
            L"Velora is already installed on this system.\n\n"
            L"What would you like to do?",
            L"Velora Language",
            MB_YESNOCANCEL | MB_ICONQUESTION);

        // Yes = repair/reinstall, No = uninstall, Cancel = open folder
        if (choice == IDYES) {
            show_install_ui();
        } else if (choice == IDNO) {
            do_uninstall();
            MessageBoxW(NULL, L"Velora has been uninstalled.", L"Done", MB_OK);
        } else {
            ShellExecuteW(NULL, L"explore", install_dir().c_str(), NULL, NULL, SW_SHOW);
        }
    } else {
        show_install_ui();
    }

    return 0;
}
