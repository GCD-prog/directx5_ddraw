#include <windows.h>
#include <ddraw.h>
#include <math.h>

#pragma comment(lib, "ddraw.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "winmm.lib")

#define INITGUID

#define WAIT 25

LPDIRECTDRAWSURFACE lpPrimary;
LPDIRECTDRAWSURFACE lpBackbuffer;
LPDIRECTDRAWSURFACE lpScreen;

LPDIRECTDRAW lpDDraw;

void ClipBlt(LPDIRECTDRAWSURFACE lpTarget,int x,int y,LPDIRECTDRAWSURFACE lpSource,RECT rcChar,int Flg)
{
	if(x < 0){
		rcChar.left -= x;
		x=0;
	}
	if(x + (rcChar.right - rcChar.left) > 640){
			rcChar.right -= (x + (rcChar.right - rcChar.left) - 640);
	}
	if(y < 0){
			rcChar.top -= y;
			y = 0;
	}
	if(y + (rcChar.bottom - rcChar.top) > 480){
		rcChar.bottom -= (y + (rcChar.bottom - rcChar.top) - 480);
	}
	lpTarget->BltFast(x,y,lpSource,&rcChar,Flg);
}

void FrameCnt(void)
{
		static int frame_cnt;
		static DWORD cnt_Nowtime,cnt_Prevtime;
		static char text[10];
		
		HDC hdc;
		frame_cnt++;
		cnt_Nowtime=timeGetTime();

		if((cnt_Nowtime - cnt_Prevtime) >= 1000){
			cnt_Prevtime = cnt_Nowtime;
			wsprintf(text,"%d fps",frame_cnt);
			frame_cnt=0;
		}

		lpBackbuffer->GetDC(&hdc);
		TextOut(hdc,0,0,text,strlen(text));
		lpBackbuffer->ReleaseDC(hdc);
}

void DrawFrame(void)
{
	
		static DWORD nowTime, prevTime;
		nowTime = timeGetTime();
		if((nowTime - prevTime) < 1000 / 60){
			prevTime = nowTime;
		}

		RECT Scrrc={0,0,640,480};
		
		lpBackbuffer->BltFast(0,0,lpScreen,&Scrrc,DDBLTFAST_NOCOLORKEY|DDBLTFAST_WAIT);

		int i;
		static int cnt;

		for(i=0;i<480;i++){
			Scrrc.top = i;
			Scrrc.bottom = i+1;
			ClipBlt(lpBackbuffer,(int)(sin((i + cnt) * 3.141592 / 180) * 100),i,lpBackbuffer,Scrrc,DDBLTFAST_NOCOLORKEY|DDBLTFAST_WAIT);
		}

		cnt++;
		if(cnt >= 360){
			cnt = 0;
		}
		
		//FPS
		FrameCnt();

		lpPrimary->Flip(NULL,DDFLIP_WAIT);
}

void LoadBMP(LPDIRECTDRAWSURFACE lpSurface,char *fname)
{
		HBITMAP hBmp=NULL;
		BITMAP bm;
		HDC hdc,hMemdc;
		LPDIRECTDRAWPALETTE lpPal;
		RGBQUAD rgb[256];
		PALETTEENTRY pe[256];
		int i;

		hBmp=(HBITMAP)LoadImage(GetModuleHandle(NULL),fname,IMAGE_BITMAP,0,0,LR_CREATEDIBSECTION|LR_LOADFROMFILE);
		
		GetObject(hBmp,sizeof(bm),&bm);

			hMemdc=CreateCompatibleDC(NULL);
			SelectObject(hMemdc,hBmp);

			GetDIBColorTable(hMemdc,0,256,rgb);
			for(i=0;i<256;i++){
				pe[i].peRed=rgb[i].rgbRed;
				pe[i].peGreen=rgb[i].rgbGreen;
				pe[i].peBlue=rgb[i].rgbBlue;
				pe[i].peFlags=PC_RESERVED|PC_NOCOLLAPSE;
			}
			lpDDraw->CreatePalette(DDPCAPS_8BIT,pe,&lpPal,NULL);
			lpPrimary->SetPalette(lpPal);
		
			lpSurface->GetDC(&hdc);
			BitBlt(hdc,0,0,bm.bmWidth,bm.bmHeight,hMemdc,0,0,SRCCOPY);
			lpSurface->ReleaseDC(hdc);

			DeleteDC(hMemdc);
			lpPal->Release();
			DeleteObject(hBmp);

}

LRESULT APIENTRY WndFunc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg){
	case WM_CREATE:

		break;

	case WM_KEYDOWN:
		switch(wParam){

			case VK_ESCAPE:

			lpDDraw->RestoreDisplayMode();

			lpPrimary->Release();
			
			lpScreen->Release();
			
			lpDDraw->Release();

			PostQuitMessage(0);
			return 0;
		}

		return 0;
	}
	return DefWindowProc(hwnd,msg,wParam,lParam);
}

int WINAPI WinMain(HINSTANCE hInst,HINSTANCE hPrev,LPSTR lpszCmdParam,int nCmdshow)
{
		MSG msg;
		DDSURFACEDESC Dds;
		DDSCAPS Ddscaps;
		
		HWND hwnd;
		WNDCLASS wc;
		char szAppName[]="Generic Game SDK Window";
		
		wc.style=CS_DBLCLKS;
		wc.lpfnWndProc=WndFunc;
		wc.cbClsExtra=0;
		wc.cbWndExtra=0;
		wc.hInstance=hInst;
		wc.hIcon=LoadIcon(NULL,IDI_APPLICATION);
		wc.hCursor=LoadCursor(NULL,IDC_ARROW);
		wc.hbrBackground=(HBRUSH)GetStockObject(WHITE_BRUSH);
		wc.lpszMenuName=NULL;
		wc.lpszClassName=szAppName;
		
		RegisterClass(&wc);
		
		hwnd=CreateWindowEx(
					WS_EX_TOPMOST,
					szAppName,
					"Direct X",
					WS_VISIBLE|WS_POPUP,
					CW_USEDEFAULT,
					CW_USEDEFAULT,
					640,480,
					NULL,NULL,hInst,
					NULL);
		
		if(!hwnd)return FALSE;

		ShowWindow(hwnd,nCmdshow);
		UpdateWindow(hwnd);
		SetFocus(hwnd);
		ShowCursor(FALSE);
		
		DirectDrawCreate(NULL,&lpDDraw,NULL);

		lpDDraw->SetCooperativeLevel(hwnd,DDSCL_EXCLUSIVE|DDSCL_FULLSCREEN);

		lpDDraw->SetDisplayMode(640,480,8);

		Dds.dwSize=sizeof(Dds);
		Dds.dwFlags=DDSD_CAPS|DDSD_BACKBUFFERCOUNT;
		Dds.ddsCaps.dwCaps=DDSCAPS_PRIMARYSURFACE|DDSCAPS_FLIP|DDSCAPS_COMPLEX;
		Dds.dwBackBufferCount=1;
		lpDDraw->CreateSurface(&Dds,&lpPrimary,NULL);


		Ddscaps.dwCaps=DDSCAPS_BACKBUFFER;
		lpPrimary->GetAttachedSurface(&Ddscaps,&lpBackbuffer);

		Dds.dwFlags=DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH;
		Dds.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN;
		Dds.dwWidth=640;
		Dds.dwHeight=480;
		lpDDraw->CreateSurface(&Dds,&lpScreen,NULL);

		LoadBMP(lpScreen,"test.BMP");
		
		while(1){
					if(PeekMessage(&msg,NULL,0,0,PM_NOREMOVE))
					{
							if(!GetMessage(&msg,NULL,0,0))
								return msg.wParam;
								TranslateMessage(&msg);
								DispatchMessage(&msg);
					}else{
							DrawFrame();
					}
		}
		return msg.wParam;
}