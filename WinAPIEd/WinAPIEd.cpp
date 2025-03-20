#define SelectBrush(hdc, hbr) \
   ((HBRUSH)SelectObject((hdc), (HGDIOBJ)(HBRUSH)(hbr)))

#include <windows.h>
#include <windowsx.h>
#include <time.h>
#include <stack>
#include <iostream>
#include <conio.h>

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

using namespace std;

enum TypeMode { _rubber = 0, _menu, _free };
enum TypeCUR { с_line = 0, с_rect, с_ark, с_square };
enum TypeObj { _line = 0, _rect, _ark };
enum TypeMenu { _hor = 0, _vert };
CONST CHAR* m1text[4]{ "line" ,"rect","ellips", "unzoom" };
bool mouseLDownF;
HPEN lPen = CreatePen(PS_SOLID, 2, RGB(0, 0, 255));
HPEN lPen2 = CreatePen(PS_SOLID, 3, RGB(0, 255, 0));

struct fPOINT {
	DOUBLE x, y;
};

struct MyPrim {
	fPOINT p1, p2;
	TypeObj PrimType;
	bool hatch;
};

struct MyMenu {
	int x = 0, y = 0;
	int h = 50;
	int w = 100;
	int s = 20, itemCount = 0;
	TypeMenu tp = _hor;
	CONST CHAR** text = NULL;
};
DWORD SelectedItem = 0;

stack <fPOINT> stackW;
HDC hdc_m;
HDC hdc_mm;
HDC hdc;
HBRUSH hbr = GetStockBrush(NULL_BRUSH);
HBRUSH h_hbr = CreateHatchBrush(HS_BDIAGONAL, RGB(0, 0, 255));

MyMenu m1;
MyPrim	Model[100];
TypeObj objType = _line;
INT PrimCount = 0;
bool hatch_m = FALSE;

DWORD xmin = 5;
DWORD ymin = 65;
DWORD xmax = xmin + 600;
DWORD ymax = ymin + 600;

tagPOINT VpMin = { xmin,ymin };
tagPOINT VpMax = { xmax,ymax };
fPOINT WinMin = { 0,0 };
fPOINT WinMax = { 1,1 };

tagPOINT pb = { 0,0 };
tagPOINT pe = { 0,0 };
TypeCUR Cursor;
TypeMode Mode = _free;

void drawModel(HDC hdc)
{
	HPEN lPen = CreatePen(PS_SOLID, 3, RGB(0, 0, 255)); // создаем перо
	HGDIOBJ hOld = SelectObject(hdc, lPen);
	HRGN clip = CreateRectRgn(VpMin.x, VpMin.y, VpMax.x, VpMax.y);
	SelectClipRgn(hdc, clip);
	Rectangle(hdc, VpMin.x, VpMin.y, VpMax.x, VpMax.y);
	if (PrimCount > 0) {
		for (DWORD i = 0; i <= PrimCount - 1; i++) {
			//drawPrim(hdc, Model[i].PrimType, NDCtoDC(Model[i].p1), NDCtoDC(Model[i].p2));
		}
	}
	SelectObject(hdc, hOld);
	DeleteObject(lPen);
	DeleteObject(clip);
	SelectClipRgn(hdc, NULL);
}

void onMouseLUp(HWND hwnd, LPARAM lParam) {
	//HDC hdc = GetDC(hwnd);
	tagPOINT Point;
	Point.x = lParam % 0x10000;
	Point.y = lParam / 0x10000;

	mouseLDownF = FALSE;
	
	SetROP2(hdc, R2_NOTXORPEN);
	pe = Point;
	
	MoveToEx(hdc, pb.x, pb.y, NULL);
	LineTo(hdc, pe.x, pe.y);

	Model[PrimCount].p1.x = pb.x;
	Model[PrimCount].p1.y = pb.y;

	Model[PrimCount].p2.x = pe.x;
	Model[PrimCount].p2.y = pe.y;

	//Model[PrimCount].PrimType = _line;
	Model[PrimCount].PrimType = objType;
	Model[PrimCount].hatch = hatch_m;
	PrimCount++;

	//ReleaseDC(hwnd, hdc);
	InvalidateRect(hwnd, NULL, TRUE);
}

void onMouseLDown(HWND hwnd, LPARAM lParam) {
	//HDC hdc = GetDC(hwnd); 
	tagPOINT Point;

	Point.x = lParam % 0x10000;
	Point.y = lParam / 0x10000;

	if (Point.y < ymin) {
		
		mouseLDownF = FALSE;
		return;
	}

	mouseLDownF = TRUE;

	pb=Point;
	pe = pb;

	//ReleaseDC(hwnd, hdc);

}

void onMouseMove(HWND hwnd, LPARAM lParam) {
	
	if (mouseLDownF) {
		SelectObject(hdc, lPen2);
		SetROP2(hdc, R2_NOTXORPEN);
		// Стираем предыдущую линию
		MoveToEx(hdc, pb.x, pb.y, NULL);
		LineTo(hdc, pe.x, pe.y);

		pe.x = LOWORD(lParam);
		pe.y = HIWORD(lParam);
		
		MoveToEx(hdc, pb.x, pb.y, NULL);
		LineTo(hdc, pe.x, pe.y);
		//ReleaseDC(hwnd, hdc);
	}
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	PWSTR lpCmdLine, int nCmdShow) {
	HWND hwnd;
	MSG  msg;
	WNDCLASSW wc = { 0 };

	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpszClassName = L"Graph";
	wc.hInstance = hInstance;
	wc.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
	wc.lpfnWndProc = WndProc;
	wc.hCursor = LoadCursor(0, IDC_ARROW);

	RegisterClassW(&wc);
	hwnd = CreateWindowW(wc.lpszClassName, L"Простой редактор",
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		100, 100, 750, 750, NULL, NULL, hInstance, NULL);

	CreateWindowW(L"BUTTON", L"Line", WS_VISIBLE | WS_CHILD, 10, 10, 80, 30, hwnd, (HMENU)1, hInstance, NULL);
	CreateWindowW(L"BUTTON", L"Rect", WS_VISIBLE | WS_CHILD, 100, 10, 80, 30, hwnd, (HMENU)2, hInstance, NULL);
	CreateWindowW(L"BUTTON", L"Ellips", WS_VISIBLE | WS_CHILD, 190, 10, 80, 30, hwnd, (HMENU)3, hInstance, NULL);
	CreateWindowW(L"BUTTON", L"Штриховка", WS_VISIBLE | WS_CHILD, 280, 10, 80, 30, hwnd, (HMENU)4, hInstance, NULL);
	

	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg,
	WPARAM wParam, LPARAM lParam) {
	PAINTSTRUCT ps;
	RECT r;
	//HDC hdc;
	
	
	HGDIOBJ hOld2, hOld;


	switch (msg) {
	case WM_PAINT:
		GetClientRect(hwnd, &r);
		hdc = BeginPaint(hwnd, &ps);
		SetROP2(hdc, R2_COPYPEN);
		
		SelectObject(hdc, lPen);
		for (int i = 0; i < PrimCount; i++) {

			if (Model[i].hatch) {
				SelectBrush(hdc,h_hbr);
			}
			else {
				SelectBrush(hdc, hbr);
			}

			if (Model[i].PrimType == _line) {
				MoveToEx(hdc, Model[i].p1.x, Model[i].p1.y, NULL);
				LineTo(hdc, Model[i].p2.x, Model[i].p2.y);
			}
			else if (Model[i].PrimType == _rect) {
				MoveToEx(hdc, Model[i].p1.x, Model[i].p1.y, NULL);
				//hbr = CreateHatchBrush(HS_BDIAGONAL, RGB(0, 0, 255));
				//hbr = GetStockBrush(NULL_BRUSH);
				//SelectBrush(hdc, hbr);
				Rectangle(hdc, Model[i].p1.x, Model[i].p1.y, Model[i].p2.x, Model[i].p2.y);
			}
			else if (Model[i].PrimType == _ark){
				MoveToEx(hdc, Model[i].p1.x, Model[i].p1.y, NULL);
				//hbr = GetStockBrush(NULL_BRUSH);
				//SelectBrush(hdc, hbr);
				Ellipse(hdc, Model[i].p1.x, Model[i].p1.y, Model[i].p2.x, Model[i].p2.y);
			}
			
		}

		//hdc_m = hdc;
		EndPaint(hwnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_MOUSEMOVE:
		onMouseMove(hwnd, lParam);
		break;
	case WM_LBUTTONDOWN:
		onMouseLDown(hwnd, lParam);
		break;
	case WM_LBUTTONUP:
		onMouseLUp(hwnd, lParam);
		break;
	case WM_RBUTTONDOWN:
		//onMouseRDown(hwnd, lParam);
		break;
	case WM_RBUTTONUP:
		//onMouseRUp(hwnd, lParam);
		break;
	case WM_COMMAND:
		// Обработка нажатий на кнопки
		switch (LOWORD(wParam)) {
		case 1: 
			//Cursor = с_line;
			objType = _line;
			break;
		case 2: 
			//Cursor = с_rect;
			objType = _rect;
			break;
		case 3: 
			//Cursor = с_ark;
			objType = _ark;
			break;
		case 4:
			if (!hatch_m) {
				hatch_m = TRUE;
			}
			else {
				hatch_m = FALSE;
			}
			
			//hbr = CreateHatchBrush(HS_BDIAGONAL, RGB(0, 0, 255));
		}
		break;
	}
	return DefWindowProcW(hwnd, msg, wParam, lParam);
}

/*
hdc_m = GetDC(hwnd);
		hdc_mm = GetDC(hwnd);
		m1.tp = _hor; m1.itemCount = 4; m1.x = 5; m1.y = 5; m1.text = m1text;

		lPen = CreatePen(PS_SOLID, 3, RGB(0, 0, 255)); // создаем перо
		lPen2 = CreatePen(PS_DASH, 1, RGB(255, 0, 0)); // создаем перо
		hbr = CreateHatchBrush(HS_BDIAGONAL, RGB(0, 0, 255));

		hOld2 = SelectObject(hdc_mm, lPen2);

		MoveToEx(hdc_m, 10, 10, NULL);
		LineTo(hdc_m, 100, 10);

		hOld = SelectObject(hdc_m, lPen);
		MoveToEx(hdc_mm, 100, 10, NULL);
		LineTo(hdc_mm, 200, 100);

		MoveToEx(hdc_m, 200, 10, NULL);
		LineTo(hdc_m, 300, 100);

		MoveToEx(hdc_m, 10, 200, NULL);
		Rectangle(hdc_m, 200, 150, 300, 350);

		SelectObject(hdc_mm, hbr);
		MoveToEx(hdc_m, 10, 200, NULL);
		Rectangle(hdc_m, 200, 150, 300, 350);

		SelectObject(hdc_mm, lPen2);
		SelectObject(hdc_mm, GetStockObject(NULL_BRUSH));
		MoveToEx(hdc_m, 300, 200, NULL);
		Ellipse(hdc_m, 500, 150, 300, 350);

		ReleaseDC(hwnd, hdc_mm);
		ReleaseDC(hwnd, hdc_m);
*/

