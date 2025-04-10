#include "framework.h"
#include "Data Encrypter And Decrypter.h"
#include <windowsx.h>
#include <string>
#include <algorithm>
#include <cctype>
#include <vector>
#include <sstream>
#include <iomanip>
#include <atlbase.h>

#define MAX_LOADSTRING 100

// Forward declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);

// Global Variables:
HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];

// UI Elements
HWND hInputBox, hOutputBox, hShiftBox, hEncryptButton, hDecryptButton;
HWND hInputLabel, hOutputLabel, hShiftLabel;

std::string CaesarEncrypt(const std::string& text, int shift) {
    std::string result;
    for (char c : text) {
        if (std::isupper(c))
            result += (c - 'A' + shift) % 26 + 'A';
        else if (std::islower(c))
            result += (c - 'a' + shift) % 26 + 'a';
        else
            result += c;
    }
    return result;
}

std::string CaesarDecrypt(const std::string& text, int shift) {
    return CaesarEncrypt(text, 26 - (shift % 26));
}

std::string Atbash(const std::string& text) {
    std::string result;
    for (char c : text) {
        if (std::isupper(c))
            result += 'Z' - (c - 'A');
        else if (std::islower(c))
            result += 'z' - (c - 'a');
        else
            result += c;
    }
    return result;
}

std::string ROT13(const std::string& text) {
    return CaesarEncrypt(text, 13);
}

std::string Base64Encode(const std::string& input) {
    static const char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string result;
    int val = 0, valb = -6;
    for (unsigned char c : input) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            result.push_back(table[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) result.push_back(table[((val << 8) >> (valb + 8)) & 0x3F]);
    while (result.size() % 4) result.push_back('=');
    return result;
}

std::string Base64Decode(const std::string& input) {
    static const char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::vector<int> T(256, -1);
    for (int i = 0; i < 64; ++i) T[table[i]] = i;
    int val = 0, valb = -8;
    std::string result;
    for (unsigned char c : input) {
        if (T[c] == -1) break;
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0) {
            result.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return result;
}

std::string EncryptFull(const std::string& input, int shift) {
    std::string caesar = CaesarEncrypt(input, shift);
    std::string atbash = Atbash(caesar);
    std::string rot13 = ROT13(atbash);
    return Base64Encode(rot13);
}

std::string DecryptFull(const std::string& input, int shift) {
    std::string decoded = Base64Decode(input);
    std::string rot13 = ROT13(decoded); // Reverse ROT13
    std::string atbash = Atbash(rot13);  // Reverse Atbash
    return CaesarDecrypt(atbash, shift); // Reverse Caesar shift
}

void SetText(HWND hWnd, const std::string& text) {
    SetWindowTextA(hWnd, text.c_str());
}

std::string GetText(HWND hWnd) {
    int len = GetWindowTextLengthA(hWnd);
    std::vector<char> buffer(len + 1);
    GetWindowTextA(hWnd, buffer.data(), len + 1);
    return std::string(buffer.data());
}

void OnEncrypt() {
    std::string input = GetText(hInputBox);
    std::string shiftStr = GetText(hShiftBox);
    if (input.empty()) {
        MessageBoxA(NULL, "Please enter text to encrypt.", "Input Error", MB_ICONERROR);
        return;
    }
    if (shiftStr.empty() || !std::all_of(shiftStr.begin(), shiftStr.end(), ::isdigit)) {
        MessageBoxA(NULL, "Please enter a valid number for Caesar shift.", "Input Error", MB_ICONERROR);
        return;
    }
    int shift = std::stoi(shiftStr);
    std::string result = EncryptFull(input, shift);
    SetText(hOutputBox, result);
}

void OnDecrypt() {
    std::string input = GetText(hInputBox);
    std::string shiftStr = GetText(hShiftBox);
    if (input.empty()) {
        MessageBoxA(NULL, "Please enter text to decrypt.", "Input Error", MB_ICONERROR);
        return;
    }
    if (shiftStr.empty() || !std::all_of(shiftStr.begin(), shiftStr.end(), ::isdigit)) {
        MessageBoxA(NULL, "Please enter a valid number for Caesar shift.", "Input Error", MB_ICONERROR);
        return;
    }
    int shift = std::stoi(shiftStr);
    std::string result = DecryptFull(input, shift);
    SetText(hOutputBox, result);
}

ATOM MyRegisterClass(HINSTANCE hInstance) {
    WNDCLASSEXW wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DATAENCRYPTERANDDECRYPTER));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_DATAENCRYPTERANDDECRYPTER);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
    hInst = hInstance;
    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, 600, 400, nullptr, nullptr, hInstance, nullptr);
    if (!hWnd) return FALSE;
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE:
        hInputLabel = CreateWindowW(L"STATIC", L"Input Text:", WS_CHILD | WS_VISIBLE,
            20, 20, 100, 20, hWnd, NULL, hInst, NULL);

        hInputBox = CreateWindowW(L"EDIT", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
            130, 20, 400, 20, hWnd, NULL, hInst, NULL);

        hShiftLabel = CreateWindowW(L"STATIC", L"Caesar Shift:", WS_CHILD | WS_VISIBLE,
            20, 60, 100, 20, hWnd, NULL, hInst, NULL);

        hShiftBox = CreateWindowW(L"EDIT", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
            130, 60, 100, 20, hWnd, NULL, hInst, NULL);

        hEncryptButton = CreateWindowW(L"BUTTON", L"Encrypt", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            250, 60, 100, 24, hWnd, (HMENU)1, hInst, NULL);

        hDecryptButton = CreateWindowW(L"BUTTON", L"Decrypt", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            370, 60, 100, 24, hWnd, (HMENU)2, hInst, NULL);

        hOutputLabel = CreateWindowW(L"STATIC", L"Output:", WS_CHILD | WS_VISIBLE,
            20, 100, 100, 20, hWnd, NULL, hInst, NULL);

        hOutputBox = CreateWindowW(L"EDIT", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_READONLY,
            130, 100, 400, 20, hWnd, NULL, hInst, NULL);
        break;

    case WM_SIZE: {
        // Get the current client size of the window
        int clientWidth = LOWORD(lParam);
        int clientHeight = HIWORD(lParam);

        // Resize and reposition controls based on the client area
        int labelWidth = 100;
        int labelHeight = 20;
        int boxHeight = 20;
        int buttonWidth = 100;
        int buttonHeight = 24;
        int buttonPadding = 10; // Padding between buttons
        int controlPadding = 10;

        // Calculate the width for the input/output boxes (leaving space for labels)
        int boxWidth = clientWidth - 160; // 130 for the left margin and 30 for the label space

        // Resize and reposition labels
        MoveWindow(hInputLabel, 20, 20, labelWidth, labelHeight, TRUE);
        MoveWindow(hShiftLabel, 20, 60, labelWidth, labelHeight, TRUE);
        MoveWindow(hOutputLabel, 20, 100, labelWidth, labelHeight, TRUE);

        // Resize and reposition input box
        MoveWindow(hInputBox, 130, 20, boxWidth, boxHeight, TRUE);

        // Resize and reposition Caesar shift box (reduce width if necessary)
        int shiftBoxWidth = 80; // Adjust the width to fit better
        MoveWindow(hShiftBox, 130, 60, shiftBoxWidth, boxHeight, TRUE);

        // Resize and reposition output box
        MoveWindow(hOutputBox, 130, 100, boxWidth, boxHeight, TRUE);

        // Adjust button positioning to ensure they don't overlap
        // Place buttons below the text boxes, centered
        int buttonX = (clientWidth - buttonWidth * 2 - buttonPadding) / 2; // Center both buttons
        MoveWindow(hEncryptButton, buttonX, 140, buttonWidth, buttonHeight, TRUE);
        MoveWindow(hDecryptButton, buttonX + buttonWidth + buttonPadding, 140, buttonWidth, buttonHeight, TRUE);

        break;
    }


    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case 1:
            OnEncrypt();
            break;
        case 2:
            OnDecrypt();
            break;
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
        break;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}


INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    UNREFERENCED_PARAMETER(lParam);
    switch (message) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hDlg, &ps);
        const wchar_t* credits = L"Made by Quinn Taylor\n";
        RECT rect;
        GetClientRect(hDlg, &rect);
        DrawTextW(hdc, credits, -1, &rect, DT_CENTER | DT_BOTTOM | DT_NOPREFIX);
        EndPaint(hDlg, &ps);
        return (INT_PTR)TRUE;
    }
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_DATAENCRYPTERANDDECRYPTER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    if (!InitInstance(hInstance, nCmdShow))
        return FALSE;

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DATAENCRYPTERANDDECRYPTER));

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}
