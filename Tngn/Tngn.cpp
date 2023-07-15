#include "pch.h"
#include "Tngn.h"

using namespace Tngn;

Class1::~Class1()
{

}

static void DrawImpl(HWND hWnd, HDC hDC)
{
	RECT rect;
	BOOL isOk = ::GetClientRect(hWnd, &rect);
	if (!isOk) {
		return;
	}
	HBRUSH hBrush = ::CreateSolidBrush(RGB(255, 255, 0));
	HGDIOBJ hOldBrush = ::SelectObject(hDC, hBrush);
	::Rectangle(hDC, rect.left, rect.top, rect.right, rect.bottom);
	::SelectObject(hDC, hOldBrush);
	::DeleteObject(hBrush);
}

void Class1::DrawToWindow(IntPtr handleWnd)
{
	HWND hWnd = reinterpret_cast<HWND>(handleWnd.ToPointer());
	HDC hDC = GetDC(hWnd);
	DrawImpl(hWnd, hDC);
	ReleaseDC(hWnd, hDC);
}
