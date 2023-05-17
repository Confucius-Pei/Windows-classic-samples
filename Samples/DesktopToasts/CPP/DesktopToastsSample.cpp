#include <Windows.h>

#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.system.h>
#include <winrt/windows.ui.xaml.hosting.h>
#include <winrt/windows.ui.xaml.controls.h>
#include <winrt/Windows.ui.xaml.media.h>

#include <windows.ui.xaml.hosting.desktopwindowxamlsource.h>
#include <winrt/MyUWPApp.h>

using namespace winrt;
using namespace winrt::Windows::UI;
using namespace winrt::Windows::UI::Composition;
using namespace Windows::UI::Xaml::Hosting;
using namespace winrt::Windows::Foundation::Numerics;

class DesktopToastsApp
{
public:
    static DesktopToastsApp* GetInstance()
    {
        return s_currentInstance;
    }

    HWND m_hwnd = nullptr;

    DesktopToastsApp();
    ~DesktopToastsApp();
    HRESULT Initialize(_In_ HINSTANCE hInstance);
    void RunMessageLoop();

    HRESULT SetMessage(PCWSTR message);

private:
    static LRESULT CALLBACK WndProc(
        _In_ HWND hWnd,
        _In_ UINT message,
        _In_ WPARAM wParam,
        _In_ LPARAM lParam
        );

    HWND m_hEdit = nullptr;

    static const WORD HM_TEXTBUTTON = 1;

    static DesktopToastsApp* s_currentInstance;
};

DesktopToastsApp* DesktopToastsApp::s_currentInstance = nullptr;


// Main function
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ int)
{
    // The call to winrt::init_apartment initializes COM; by default, in a multithreaded apartment.
    winrt::init_apartment();

    DesktopToastsApp app;
    app.Initialize(hInstance);

    // Begin XAML Island section.

    auto hostApp = winrt::MyUWPApp::App{};

    // Initialize the XAML framework's core window for the current thread.
    auto winxamlmanager = WindowsXamlManager::InitializeForCurrentThread();

    // This DesktopWindowXamlSource is the object that enables a non-UWP desktop application 
    // to host WinRT XAML controls in any UI element that is associated with a window handle (HWND).
    DesktopWindowXamlSource desktopSource;

    // Get handle to the core window.
    auto interop = desktopSource.as<IDesktopWindowXamlSourceNative>();

    // Parent the DesktopWindowXamlSource object to the current window.
    check_hresult(interop->AttachToWindow(app.m_hwnd));

    // This HWND will be the window handler for the XAML Island: A child window that contains XAML.  
    HWND hWndXamlIsland = nullptr;

    // Get the new child window's HWND. 
    interop->get_WindowHandle(&hWndXamlIsland);

    // Update the XAML Island window size because initially it is 0,0.
    SetWindowPos(hWndXamlIsland, 0, 0, 95, 400, 30, SWP_SHOWWINDOW);

    // Create the XAML content.
    winrt::Windows::UI::Xaml::Controls::StackPanel xamlContainer;
    xamlContainer.Background(winrt::Windows::UI::Xaml::Media::SolidColorBrush{ winrt::Windows::UI::Colors::LightGray() });

    winrt::Windows::UI::Xaml::Controls::TextBlock tb;
    tb.Text(L"Hello World from Xaml Islands!");
    tb.VerticalAlignment(winrt::Windows::UI::Xaml::VerticalAlignment::Center);
    tb.HorizontalAlignment(winrt::Windows::UI::Xaml::HorizontalAlignment::Center);
    tb.FontSize(18);

    xamlContainer.Children().Append(tb);
    xamlContainer.UpdateLayout();

    auto _myUserControl = winrt::MyUWPApp::MyUserControl();
    desktopSource.Content(_myUserControl);

    // End XAML Island section.

    app.RunMessageLoop();

    return 0;
}

DesktopToastsApp::DesktopToastsApp()
{
    s_currentInstance = this;
}

DesktopToastsApp::~DesktopToastsApp()
{
    s_currentInstance = nullptr;
}

// Prepare the main window
_Use_decl_annotations_
HRESULT DesktopToastsApp::Initialize(HINSTANCE hInstance)
{
    WNDCLASSEX wcex = { sizeof(wcex) };
    // Register window class
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = DesktopToastsApp::WndProc;
    wcex.cbWndExtra = sizeof(LONG_PTR);
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.lpszClassName = L"DesktopToastsApp";
    wcex.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
    ::RegisterClassEx(&wcex);

    // Create window
    m_hwnd = CreateWindow(
        L"DesktopToastsApp",
        L"Desktop Toasts Demo App",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        500, 300,
        nullptr, nullptr,
        hInstance, this);

    HRESULT hr = m_hwnd ? S_OK : E_FAIL;
    if (SUCCEEDED(hr))
    {
        ::CreateWindow(
            L"BUTTON",
            L"View Text Toast",
            BS_PUSHBUTTON | WS_CHILD | WS_VISIBLE,
            10, 10, 150, 25,
            m_hwnd, reinterpret_cast<HMENU>(HM_TEXTBUTTON),
            hInstance, nullptr);
        m_hEdit = ::CreateWindow(
            L"EDIT",
            L"Whatever action you take on the displayed toast will be shown here.",
            ES_LEFT | ES_MULTILINE | ES_READONLY | WS_CHILD | WS_VISIBLE | WS_BORDER,
            10, 40, 300, 50,
            m_hwnd, nullptr,
            hInstance, nullptr);

        ::ShowWindow(m_hwnd, SW_SHOWNORMAL);
        ::UpdateWindow(m_hwnd);
    }

    return hr;
}

// Standard message loop
void DesktopToastsApp::RunMessageLoop()
{
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

HRESULT DesktopToastsApp::SetMessage(PCWSTR message)
{
    ::SetForegroundWindow(m_hwnd);

    ::SendMessage(m_hEdit, WM_SETTEXT, reinterpret_cast<WPARAM>(nullptr), reinterpret_cast<LPARAM>(message));

    return S_OK;
}

// Standard window procedure
_Use_decl_annotations_
LRESULT CALLBACK DesktopToastsApp::WndProc(HWND hwnd, UINT32 message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_CREATE)
    {
        auto pcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
        auto app = reinterpret_cast<DesktopToastsApp *>(pcs->lpCreateParams);

        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(app));

        return 1;
    }

    auto app = reinterpret_cast<DesktopToastsApp *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    if (app)
    {
        switch (message)
        {
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint(hwnd, &ps);
            EndPaint(hwnd, &ps);
        }
            return 0;

        case WM_DESTROY:
        {
            PostQuitMessage(0);
        }
            return 1;
        }
    }
    return DefWindowProc(hwnd, message, wParam, lParam);
}
