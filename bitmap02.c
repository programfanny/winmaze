#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

typedef struct _step{
	short int x;
	short int y;
	}STEP;
typedef struct _dot{
	WORD x;
	WORD y;
	}DOT;
typedef struct _border{
	WORD width,height,len;
	DOT *path;
	}BORDER;
typedef struct _chain{
	WORD width,height,len;
	DOT start;
	BYTE *chain;
	}CHAIN;

void BorderChainToPath( BORDER *pBorder, CHAIN *pChain);
void BorderPathToChain( BORDER *pBorder, CHAIN *pChain);
void BorderToImage(BITMAP* pbmp, BYTE* pBits, BORDER* pBorder);
void CheckContinuous(HWND hwnd, BORDER *pBorder);	
void dispBorder(HWND, BORDER*);  //------------------------------------------------
void dispPath(HWND, int, int, STEP*, BYTE*);
void dispval(HWND, BITMAP*, BYTE*, int, int, int, int, int);
void FindBorder(HWND, BITMAP*, BYTE*, BORDER*);
void GetBits(BITMAP* pbmp, BYTE* pBits);
void GetBorder(BITMAP*, BYTE*, BYTE*);
void LoadBorderChain(HWND hwnd, LPSTR filepath, CHAIN* pChain);
void LoadBorderPath( HWND hwnd, LPSTR filepath, BORDER* pBorder);	
void SaveBorderChain(HWND hwnd, LPSTR filepath, CHAIN* pChain);
void SaveBorderPath( HWND hwnd, LPSTR filepath, BORDER* pBorder);
void ShowBits(HWND,BYTE*,int,int,int,int);
void Transform(BITMAP* pbmp, BYTE* pBits);

LRESULT CALLBACK WndProc (HWND, UINT, WPARAM, LPARAM);
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,PSTR szCmdLine, int iCmdShow){
	static TCHAR szAppName[] = TEXT ("HelloWin") ;
	HWND        hwnd ;
	MSG         msg ;
	WNDCLASS    wndclass ;

	wndclass.style         = CS_HREDRAW | CS_VREDRAW ;
	wndclass.lpfnWndProc   = WndProc ;
	wndclass.cbClsExtra    = 0 ;
	wndclass.cbWndExtra    = 0 ;
	wndclass.hInstance     = hInstance ;
	wndclass.hIcon         = LoadIcon (NULL, IDI_APPLICATION) ;
	wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW) ;
	wndclass.hbrBackground = (HBRUSH) GetStockObject (WHITE_BRUSH) ;
	wndclass.lpszMenuName  = NULL ;
	wndclass.lpszClassName = szAppName;

	if (!RegisterClass (&wndclass)) {
		MessageBox (NULL, TEXT ("This program requires Windows NT!"), szAppName, MB_ICONERROR) ;
		return 0 ;
		}
	hwnd = CreateWindow (szAppName, 	// window class name
		TEXT ("The Hello Program"), 	  // window caption
		WS_OVERLAPPEDWINDOW,          	// window style
		CW_USEDEFAULT,                	// initial x position
		CW_USEDEFAULT,                	// initial y position
		CW_USEDEFAULT,                	// initial x size
		CW_USEDEFAULT,                	// initial y size
		NULL,                         	// parent window handle
		NULL,                         	// window menu handle
		hInstance,                    	// program instance handle
		NULL) ;                       	// creation parameters
	 
	//ShowWindow (hwnd, iCmdShow) ;
	ShowWindow(hwnd, SW_MAXIMIZE);
	UpdateWindow (hwnd) ;
	 
	while (GetMessage (&msg, NULL, 0, 0)) {
		TranslateMessage (&msg) ;
		DispatchMessage (&msg);
		}
	return msg.wParam;
	}
LRESULT CALLBACK WndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam){
	PAINTSTRUCT ps ;
	HINSTANCE 	hInstance;
	HPEN hpen,hOldPen;
	char buf[100];
	BYTE *p,*q,*r;
	static HDC hdc,hdcmem;
	static HFONT hfont,hOldFont;
	static HBITMAP hbmp,hOldBitmap;
	static BITMAP bitmap;	
	static BYTE *pBits,*qBits;
	static RECT rect;
	static BORDER border,border2;
	static CHAIN chain;
	STEP shift8[]={{1,0},{1,1}, {0,1},{-1,1}, {-1,0},{-1,-1}, {0,-1},{1,-1}};
	int xPos,yPos,i,j,x,y,n;
	BYTE B,G,R,H;
	switch (message){
		case WM_CREATE:
			hInstance=((LPCREATESTRUCT) lParam)->hInstance;
			hfont= CreateFont(-8, -8, 0, 0, 400,FALSE, FALSE, FALSE,DEFAULT_CHARSET,OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY,FF_DONTCARE,"微软雅黑");
			hbmp=(HBITMAP)LoadImage(hInstance,"..\\\\img\\\\rooster.bmp",IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
			hdcmem=CreateCompatibleDC(hdc);
			SelectObject(hdcmem,hbmp);
			GetObject(hbmp,sizeof(BITMAP),&bitmap);
			bitmap.bmBits=(BYTE*)malloc(bitmap.bmWidthBytes * bitmap.bmHeight);
			GetBitmapBits(hbmp, bitmap.bmWidthBytes * bitmap.bmHeight, bitmap.bmBits);
			border.path=(DOT*)malloc(bitmap.bmWidth * bitmap.bmHeight*sizeof(STEP));
			
			pBits=(BYTE*)malloc(bitmap.bmWidth * bitmap.bmHeight);
			qBits=(BYTE*)malloc(bitmap.bmWidth * bitmap.bmHeight);

			rect.left=15;
			rect.top=0;
			border.len=0;
			rect.right=bitmap.bmWidth+30;
			rect.bottom=bitmap.bmHeight+10;
			return 0;
		case WM_PAINT:
			hdc=BeginPaint(hwnd,&ps);
			Rectangle(hdc,15,0,bitmap.bmWidth+30,bitmap.bmHeight+10);
			BitBlt(hdc,20,5,bitmap.bmWidth,bitmap.bmHeight,hdcmem,0,0,SRCCOPY);		

			sprintf(buf,"bmWidth:%5d, bmHeight:%5d",bitmap.bmWidth,bitmap.bmHeight);
			TextOut(hdc,50,580,buf,strlen(buf));

			sprintf(buf,"pBits = %p, qBits = %p",pBits,qBits);
			TextOut(hdc,350,580,buf,strlen(buf));			
			
			sprintf(buf,"path_len = %d",border.len);
			TextOut(hdc,835,0,buf,strlen(buf));	
			EndPaint(hwnd,&ps);
			return 0;
		case WM_KEYDOWN:
			switch(wParam){ 
				case VK_ESCAPE:
					PostMessage(hwnd,WM_DESTROY,0,0);
					break;
				case VK_RETURN:
					//void BorderToImage(BITMAP *pbmp,BYTE *pBits,BORDER *pBorder)
					BorderToImage(&bitmap,pBits,&border);
					Transform(&bitmap, pBits);
					SetBitmapBits(hbmp, bitmap.bmWidthBytes*bitmap.bmHeight, bitmap.bmBits);
					InvalidateRect(hwnd,NULL,TRUE);
					break;
				case VK_F2:
					break;
				case VK_F3:
					break;
				case VK_F4:
					break;
				case VK_SPACE:
					GetBits(&bitmap, pBits);
					SetBitmapBits(hbmp, bitmap.bmWidthBytes*bitmap.bmHeight, bitmap.bmBits);
					InvalidateRect(hwnd,NULL,TRUE);
					break;	
				case VK_F5:
					Transform(&bitmap, pBits);
					SetBitmapBits(hbmp, bitmap.bmWidthBytes*bitmap.bmHeight, bitmap.bmBits);
					InvalidateRect(hwnd,NULL,TRUE);
					break;
				case VK_F6:
					GetBorder(&bitmap, pBits, qBits);//, &border);
					Transform(&bitmap, qBits);
					SetBitmapBits(hbmp, bitmap.bmWidthBytes*bitmap.bmHeight, bitmap.bmBits);
					InvalidateRect(hwnd,&rect,FALSE);
					break;
				case VK_F7:
					FindBorder(hwnd, &bitmap, pBits, &border);
					SaveBorderPath(hwnd, "path01.dat",&border);
					//CopyMemory(pBits,qBits,bitmap.bmWidth*bitmap.bmHeight);
					Transform(&bitmap, pBits);
					SetBitmapBits(hbmp, bitmap.bmWidthBytes*bitmap.bmHeight, bitmap.bmBits);
					InvalidateRect(hwnd,&rect,TRUE);
					break;	
				case VK_F8:
					LoadBorderPath(hwnd,"path01.dat",&border2);
					CheckContinuous(hwnd,&border2);
					//free(border2.path);
					break;
				case VK_F9:
					BorderPathToChain(&border2,&chain);
					hdc=GetDC(hwnd);
					LoadBorderChain(hwnd,"pathChain.dat",&chain);
					ReleaseDC(hwnd,hdc);
					break;
			}
			return 0;
		case WM_LBUTTONDOWN:
			xPos = LOWORD(lParam)-20; 
			yPos = HIWORD(lParam)-5;
			dispval(hwnd,&bitmap,pBits,xPos,yPos,10,1150,120);
			dispval(hwnd,&bitmap,qBits,xPos,yPos,10,1150,360);
			return 0;
		case WM_DESTROY:
			if(qBits)free(qBits);
			if(pBits)free(pBits);
			if(border.path)free(border.path);
			if(border2.path)free(border2.path);
			if(chain.chain)free(chain.chain);
			if(bitmap.bmBits)free(bitmap.bmBits);
			PostQuitMessage (0);
			return 0;
		}
	return DefWindowProc (hwnd, message, wParam, lParam);
	}
void ShowBits(HWND hwnd,BYTE *pBits,int width,int height,int xpos,int ypos){
	int i,j,val;
	char buf[80];
	HDC hdc;
	HFONT hFont,hOldFont;
	HBRUSH hBrush[7],hOldBrush;
	COLORREF color[] = {
		RGB(222,222,222),
		RGB(192,190,217),
		RGB(38,196,177),
		RGB(72,196,38),
		RGB(196,196,38),
		RGB(193,119,13),
		RGB(152,13,193)
		};
	for(i = 0;i<7;i++)hBrush[i] = CreateSolidBrush(color[i]);
	hFont = CreateFont(-8,-4,0,0,400,0,0,0,0,0,0,0,0,"Arial");
	hdc = GetDC(hwnd);
	hOldFont = SelectObject(hdc,hFont);
	hOldBrush = SelectObject(hdc,hBrush[0]);
	for(j = 0;j<height;j++)
	{
		for(i = 0;i<width;i++)
		{
			val = pBits[(j*width+i)*4];
			SelectObject(hdc,hBrush[val % 6]);
			Rectangle(hdc,xpos+10+i*10,ypos+10+j*10,xpos+21+i*10,ypos+21+j*10);
			SetBkColor(hdc,color[val%6]);
			SetTextColor(hdc,5-color[val%6]);
			sprintf(buf,"%02X",val&0x3F);
			TextOut(hdc,xpos+11+i*10,ypos+11+j*10,buf,strlen(buf));
		}
	}
	SelectObject(hdc,hOldFont);
	SelectObject(hdc,hOldBrush);
	for(i = 0;i<7;i++)DeleteObject(hBrush[i]);
	DeleteObject(hFont);
	ReleaseDC(hwnd,hdc);
	}

void BorderChainToPath( BORDER *pBorder, CHAIN *pChain ){
	int i,j,k,x,y;
	STEP pt;
	//STEP shift4[]={{1,0}, {0,1}, {-1,0}, {0,-1}};
	STEP shift8[]={{1,0},{1,1}, {0,1},{-1,1}, {-1,0},{-1,-1}, {0,-1},{1,-1}};
	pBorder->width = pChain->width ;
	pBorder->height = pChain->height ;
	pBorder->len = pChain->len + 1 ;	
	pBorder->path = (DOT*)malloc(pBorder->len);
	pBorder->path[0] = pChain->start ;
	for(i=0;i<pChain->len;i++){
		k = pChain->chain[i];
		pt.x=pBorder->path[i+1].x + shift8[k].x;
		pt.y=pBorder->path[i+1].y + shift8[k].y;
	}
	}
void BorderPathToChain( BORDER *pBorder,CHAIN *pChain){
	int i,j,k,x,y;
	STEP pt;
	//STEP shift4[]={{1,0}, {0,1}, {-1,0}, {0,-1}};
	STEP shift8[]={{1,0},{1,1}, {0,1},{-1,1}, {-1,0},{-1,-1}, {0,-1},{1,-1}};
	pChain->width = pBorder->width;
	pChain->height = pBorder->height;
	pChain->len = pBorder->len-1;
	pChain->start = pBorder->path[0];
	pChain->chain=malloc(pBorder->len);
	for(i=0;i<pChain->len;i++){
		pt.x=pBorder->path[i+1].x-pBorder->path[i].x;
		pt.y=pBorder->path[i+1].y-pBorder->path[i].y;
		for(k=0;k<8;k++){
			//if( pt == shift8[k] ) break; 
			if( pt.x == shift8[k].x && pt.y == shift8[k].y )break;
		}
		pChain->chain[i]=k;
	}
	}
void BorderToImage(BITMAP *pbmp,BYTE *pBits,BORDER *pBorder){
	int i,x,y;
	int width=pbmp->bmWidth;
	int height=pbmp->bmHeight;
	ZeroMemory(pBits,width*height);
	for(i=0;i<pBorder->len;i++){
		x=pBorder->path[i].x;
		y=pBorder->path[i].y;
		pBits[y*width+x]=16;
	}
	}
void CheckContinuous(HWND hwnd,BORDER* pBorder){
	int i;
	char buf[100];
	BOOL clif = TRUE;
	HDC hdc=GetDC(hwnd);

	for(i=1;i<pBorder->len;i++){
		if((abs((int)pBorder->path[i-1].x - (int)pBorder->path[i].x) > 1)
		 && (abs((int)pBorder->path[i-1].y - (int)pBorder->path[i].y) > 1)){
			sprintf(buf,"path[%4d]<->path[%4d]:\n (%3d , %3d) <--> (%3d , %3d)  ", i-1, i, 
				pBorder->path[i-1].x,pBorder->path[i-1].y,pBorder->path[i].x,pBorder->path[i].y);
			MessageBox(NULL,buf,"not continuous",MB_OK);
			clif=FALSE;
		}
	}
	sprintf(buf,"Border : (width=%04x,height=%04x),path-len=%04x",pBorder->width,pBorder->height,pBorder->len);
	TextOut(hdc,900,50,buf,strlen(buf));
	for(i=0;i<10;i++){
		sprintf(buf,"P%d : (%04x,%04x)=(%04d,%04d)",i,pBorder->path[i].x,pBorder->path[i].y,pBorder->path[i].x,pBorder->path[i].y);
		TextOut(hdc,900,100+i*20,buf,strlen(buf));
	}
	if(clif){ MessageBox(hwnd,"The border is Continuous!","Result",MB_OK); }
	else{ MessageBox(hwnd,"The border is NOT Continuous!","Result",MB_OK); }
	ReleaseDC(hwnd,hdc);
	}

void dispBorder(HWND, BORDER*){}  //------------------------------------------------------------------

void dispPath(HWND hwnd, int width,int len, STEP *path,BYTE* pBits){
	int i;
	char buf[80];
	HANDLE hFile=CreateFile("roosterPath.dat",GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ,
                              NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL,NULL);
	if(hFile==NULL)MessageBox(NULL,"Error to Create File","Error",MB_OK);
	HDC hdc=GetDC(hwnd);
	for(i=0;i<len;i++){
		sprintf(buf,"[%d,%d]=%d",path[i+550].x,path[i+550].y,pBits[path[i+550].y*width+path[i+550].x]);
		TextOut(hdc,835+i/30*100,30+i%30*20,buf,strlen(buf));
		SetPixel(hdc,path[i+550].x+20,path[i+550].y+5,RGB(0,0,255));
	}
	ReleaseDC(hwnd,hdc);
	}	
void dispval(HWND hwnd,BITMAP *pbmp,BYTE* pBits,int xPos,int yPos,int n,int posx,int posy){
	HFONT hFont,hOldFont;
	int i,j,k,x,y,width,height,val;
	char buf[80];
	if(pBits==NULL)return;
	HDC hdc = GetDC(hwnd);
	COLORREF color[] = {
		RGB(222,222,222),
		RGB(192,190,217),
		RGB(38,196,177),
		RGB(72,196,38),
		RGB(196,196,38),
		RGB(193,119,13),
		RGB(152,13,193)
		};
	width = pbmp->bmWidth;
	height = pbmp->bmHeight;
	sprintf(buf," %8p  [ %3d , %3d ] ",pBits,xPos,yPos);
	TextOut(hdc,posx-100,posy-120,buf,strlen(buf));
	hFont= CreateFont(-8, -4, 0, 0, 400,FALSE, FALSE, FALSE,
		DEFAULT_CHARSET,OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS,
		DEFAULT_QUALITY,FF_DONTCARE,"Arial");
	hOldFont=SelectObject(hdc,hFont);
	//SetTextColor(hdc,RGB(0,0,0));
	for(j = -n; j <= n; j++){ 
		y=yPos+j;
		for(i = -n; i <= n; i++){ 
			x=xPos+i;
			if((x>=0) && (x<width) && (y>=0) && (y<height)){
				val=pBits[y*width+x];
				sprintf(buf,"%02x",val);
				SetBkColor(hdc,RGB(128+val%7*31,128,128));
				SetTextColor(hdc,RGB(255-val*80,val*80+128,255));
			}else{sprintf(buf,"  "); SetBkColor(hdc,RGB(192,192,192));}
				TextOut(hdc,posx+1+i*10,posy+1+j*10,buf,strlen(buf));
		}
	}
	Ellipse(hdc,posx-3,posy-3,posx+3,posy+3);
	MoveToEx(hdc,posx,posy+10,NULL);
	LineTo(hdc,posx,posy);
	LineTo(hdc,posx+10,posy);
	LineTo(hdc,posx+10,posy+10);
	LineTo(hdc,posx,posy+10);	
	SelectObject(hdc,hOldFont);
	DeleteObject(hFont);
	ReleaseDC(hwnd,hdc);
	}
void FindBorder(HWND hwnd, BITMAP* pbmp, BYTE* pBits, BORDER* pBorder){
	int s,t,i,j,k,x,y,len;
	BOOL stop;
	int width, height;
	STEP round[8];
	POINT pt;
	BYTE *pA;
	width=pbmp->bmWidth;
	height=pbmp->bmHeight;
	pBorder->width=(WORD)width;
	pBorder->height=(WORD)height;
	pA=(BYTE*)malloc(width*height);
	ZeroMemory(pA,width*height);
	STEP shift8[]={{1,0},{1,1}, {0,1},{-1,1}, {-1,0},{-1,-1}, {0,-1},{1,-1}};
	for(j=1;j<height-1;j++){
		for(i=1;i<width-1;i++){
			s=j*width+i;
			for(k=0;k<8;k++){
				x=i+shift8[k].x;
				y=j+shift8[k].y;
				t=y*width+x;
				if(abs(pBits[t]-pBits[s])==1)pA[s]=pBits[s] && 1;
			}
		}
	}
	len=s=0;
	while(pA[s]==0){s++;}
	pA[s]=100; i=s%width; j=s/width;
	pBorder->path[len].x=i; pBorder->path[len++].y=j;
	stop=FALSE;
	do{
		s=0;
		for(k=0;k<8;k++){
			x=i+shift8[k].x;
			y=j+shift8[k].y;
			t=y*width+x;
			s+=pA[t];
			if(pA[t]==1){
				break;
				}
			if(pA[t]==10){s-=pA[t];}
			}
		if( s>0 ){
			pBorder->path[len].x=x; pBorder->path[len++].y=y;
			if(pA[j*width+i]!=100)pA[j*width+i]=10;
			i=x; j=y;
		} else { pA[j*width+i]=10; len--;
			i=pBorder->path[len].x; j=pBorder->path[len].y;}
	}while(!((pBorder->path[0].x==i) && (pBorder->path[0].y==j)));
	pBorder->len=len;
	CopyMemory(pBits, pA, width*height);
	free(pA);
	}
void GetBits(BITMAP* pbmp, BYTE* pBits){
	int x,y;
	BYTE *p,*q,*r;
	p=pBits;q=pbmp->bmBits;
	for(y=0;y<pbmp->bmHeight;y++){
		q=pbmp->bmBits+pbmp->bmWidthBytes*y;
		for(x=0;x<pbmp->bmWidth;x++,p++){
			if(q[0]>128 && q[1]>128 && q[2]>128){q[0]=0;q[1]=0;q[2]=255;q[3]=0;*p=0;}
			else{q[0]=0;q[1]=255; q[2]=0;q[3]=0;*p=1;}
			q=q+4;
		}
	}
	}
void GetBorder(BITMAP* pbmp, BYTE* pBits, BYTE* qBits){ 
	int width,height;
	int s,t,i=1,j=1,k,x,y; 
	BYTE *p,*q,*r,tmp;	
	width=pbmp->bmWidth;
	height=pbmp->bmHeight;
	CopyMemory(qBits,pBits,width*height);
	STEP shift4[]={{1,0}, {0,1}, {-1,0}, {0,-1}};
	i=1;j=1;
	for(j=1;j<height-1;j++){
		for(i=1;i<width-1;i++){
			tmp=qBits[j*width+i];
			for(k=0;k<4;k++){
				x=i+shift4[k].x;
				y=j+shift4[k].y;
				tmp+=pBits[y*width+x];
				}
			qBits[j*width+i]=(tmp==0||tmp==5)?0:1;
		}
	}	
	}
void LoadBorderChain(HWND hwnd,LPSTR filepath,CHAIN* pChain){
	char buf[80];
	DWORD NumberOfBytesRead=0;
	HANDLE hFile=CreateFile(filepath,GENERIC_READ,FILE_SHARE_READ,NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL,NULL);
	ReadFile(hFile,pChain,sizeof(WORD)*5,NULL,0);
	HDC hdc=GetDC(hwnd);
	sprintf(buf,"hFile = %p chain-len = %d", hFile, pChain->len);
	TextOut(hdc,900,30,buf,strlen(buf));
	pChain->chain=(BYTE*)malloc(pChain->len);
	ReadFile(hFile,pChain->chain, pChain->len, &NumberOfBytesRead,0);
	CloseHandle(hFile);
	ReleaseDC(hwnd,hdc);
	}	
void LoadBorderPath(HWND hwnd, LPSTR filepath, BORDER* pBorder){
	char buf[80];
	DWORD NumberOfBytesRead=0;
	HDC hdc=GetDC(hwnd);
	HANDLE hFile=CreateFile(filepath,GENERIC_READ,FILE_SHARE_READ,NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL,NULL);
	ReadFile(hFile,pBorder,sizeof(WORD)*3,&NumberOfBytesRead,0);
	pBorder->path = (DOT*)malloc(pBorder->width * pBorder->height*sizeof(STEP));
	ReadFile(hFile,pBorder->path,sizeof(DOT)*pBorder->len,&NumberOfBytesRead,0);
	CloseHandle(hFile);
	ReleaseDC(hwnd,hdc);
	}
void SAVEBorderChain(HWND hwnd,LPSTR filepath,CHAIN *pChain){
	char buf[80];
	DWORD NumberOfBytesWrite=0;
	HANDLE hFile=CreateFile(filepath,GENERIC_WRITE,FILE_SHARE_READ,NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL,NULL);
	HDC hdc=GetDC(hwnd);	
	sprintf(buf,"hFile = %p chain-len = %d", hFile, pChain->len);
	TextOut(hdc,900,30,buf,strlen(buf));
	WriteFile(hFile,pChain,sizeof(WORD)*5,NULL,0);
	WriteFile(hFile,pChain->chain,pChain->len,&NumberOfBytesWrite,0);
	CloseHandle(hFile);
	ReleaseDC(hwnd,hdc);
	}
void SaveBorderPath(HWND hwnd,LPSTR filepath,BORDER* pBorder){
	char buf[80];
	DWORD NumberOfBytesWrite;
	HANDLE hFile=CreateFile(filepath,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ,NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL,NULL);
	HDC hdc=GetDC(hwnd);
	sprintf(buf,"hFile = %p path-len = %d", hFile, pBorder->len);
	TextOut(hdc,900,30,buf,strlen(buf));
	ReleaseDC(hwnd,hdc);
	WriteFile(hFile,pBorder,sizeof(WORD)*3,NULL,0);
	WriteFile(hFile,pBorder->path,sizeof(DOT)*pBorder->len,NULL,0);
	CloseHandle(hFile);
	}
void Transform(BITMAP* pbmp, BYTE* pBits){
	int x,y;
	BYTE *p,*q,*r;
	p=pBits;q=pbmp->bmBits;
	for(y=0;y<pbmp->bmHeight;y++){
		q=pbmp->bmBits+pbmp->bmWidthBytes*y;
		for(x=0;x<pbmp->bmWidth;x++,p++){
			if(*p){q[0]=*p*50;q[1]=0;q[2]=128+*p*5;q[3]=0;}else{q[0]=255;q[1]=255;q[2]=255;q[3]=0;}
			q=q+4;
		}
	}
	}
