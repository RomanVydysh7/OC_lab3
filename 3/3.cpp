
#include <windows.h>
#include <string>
#include <sstream>

#define IDT_TIMER 1
#define ID_BUTTON_TIMER 2
#define IDT_COUNTDOWN_TIMER 3

#define ID_BUTTON_START 1001
#define ID_BUTTON_STOP 1002
#define ID_BUTTON_RESET 1003
#define ID_EDIT_COUNTDOWN 1004
#define ID_BUTTON_SET_DATETIME 1005
#define ID_EDIT_DATE 1006
#define ID_EDIT_TIME 1007

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK CountdownWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK DateTimeWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

HWND countdownWindow = NULL;
HWND dateTimeWindow = NULL;
int countdownTime = 600; // Початковий час у секундах для зворотного таймера (10 хвилин)
bool countdownRunning = false;

std::wstring GetSystemTimeAsString() {
    SYSTEMTIME currentTime;
    GetLocalTime(&currentTime);

    std::wstringstream ss;
    ss << currentTime.wHour << L":" << currentTime.wMinute << L":" << currentTime.wSecond;

    return ss.str();
}

std::wstring GetSystemDateAsString() {
    SYSTEMTIME currentDate;
    GetLocalTime(&currentDate);

    std::wstringstream ss;
    ss << currentDate.wYear << L"/" << currentDate.wMonth << L"/" << currentDate.wDay;

    return ss.str();
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"Sample Window Class";

    WNDCLASS wc = { };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClass(&wc)) {
        MessageBox(NULL, L"Не вдалося зареєструвати клас вікна", L"Помилка", MB_OK | MB_ICONERROR);
        return 0;
    }

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"Time",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hwnd == NULL) {
        MessageBox(NULL, L"Не вдалося створити головне вікно", L"Помилка", MB_OK | MB_ICONERROR);
        return 0;
    }

    HWND button = CreateWindow(
        L"BUTTON",
        L"Таймер",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        10,
        400,
        100,
        30,
        hwnd,
        (HMENU)ID_BUTTON_TIMER,
        hInstance,
        NULL
    );

    if (button == NULL) {
        MessageBox(hwnd, L"Не вдалося створити кнопку 'Таймер'", L"Помилка", MB_OK | MB_ICONERROR);
        return 0;
    }

    button = CreateWindow(
        L"BUTTON",
        L"Встановити дату та час",
        WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        120,
        400,
        180,
        30,
        hwnd,
        (HMENU)ID_BUTTON_SET_DATETIME,
        hInstance,
        NULL
    );

    if (button == NULL) {
        MessageBox(hwnd, L"Не вдалося створити кнопку 'Встановити дату та час'", L"Помилка", MB_OK | MB_ICONERROR);
        return 0;
    }

    SetTimer(hwnd, IDT_TIMER, 1000, NULL);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT clientRect;
        GetClientRect(hwnd, &clientRect);
        FillRect(hdc, &clientRect, (HBRUSH)(COLOR_WINDOW + 1));

        std::wstring currentTime = GetSystemTimeAsString();
        std::wstring currentDate = GetSystemDateAsString();

        SetTextColor(hdc, RGB(0, 0, 0));
        SetBkMode(hdc, TRANSPARENT);

        HFONT hFontTime = CreateFont(48, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Arial");
        HFONT hOldFontTime = (HFONT)SelectObject(hdc, hFontTime);
            SIZE textSizeTime;
        GetTextExtentPoint32(hdc, currentTime.c_str(), static_cast<int>(currentTime.length()), &textSizeTime);

        int xTime = (clientRect.right - textSizeTime.cx) / 2;
        int yTime = (clientRect.bottom - textSizeTime.cy - 30) / 2;

        TextOut(hdc, xTime, yTime, currentTime.c_str(), static_cast<int>(currentTime.length()));
        HFONT hFontDate = CreateFont(24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Arial");
        HFONT hOldFontDate = (HFONT)SelectObject(hdc, hFontDate);

        SIZE textSizeDate;
        GetTextExtentPoint32(hdc, currentDate.c_str(), static_cast<int>(currentDate.length()), &textSizeDate);

        int xDate = (clientRect.right - textSizeDate.cx) / 2;
        int yDate = yTime + textSizeTime.cy;

        TextOut(hdc, xDate, yDate, currentDate.c_str(), static_cast<int>(currentDate.length()));

        SelectObject(hdc, hOldFontTime);
        DeleteObject(hFontTime);

        SelectObject(hdc, hOldFontDate);
        DeleteObject(hFontDate);

        EndPaint(hwnd, &ps);
        break;
    }
    case WM_TIMER: {
        if (wParam == IDT_TIMER) {
            RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE);
        }
        break;
    }
    case WM_COMMAND: {
        if (LOWORD(wParam) == ID_BUTTON_TIMER) {
            HINSTANCE hInstance = GetModuleHandle(NULL);

            if (countdownWindow == NULL) {
                const wchar_t COUNTDOWN_CLASS[] = L"Countdown Window Class";
                WNDCLASS countdownWc = { };
                countdownWc.lpfnWndProc = CountdownWindowProc;
                countdownWc.hInstance = hInstance;
                countdownWc.lpszClassName = COUNTDOWN_CLASS;
                countdownWc.hIcon = LoadIcon(NULL, IDI_APPLICATION);

                if (!RegisterClass(&countdownWc)) {
                    MessageBox(hwnd, L"Не вдалося зареєструвати клас вікна 'Таймер'", L"Помилка", MB_OK | MB_ICONERROR);
                    return 0;
                }

                countdownWindow = CreateWindowEx(
                    0,
                    COUNTDOWN_CLASS,
                    L"Зворотній таймер",
                    WS_OVERLAPPEDWINDOW,
                    CW_USEDEFAULT,
                    CW_USEDEFAULT,
                    300,
                    200,
                    hwnd,
                    NULL,
                    hInstance,
                    NULL
                );

                if (countdownWindow == NULL) {
                    MessageBox(hwnd, L"Не вдалося створити вікно 'Таймер'", L"Помилка", MB_OK | MB_ICONERROR);
                }
                else {
                    ShowWindow(countdownWindow, SW_SHOWNORMAL);
                    UpdateWindow(countdownWindow);
                }
            }
            else {
                SetForegroundWindow(countdownWindow);
            }
        }
        else if (LOWORD(wParam) == ID_BUTTON_SET_DATETIME) {
            HINSTANCE hInstance = GetModuleHandle(NULL);

            if (dateTimeWindow == NULL) {
                const wchar_t DATETIME_CLASS[] = L"DateTime Window Class";
                WNDCLASS dateTimeWc = { };
                dateTimeWc.lpfnWndProc = DateTimeWindowProc;
                dateTimeWc.hInstance = hInstance;
                dateTimeWc.lpszClassName = DATETIME_CLASS;
                dateTimeWc.hIcon = LoadIcon(NULL, IDI_APPLICATION);

                if (!RegisterClass(&dateTimeWc)) {
                    MessageBox(hwnd, L"Не вдалося зареєструвати клас вікна 'Встановлення дати та часу'", L"Помилка", MB_OK | MB_ICONERROR);
                    return 0;
                }
                    dateTimeWindow = CreateWindowEx(
                        0,
                        DATETIME_CLASS,
                        L"Встановлення дати та часу",
                        WS_OVERLAPPEDWINDOW,
                        CW_USEDEFAULT,
                        CW_USEDEFAULT,
                        300,
                        200,
                        hwnd,
                        NULL,
                        hInstance,
                        NULL
                    );

                if (dateTimeWindow == NULL) {
                    MessageBox(hwnd, L"Не вдалося створити вікно 'Встановлення дати та часу'", L"Помилка", MB_OK | MB_ICONERROR);
                }
                else {
                    ShowWindow(dateTimeWindow, SW_SHOWNORMAL);
                    UpdateWindow(dateTimeWindow);
                }
            }
            else {
                SetForegroundWindow(dateTimeWindow);
            }
        }
        break;
    }
    case WM_SIZE: {
        RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE);
        break;
    }
    case WM_DESTROY: {
        KillTimer(hwnd, IDT_TIMER);
        PostQuitMessage(0);
        return 0;
    }
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK CountdownWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HWND editHours, editMinutes, editSeconds;
    static HWND buttonStart, buttonStop, buttonReset;
    wchar_t buffer[16];

    switch (uMsg) {
    case WM_CREATE: {
        int left = 10, top = 10, width = 30, height = 20, gap = 10;

        editHours = CreateWindow(L"EDIT", L"0",
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
            left, top, width, height, hwnd, (HMENU)(ID_EDIT_COUNTDOWN + 1),
            (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);

        left += width + gap;

        editMinutes = CreateWindow(L"EDIT", L"0",
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
            left, top, width, height, hwnd, (HMENU)(ID_EDIT_COUNTDOWN + 2),
            (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);

        left += width + gap;

        editSeconds = CreateWindow(L"EDIT", L"0",
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
            left, top, width, height, hwnd, (HMENU)(ID_EDIT_COUNTDOWN + 3),
            (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);

        top += height + gap;
        left = 10;

        buttonStart = CreateWindow(L"BUTTON", L"Старт",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            left, top, 60, 15, hwnd, (HMENU)ID_BUTTON_START,
            (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);

        left += 80 + gap;

        buttonStop = CreateWindow(L"BUTTON", L"Стоп",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            left, top, 60, 15, hwnd, (HMENU)ID_BUTTON_STOP,
            (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);

        left += 60 + gap;

        buttonReset = CreateWindow(L"BUTTON", L"Скинути",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            left, top, 60, 15, hwnd, (HMENU)ID_BUTTON_RESET,
            (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
            return 0;
    }
    case WM_COMMAND: {
        int wmId = LOWORD(wParam);
        switch (wmId) {
        case ID_BUTTON_START: {
            int hours = GetDlgItemInt(hwnd, ID_EDIT_COUNTDOWN + 1, NULL, FALSE);
            int minutes = GetDlgItemInt(hwnd, ID_EDIT_COUNTDOWN + 2, NULL, FALSE);
            int seconds = GetDlgItemInt(hwnd, ID_EDIT_COUNTDOWN + 3, NULL, FALSE);
            countdownTime = hours * 3600 + minutes * 60 + seconds;
            if (countdownTime > 0) {
                countdownRunning = true;
                SetTimer(hwnd, IDT_COUNTDOWN_TIMER, 1000, NULL);
            }
            break;
        }
        case ID_BUTTON_STOP: {
            if (countdownRunning) {
                countdownRunning = false;
                KillTimer(hwnd, IDT_COUNTDOWN_TIMER);
            }
            break;
        }
        case ID_BUTTON_RESET: {
            countdownTime = 0;
            countdownRunning = false;
            KillTimer(hwnd, IDT_COUNTDOWN_TIMER);
            SetDlgItemText(hwnd, ID_EDIT_COUNTDOWN + 1, L"0");
            SetDlgItemText(hwnd, ID_EDIT_COUNTDOWN + 2, L"0");
            SetDlgItemText(hwnd, ID_EDIT_COUNTDOWN + 3, L"0");
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        }
        }
        break;
    }
    case WM_TIMER: {
        if (wParam == IDT_COUNTDOWN_TIMER && countdownRunning) {
            if (countdownTime > 0) {
                countdownTime--;
                InvalidateRect(hwnd, NULL, TRUE);
            }
            else {
                KillTimer(hwnd, IDT_COUNTDOWN_TIMER);
                countdownRunning = false;
                MessageBox(hwnd, L"Час вийшов!", L"Таймер", MB_OK | MB_ICONINFORMATION);
            }
        }
        break;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        SetTextColor(hdc, RGB(0, 0, 0));
        SetBkMode(hdc, TRANSPARENT);

        HFONT hFont = CreateFont(48, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, VARIABLE_PITCH, L"Arial");
        HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);

        int minutes = countdownTime / 60;
        int seconds = countdownTime % 60;
        wsprintf(buffer, L"%02d:%02d", minutes, seconds);

        RECT rect;
        GetClientRect(hwnd, &rect);
        DrawText(hdc, buffer, -1, &rect, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

        SelectObject(hdc, hOldFont);
        DeleteObject(hFont);

        EndPaint(hwnd, &ps);
        break;
    }
    case WM_ERASEBKGND: {
        HDC hdc = (HDC)wParam;
        RECT rect;
        GetClientRect(hwnd, &rect);
        FillRect(hdc, &rect, (HBRUSH)(COLOR_WINDOW + 1));
        return 1;
    }
    case WM_DESTROY: {
        countdownRunning = false;
        KillTimer(hwnd, IDT_COUNTDOWN_TIMER);
        countdownWindow = NULL;
        PostQuitMessage(0);
        break;
    }
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK DateTimeWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE: {
        int left = 10, top = 10, width = 100, height = 20, gap = 10;

        CreateWindow(L"STATIC", L"Дата (рік/місяць/день):",
            WS_CHILD | WS_VISIBLE,
            left, top, 200, height, hwnd, NULL, NULL, NULL);

        top += height + gap;

        CreateWindow(L"EDIT", GetSystemDateAsString().c_str(),
            WS_CHILD | WS_VISIBLE | WS_BORDER,
            left, top, 200, height, hwnd, (HMENU)ID_EDIT_DATE, NULL, NULL);

        top += height + gap;

        CreateWindow(L"STATIC", L"Час (година:хвилина):",
            WS_CHILD | WS_VISIBLE,
            left, top, 200, height, hwnd, NULL, NULL, NULL);

        top += height + gap;

        CreateWindow(L"EDIT", GetSystemTimeAsString().c_str(),
            WS_CHILD | WS_VISIBLE | WS_BORDER,
            left, top, 200, height, hwnd, (HMENU)ID_EDIT_TIME, NULL, NULL);

        top += height + gap;
            CreateWindow(L"BUTTON", L"Змінити",
                WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                left, top, 100, 25, hwnd, (HMENU)ID_BUTTON_START, NULL, NULL);

        break;
    }
    case WM_COMMAND: {
        if (LOWORD(wParam) == ID_BUTTON_START) {
            wchar_t dateBuffer[50];
            wchar_t timeBuffer[50];

            GetDlgItemText(hwnd, ID_EDIT_DATE, dateBuffer, 50);
            GetDlgItemText(hwnd, ID_EDIT_TIME, timeBuffer, 50);

            SYSTEMTIME newSystemTime;
            GetLocalTime(&newSystemTime);

            swscanf_s(dateBuffer, L"%d/%d/%d", &newSystemTime.wYear, &newSystemTime.wMonth, &newSystemTime.wDay);
            swscanf_s(timeBuffer, L"%d:%d", &newSystemTime.wHour, &newSystemTime.wMinute);

            SetLocalTime(&newSystemTime);
        }
        break;
    }
    case WM_CLOSE:
    case WM_DESTROY: {
        dateTimeWindow = NULL;
        PostQuitMessage(0);
        break;
    }
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}