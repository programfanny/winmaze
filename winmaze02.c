#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <memory.h>

#define SIZE 15
#define MAX_SIZE 2*SIZE+1

#define WALL 1
#define ROUTE 0

typedef struct _step
{
	BYTE x;
	BYTE y;
}STEP;

typedef struct _maze
{
	WORD size;
	BYTE *data;
	STEP entrance;
	STEP exit;
	STEP *path;
}MAZE;

STEP path[MAX_SIZE * MAX_SIZE];
int path_len;
WORD Total,Current;

static STEP s_shift[]={{1,0}, {0,1}, {-1,0}, {0,-1}};

//void InitMaze(MAZE *maze);
BOOL OK(MAZE* maze, int x, int y){
	BOOL res=TRUE;
	if(maze->data[y*maze->size+x]==1)res=FALSE;
	return res;
}
void CarveMaze(MAZE*, int, int, int x, int y);
void GenerateMaze(MAZE*, int, int);
BOOL ReadMaze(LPSTR, MAZE*, int);
BOOL SaveMaze(LPSTR, MAZE*);
void ShowMaze(HWND, MAZE*,int,int);
void markPos(HWND,int x,int y,int j,int path_len);
void ShowMazeData(HWND,MAZE*,int,int);
int search_path(HWND hwnd,MAZE *maze,STEP path[],int path_len);
BOOL inPath(MAZE *maze,STEP newPos,STEP path[],int path_len);
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
	wndclass.lpszClassName = szAppName ;

	if (!RegisterClass (&wndclass)) {
		MessageBox (NULL, TEXT ("This program requires Windows NT!"), szAppName, MB_ICONERROR) ;
		return 0 ;
		}
	hwnd = CreateWindow (szAppName, 	// window class name
		TEXT ("The Hello Program"), 	// window caption
 		WS_OVERLAPPEDWINDOW,        	// window style
		CW_USEDEFAULT,              	// initial x position
		CW_USEDEFAULT,              	// initial y position
		CW_USEDEFAULT,              	// initial x size
		CW_USEDEFAULT,              	// initial y size
		NULL,                       	// parent window handle
		NULL,                       	// window menu handle
		hInstance,                  	// program instance handle
		NULL) ;                     	// creation parameters
     
	ShowWindow (hwnd, iCmdShow) ;
  	UpdateWindow (hwnd) ;
     
	while (GetMessage (&msg, NULL, 0, 0)) {
  		TranslateMessage (&msg) ;
  		DispatchMessage (&msg);
		}
	return msg.wParam;
	}

LRESULT CALLBACK WndProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam){
	HDC         hdc ;
	PAINTSTRUCT ps ;
	HINSTANCE 	hInstance;
	static HDC 	mdc;
	static HBITMAP hbmp;
	HPEN hPen;
	static BITMAP bmp;
	HFONT hFont,hOldFont;
	int i,j,k;
	static int posx,posy;
	LPSTR szFilePath="maze.dat";
	static int step_count;
	char buf[80];
	static MAZE maze;

	switch (message){
		case WM_CREATE:
			hInstance = ((LPCREATESTRUCT)lParam)->hInstance;
			srand((unsigned)time(NULL));
			maze.data=(BYTE*)malloc(MAX_SIZE*MAX_SIZE);
			maze.size=19;
			GenerateMaze(&maze,maze.size,maze.size);
			posx=maze.entrance.x;
			posy=maze.entrance.y;
			path[0]=maze.entrance;
			step_count=0;
			path_len=0;
			posx=0; posy=1;
			return 0;
		case WM_PAINT:
			hdc=BeginPaint(hwnd,&ps);
			ShowMaze(hwnd,&maze,0,0);
			ShowMazeData(hwnd,&maze,0,405);
			Ellipse(hdc, 30+posx*20-8, 30+posy*20-8, 30+posx*20+8, 30+posy*20+8);
			EndPaint(hwnd, &ps);
			return 0;
		case WM_KEYDOWN:
			switch(wParam)
			{
				case VK_ESCAPE:
					PostMessage(hwnd,WM_DESTROY,0,0);
				break;
				case VK_SPACE:
					GenerateMaze(&maze,maze.size,maze.size);
					posx=maze.entrance.x;
					posy=maze.entrance.y;
					InvalidateRect(hwnd,NULL,TRUE);
				break;
				case VK_F3:
					if(SaveMaze(szFilePath,&maze))
					{
						hdc=GetDC(hwnd);
						TextOut(hdc,500,500,"Maze saved in file",18);
						ReleaseDC(hwnd,hdc);
					};
				break;
				case VK_F4:
					if(ReadMaze(szFilePath,&maze,0))
					{
						ShowMaze(hwnd,&maze,500,0);
					}
				break;	
				case VK_RETURN:
					path[0] = maze.entrance;
					//path[0].y=maze.entrance.y;
					step_count=search_path(hwnd,&maze,path, path_len);
					//ShowMaze(HWND, MAZE*,int,int);
					//ShowMaze(hwnd,&maze,500,0);
					hdc=GetDC(hwnd);
					for(k=0;k<step_count;k++)
					{
						sprintf(buf,"(%d,%d) ",path[k].x,path[k].y);
						TextOut(hdc,450+k/16*60,40+k%16*20,buf,strlen(buf));
						Ellipse(hdc,25+path[k].x*20,25+path[k].y*20,35+path[k].x*20,35+path[k].y*20);
					}
					hPen=CreatePen(PS_SOLID,3,RGB(255,128,255));
					SelectObject(hdc,hPen);
					k=0;
					MoveToEx(hdc,30+path[k].x*20,30+path[k].y*20,NULL);
					for(k=0;k<step_count;k++)
					{
						LineTo(hdc,30+path[k].x*20,30+path[k].y*20);
					}
					sprintf(buf,"step_count = %d",step_count);
					TextOut(hdc,150,0,buf,strlen(buf));
					DeleteObject(hPen);
					ReleaseDC(hwnd,hdc);
				break;
				case VK_UP:
					if(OK(&maze,posx,posy-1)){
						posy--;if(posy<0)posy=maze.size-1;
						InvalidateRect(hwnd, NULL, TRUE);
					}
				break;
				case VK_DOWN:
					if(OK(&maze,posx,posy+1)){
						posy++;if(posy==maze.size)posy=0;
						InvalidateRect(hwnd, NULL, TRUE);
					}				
				break;
				case VK_LEFT:
					if(OK(&maze,posx-1,posy)){
						posx--;if(posx<0)posx=maze.size-1;
						InvalidateRect(hwnd, NULL, TRUE);
					}				
				break;
				case VK_RIGHT:
					if(OK(&maze,posx+1,posy)){
						posx++;if(posx==maze.size)posx=0;
						InvalidateRect(hwnd, NULL, TRUE);
					}					
				break;												
			}
			return 0;	
		case WM_DESTROY:
			if(maze.data)free(maze.data);
			PostQuitMessage (0);
			return 0;
  		}
	return DefWindowProc (hwnd, message, wParam, lParam);
	}

void ShowMaze(HWND hwnd,MAZE *maze,int xpos,int ypos){
	COLORREF color[]={
		//RGB(255,255,255),
		//RGB(255,255,255),
		RGB(128,255,0),
		RGB(128,192,128),
		RGB(255,128,128),
		RGB(0,0,0),
		RGB(255,255,0)
	};
	HBRUSH hBrush[5],hOldBrush;
	HFONT hFont,hOldFont;
	int i,j;
	char buf[80];
	HDC hdc=GetDC(hwnd);
	for(i=0;i<5;i++){
		hBrush[i]=CreateSolidBrush(color[i]);
	}
	hFont = CreateFont(-8, -4, 0, 0, 300 , FALSE, FALSE, FALSE, 
		DEFAULT_CHARSET, OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, 
		DEFAULT_QUALITY, FF_DONTCARE, "Arial");
	hOldFont=SelectObject(hdc,hFont);
	hOldBrush=SelectObject(hdc,hBrush[0]);
	for(int i=0;i<maze->size;i++){
		sprintf(buf,"%d",i);
		TextOut(hdc,xpos+25+i*20,ypos+10,buf,strlen(buf));
	}
	for(int i=0;i<maze->size;i++){
		sprintf(buf,"%d",i);
		TextOut(hdc,xpos+10,ypos+25+i*20,buf,strlen(buf));
	}
	for(j=0;j<maze->size;j++){
		for(i=0;i<maze->size;i++){
			SelectObject(hdc,hBrush[maze->data[j*maze->size+i]]);
			Rectangle(hdc,xpos+20+i*20,ypos+20+j*20,xpos+41+i*20,ypos+41+j*20);
			//SetBkColor(hdc,RGB(255,255,255));
			SetBkColor(hdc,color[maze->data[j*maze->size+i]]);
			//SetTextColor(hdc,RGB(0,0,255));
			SetTextColor(hdc,color[5-maze->data[j*maze->size+i]]);
			sprintf(buf,"%X",maze->data[j*maze->size+i]);
			TextOut(hdc,xpos+28+i*20,ypos+26+j*20,buf,strlen(buf));
		}
	}
	SelectObject(hdc,hOldFont);
	SelectObject(hdc,hOldBrush);
	for(i=0;i<5;i++){
		DeleteObject(hBrush[i]);
	}
	DeleteObject(hFont);
	ReleaseDC(hwnd,hdc);
	}
BOOL SaveMaze(LPSTR szFilePath,MAZE *maze){
	HANDLE hFile;
	DWORD dwWriteDataSize;
	BOOL retValue=TRUE;
	hFile=CreateFile(szFilePath,GENERIC_WRITE|GENERIC_READ,0,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	if(hFile==INVALID_HANDLE_VALUE)
	{
		retValue = FALSE;
	}else{
		SetFilePointer(hFile,0,NULL,FILE_BEGIN);
		ReadFile(hFile,&Total,sizeof(WORD),&dwWriteDataSize,NULL);
		Total=Total+1;
		SetFilePointer(hFile,0,NULL,FILE_BEGIN);
		WriteFile(hFile,&Total,sizeof(WORD),&dwWriteDataSize,NULL);
		SetFilePointer(hFile,0,NULL,FILE_END);
		WriteFile(hFile,&(maze->size),sizeof(WORD),&dwWriteDataSize,NULL);
		WriteFile(hFile,maze->data,maze->size*maze->size,&dwWriteDataSize,NULL);
		CloseHandle(hFile);
	}
	return retValue;
	}	
BOOL ReadMaze(LPSTR szFilePath,MAZE *maze,int num){
	HANDLE hFile;
	DWORD dwReadDataSize;
	WORD size;
	int i;
	BYTE *data;
	BOOL retValue=TRUE;
	hFile=CreateFile(szFilePath,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if(hFile==INVALID_HANDLE_VALUE)
	{
		retValue = FALSE;
	}else{
		//SetFilePointer(hFile,0,NULL,FILE_BEGIN);
		ReadFile(hFile,&Total,sizeof(WORD),&dwReadDataSize,NULL);
		for(i=0;i<num-1;i++){
			ReadFile(hFile,&size,sizeof(WORD),&dwReadDataSize,NULL);
			SetFilePointer(hFile,size*size,NULL,FILE_CURRENT);
		}
		ReadFile(hFile,&size,sizeof(WORD),&dwReadDataSize,NULL);
		data=(BYTE*)malloc(size*size);
		ReadFile(hFile,&data,size*size,&dwReadDataSize,NULL);

		CloseHandle(hFile);
	}
	return retValue;
	}	
void ShowMazeData(HWND hwnd,MAZE *maze,int xpos,int ypos){
	HDC hdc;
	char buf[80];
	hdc=GetDC(hwnd);
	sprintf(buf,"      maze size : %d",maze->size);
	TextOut(hdc,xpos,ypos,buf,strlen(buf));
	sprintf(buf,"maze Data Point : %p",maze->data);
	TextOut(hdc,xpos,ypos+20,buf,strlen(buf));
	sprintf(buf,"  maze entrance : (%d,%d)",maze->entrance.x,maze->entrance.y);
	TextOut(hdc,xpos,ypos+40,buf,strlen(buf));
	sprintf(buf,"      maze exit : (%d,%d)",maze->exit.x,maze->exit.y);
	TextOut(hdc,xpos,ypos+60,buf,strlen(buf));
	ReleaseDC(hwnd,hdc);
	}
/*  Carve the maze starting at x, y. */
void CarveMaze(MAZE *pMaze, int width, int height, int x, int y) {
   int x1, y1;
   int x2, y2;
   int dx, dy;
   int dir, count;
   BYTE* maze=pMaze->data;

   dir = rand() % 4;
   count = 0;
   while(count < 4) {
      dx = 0; dy = 0;
      switch(dir) {
      case 0:  dx = 1;  break;
      case 1:  dy = 1;  break;
      case 2:  dx = -1; break;
      default: dy = -1; break;
      }
      x1 = x + dx;
      y1 = y + dy;
      x2 = x1 + dx;
      y2 = y1 + dy;
      if(   x2 > 0 && x2 < width && y2 > 0 && y2 < height
         && maze[y1 * width + x1] == 1 && maze[y2 * width + x2] == 1) {
         maze[y1 * width + x1] = 0;
         maze[y2 * width + x2] = 0;
         x = x2; y = y2;
         dir = rand() % 4;
         count = 0;
      } else {
         dir = (dir + 1) % 4;
         count += 1;
      }
   }
}
/* Generate maze in matrix maze with size width, height. */
void GenerateMaze(MAZE *pMaze, int width, int height) {
   int x, y;
   BYTE* maze=pMaze->data;
   /* Initialize the maze. */
   for(x = 0; x < width * height; x++) {
      maze[x] = 1;
   }
   maze[1 * width + 1] = 0;
   /* Seed the random number generator. */
   srand(time(0));
   /* Carve the maze. */
   for(y = 1; y < height; y += 2) {
      for(x = 1; x < width; x += 2) {
         CarveMaze(pMaze, width, height, x, y);
      }
   }
   /* Set up the entry and exit. */
   	pMaze->entrance.x=0;
	pMaze->entrance.y=1;
   	pMaze->exit.x=width - 1;
	pMaze->exit.y=height - 2;
	maze[width] = 0;
	maze[(height - 1) * width - 1] = 0;
}

int search_path(HWND hwnd,MAZE *maze,STEP path[],int path_len)
{
	int j=0;
	while (1)
	{		
		int i,bFind;
		STEP newPos;
		ShowMaze(hwnd,maze,0,0);
		for (bFind=0,i=0;i<4;i++)
		{
			newPos.x = path[path_len-1].x + s_shift[i].x; 
			newPos.y = path[path_len-1].y + s_shift[i].y;
			markPos(hwnd,newPos.x,newPos.y,j,path_len);
			Sleep(200);
			if ( path_len==1 )
			{
				if ( maze->data[newPos.y*maze->size+newPos.x]==0)
				{
					bFind=1; break; 
				} 
			}
			else if ( !inPath(maze,newPos,path,path_len) && maze->data[newPos.y*maze->size+newPos.x]==0)
			{
				bFind=1; break;
			}
		}
		ShowMaze(hwnd,maze,0,0);
		Sleep(100);
		j++;

		if (bFind) 
		{
			path[path_len++]=newPos;
			if ( newPos.x==maze->exit.x && newPos.y==maze->exit.y)
			{				
				return path_len;
			}
		}
		else
		{
			maze->data[path[path_len-1].y*maze->size+path[path_len-1].x]=2;
			path_len--;
			if (path_len==0) 
			{				
				return path_len;
			}
		}
	}
}
BOOL inPath(MAZE *maze,STEP newPos,STEP path[],int path_len)
{
	BOOL retValue=FALSE;
	int i;
	for(i=0;i<path_len;i++){
		if((maze->data[newPos.y*maze->size+newPos.x]>0) || ((newPos.x == path[i].x) && (newPos.y==path[i].y)))
		{
			retValue=TRUE;
			break;
		}		
	}
	return retValue;
}
void markPos(HWND hwnd,int x,int y,int j,int path_len)
{
	HDC hdc;
	hdc=GetDC(hwnd);
	Ellipse(hdc,25+x*20,25+y*20,35+x*20,35+y*20);
	Ellipse(hdc,23+path[path_len-1].x*20,23+path[path_len-1].y*20,37+path[path_len-1].x*20,37+path[path_len-1].y*20);
	ReleaseDC(hwnd,hdc);
}
