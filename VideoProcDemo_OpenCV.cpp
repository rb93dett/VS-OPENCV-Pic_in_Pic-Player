// VideoProcDemo_OpenCV.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "VideoProcDemo_OpenCV.h"
#include "shobjidl_core.h"
#include <windowsx.h>

#include "opencv2/opencv.hpp" 
#include <iostream>
using namespace std;
#define MAX_LOADSTRING 100

// 全局变量:
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名

cv::Mat img;
cv::Mat img1;
WCHAR FileNameOfVideo1[1024];                   // 视频1的文件路径和文件名
WCHAR FileNameOfVideo2[1024];                   // 画中画的文件路径和文件名
cv::VideoCapture VidCap1;                       // 视频1的读取器
cv::VideoCapture VidCap2;                       // 画中画的读取器

enum PlayState
{
    playing, paused, stopped
};
PlayState playState = PlayState::stopped;       // 播放状态  

PlayState playState1 = PlayState::stopped;       // 播放状态     

enum VideoEffect
{
    no, edge
};
VideoEffect vidEffect = VideoEffect::no;        // 视频画面效果
VideoEffect vidEffect1 = VideoEffect::no;        // 画中画画面效果

typedef struct tagPaintInfo
{
    POINT ptBegin;
    POINT ptEnd;
    int nType;
    tagPaintInfo* pNext;
}PInfo;

static int nType = 0;
static POINT ptBegin;
static POINT ptEnd;
static bool bFlag = true;
static PInfo * pHead = NULL;
static PInfo * pEnd = NULL;
static PInfo * pNew = NULL;
PInfo * pTemp = NULL;


// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
bool OpenVideoFile(HWND hWnd, LPWSTR* fn);
std::string WCHAR2String(LPCWSTR pwszSrc);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 在此处放置代码。
    //img = cv::imread("d://wallpaper.jpg");

    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_VIDEOPROCDEMOOPENCV, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_VIDEOPROCDEMOOPENCV));

    MSG msg;

    // 主消息循环:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_VIDEOPROCDEMOOPENCV));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_VIDEOPROCDEMOOPENCV);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 将实例句柄存储在全局变量中

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   SetTimer(hWnd, 1, 40, NULL);

   return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    WCHAR* fn = (WCHAR*)FileNameOfVideo1;
    WCHAR* fn1 = (WCHAR*)FileNameOfVideo2;
    bool result;
    bool result1;
    PAINTSTRUCT ps;
    HDC hdc;

    switch (message)
    {
    case WM_LBUTTONDOWN:
        ptBegin.x = LOWORD(lParam);
        ptBegin.y = HIWORD(lParam);
        break;
    case WM_LBUTTONUP:
        ptEnd.x = LOWORD(lParam);
        ptEnd.y = HIWORD(lParam);
        hdc = GetDC(hWnd);
        if (nType == 1)
        {
            MoveToEx(hdc, ptBegin.x, ptBegin.y, NULL);
            LineTo(hdc, ptEnd.x, ptEnd.y);
        }
        if (bFlag)
        {
            pNew  = (PInfo *)malloc(sizeof(PInfo));
            pNew->ptBegin = ptBegin;
            pNew->ptEnd = ptEnd;
            pNew->nType = nType;
            pNew->pNext = NULL;

            pHead = pNew;
            pEnd = pHead;
            bFlag = false;
        }
        else
        {
            // 创建一个新节点  
            pNew  = (PInfo *)malloc(sizeof(PInfo));
            pNew->ptBegin = ptBegin;
            pNew->ptEnd = ptEnd;
            pNew->nType = nType;
            pNew->pNext = NULL;

            pEnd->pNext = pNew;
            pEnd = pNew;
        }
        break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 分析菜单选择:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            case IDM_OPEN_VID1:                
                result = OpenVideoFile(hWnd, &fn);
                if (result)
                {
                    //img = cv::imread(WCHAR2String(fn));
                    bool opened = VidCap1.open(WCHAR2String(fn));

                    if (opened)
                    {
                        VidCap1 >> img; //获取第一帧图像并显示

                        //激发WM_PAINT时间，让窗口重绘
                        InvalidateRect(hWnd, NULL, false);
                    }
                    else
                    {
                        MessageBox(
                            hWnd,
                            L"视频未能打开",
                            L"错误提示",
                            MB_OK
                        );
                    }
                }
                break;
            case IDM_PLAY_VID:
                playState = PlayState::playing;
                break;
            case IDM_PAUSE_VID:
                playState = PlayState::paused;
                break;
            case IDM_STOP_VID:
                playState = PlayState::stopped;
                VidCap1.set(cv::VideoCaptureProperties::CAP_PROP_POS_FRAMES, 0);                
                break;
            case IDM_NO_EFFECT:
                vidEffect = VideoEffect::no;
                break;
            case IDM_EDGE_EFFECT:
                vidEffect = VideoEffect::edge;
                break;
            case ID_OPEN_PIP:
                result1 = OpenVideoFile(hWnd, &fn1);
                if (result1)
                {
                    //img1 = cv::imread(WCHAR2String(fn));
                    bool opened1 = VidCap2.open(WCHAR2String(fn1));

                    if (opened1)
                    {
                        VidCap2 >> img1; //获取第一帧图像并显示

                        //激发WM_PAINT时间，让窗口重绘
                        InvalidateRect(hWnd, NULL, false);
                    }
                    else
                    {
                        MessageBox(
                            hWnd,
                            L"视频未能打开",
                            L"错误提示",
                            MB_OK
                        );
                    }
                }
                break;
            case ID_PLAY_PIP:
                playState1 = PlayState::playing;
                break;
            case ID_PAUSE_PIP:
                playState1 = PlayState::paused;
                break;
            case ID_STOP_PIP:
                playState1 = PlayState::stopped;
                VidCap2.set(cv::VideoCaptureProperties::CAP_PROP_POS_FRAMES, 0);
                break;
            case ID_PIP_EDGE:
                vidEffect1 = VideoEffect::edge;
                break;
            case ID_NO_EFFECT:
                vidEffect1 = VideoEffect::no;
                break;
            case ID_DRAW:
                nType = 1;
                break;
            case ID_STOP_DRAW:
                nType = 0;
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_TIMER:
        if (VidCap1.isOpened() && playState == PlayState::playing)
        {
            //OutputDebugString(L"输出调DD试信息123");
            VidCap1 >> img;

            if (img.empty() == false)
            {
                if (vidEffect == VideoEffect::edge)
                {                    
                    //cv::cvtColor(img, img, cv::COLOR_BGR2GRAY);
                    cv::Mat edgeY, edgeX;
                    cv::Sobel(img, edgeY, CV_8U, 1, 0);
                    cv::Sobel(img, edgeX, CV_8U, 0, 1);
                    img = edgeX + edgeY;
                }

                InvalidateRect(hWnd, NULL, false);
            }
            if (img.empty() == true)
            {
                VidCap1.set(cv::VideoCaptureProperties::CAP_PROP_POS_FRAMES, 0);
            }
        }
        if (VidCap2.isOpened() && playState1 == PlayState::playing)
        {
            VidCap2 >> img1;

            if (img1.empty() == false)
            {
                if (vidEffect1 == VideoEffect::edge)
                {
                    //cv::cvtColor(img, img, cv::COLOR_BGR2GRAY);
                    cv::Mat edgeY, edgeX;
                    cv::Sobel(img1, edgeY, CV_8U, 1, 0);
                    cv::Sobel(img1, edgeX, CV_8U, 0, 1);
                    img1 = edgeX + edgeY;
                }

                InvalidateRect(hWnd, NULL, false);
            }
            if (img1.empty() == true)
            {
                VidCap2.set(cv::VideoCaptureProperties::CAP_PROP_POS_FRAMES, 0);
            }
        }
        break;

    case WM_PAINT:
    {
        hdc = BeginPaint(hWnd, &ps);
        // TODO: 在此处添加使用 hdc 的任何绘图代码...
        // 转一下格式 ,这段可以放外面,
        if (img.rows > 0)
        {
            switch (img.channels())
            {
            case 1:
                cv::cvtColor(img, img, cv::COLOR_GRAY2BGR); // GRAY单通道
                break;
            case 3:
                cv::cvtColor(img, img, cv::COLOR_BGR2BGRA);  // BGR三通道
                break;
            default:
                break;
            }

            int pixelBytes = img.channels() * (img.depth() + 1); // 计算一个像素多少个字节

                                                                 // 制作bitmapinfo(数据头)
            BITMAPINFO bitInfo;
            bitInfo.bmiHeader.biBitCount = 8 * pixelBytes;
            bitInfo.bmiHeader.biWidth = img.cols;
            bitInfo.bmiHeader.biHeight = -img.rows;
            bitInfo.bmiHeader.biPlanes = 1;
            bitInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            bitInfo.bmiHeader.biCompression = BI_RGB;
            bitInfo.bmiHeader.biClrImportant = 0;
            bitInfo.bmiHeader.biClrUsed = 0;
            bitInfo.bmiHeader.biSizeImage = 0;
            bitInfo.bmiHeader.biXPelsPerMeter = 0;
            bitInfo.bmiHeader.biYPelsPerMeter = 0;
            // Mat.data + bitmap数据头 -> MFC

            StretchDIBits(
                hdc,
                0, 0, 1100, 500,
                0, 0, img.cols, img.rows,
                img.data,
                &bitInfo,
                DIB_RGB_COLORS,
                SRCCOPY
            );   
        }

        if (img1.rows > 0)
        {
            switch (img1.channels())
            {
            case 1:
                cv::cvtColor(img1, img1, cv::COLOR_GRAY2BGR); // GRAY单通道
                break;
            case 3:
                cv::cvtColor(img1, img1, cv::COLOR_BGR2BGRA);  // BGR三通道
                break;
            default:
                break;
            }

            int pixelBytes = img1.channels() * (img1.depth() + 1); // 计算一个像素多少个字节

                                                                 // 制作bitmapinfo(数据头)
            BITMAPINFO bitInfo;
            bitInfo.bmiHeader.biBitCount = 8 * pixelBytes;
            bitInfo.bmiHeader.biWidth = img1.cols;
            bitInfo.bmiHeader.biHeight = -img1.rows;
            bitInfo.bmiHeader.biPlanes = 1;
            bitInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            bitInfo.bmiHeader.biCompression = BI_RGB;
            bitInfo.bmiHeader.biClrImportant = 0;
            bitInfo.bmiHeader.biClrUsed = 0;
            bitInfo.bmiHeader.biSizeImage = 0;
            bitInfo.bmiHeader.biXPelsPerMeter = 0;
            bitInfo.bmiHeader.biYPelsPerMeter = 0;
            // Mat.data + bitmap数据头 -> MFC

            StretchDIBits(
                hdc,
                800, 0, 300, 200,
                0, 0, img1.cols, img1.rows,
                img1.data,
                &bitInfo,
                DIB_RGB_COLORS,
                SRCCOPY
            );
        }

        pTemp = pHead;
        while (pTemp)
        {
            ptBegin = pTemp->ptBegin;
            ptEnd = pTemp->ptEnd;
            MoveToEx(hdc, ptBegin.x, ptBegin.y, NULL);
            LineTo(hdc, ptEnd.x, ptEnd.y);
            pTemp = pTemp->pNext;
        }
        EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        while (pHead)
        {
            pTemp = pHead->pNext;
            free(pHead);
            pHead = pTemp;
        }
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

bool OpenVideoFile(HWND hWnd, LPWSTR* fn)
{
    IFileDialog* pfd = NULL;
    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&pfd));

    DWORD dwFlags;
    hr = pfd->GetOptions(&dwFlags);
    hr = pfd->SetOptions(dwFlags | FOS_FORCEFILESYSTEM);

    COMDLG_FILTERSPEC rgSpec[] =
    {
        { L"MP4", L"*.mp4" },
        { L"AVI", L"*.avi" },
        { L"ALL", L"*.*" },
    };

    HRESULT SetFileTypes(UINT cFileTypes, const COMDLG_FILTERSPEC * rgFilterSpec);
    hr = pfd->SetFileTypes(ARRAYSIZE(rgSpec), rgSpec);
    hr = pfd->SetFileTypeIndex(1);

    hr = pfd->Show(hWnd);///显示打开文件对话框

    IShellItem* pShellItem = NULL;
    if (SUCCEEDED(hr))
    {
        hr = pfd->GetResult(&pShellItem);
        hr = pShellItem->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, fn);//获取文件的完整路径

        return true;
    }

    return false;

}

std::string WCHAR2String(LPCWSTR pwszSrc)
{
    int nLen = WideCharToMultiByte(CP_ACP, 0, pwszSrc, -1, NULL, 0, NULL, NULL);
    if (nLen <= 0)
        return std::string("");

    char* pszDst = new char[nLen];
    if (NULL == pszDst)
        return std::string("");

    WideCharToMultiByte(CP_ACP, 0, pwszSrc, -1, pszDst, nLen, NULL, NULL);
    pszDst[nLen - 1] = 0;

    std::string strTmp(pszDst);
    delete[] pszDst;

    return strTmp;
}
//————————————————
//版权声明：本文为CSDN博主「kingkee」的原创文章，遵循CC 4.0 BY - SA版权协议，转载请附上原文出处链接及本声明。
//原文链接：https ://blog.csdn.net/kingkee/java/article/details/98115024