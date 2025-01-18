//gcc winavl.c -owinavl.exe -mwindows
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

typedef struct treenode TREE;
typedef TREE* pBiTree;
struct treenode{
	int data;
	TREE *left, *right;
	int height;
};
typedef struct {
    TREE** nodes;
    int size;
    int capacity;
} NodeList;
void initNodeList(NodeList* list) {
    list->nodes = NULL;
    list->size = 0;
    list->capacity = 0;
}
void addNode(NodeList* list, TREE* node) {
    if (list->size == list->capacity) {
        list->capacity = list->capacity == 0 ? 1 : list->capacity * 2;
        list->nodes = realloc(list->nodes, list->capacity * sizeof(TREE*));
    }
    list->nodes[list->size++] = node;
}
void freeNodeList(NodeList* list) {
    free(list->nodes);
    list->nodes = NULL;
    list->size = 0;
    list->capacity = 0;
}
void SearchTreeDup(TREE *T, int data, NodeList* list) {
    if (T == NULL) {
        return;
    }
    if (T->data == data) {
        addNode(list, T);
    }
    SearchTreeDup(T->left, data, list);
    SearchTreeDup(T->right, data, list);
}
int cmpfunc(const void *a, const void *b){
    return (*(int*)a - *(int*)b);
}
TREE* BuildTreeHelper(int *data, int start, int end){
    if(start > end){
        return NULL;
    }
    int mid = (start + end) / 2;
    TREE* node = (TREE*)malloc(sizeof(TREE));
    node->data = data[mid];
    node->left = BuildTreeHelper(data, start, mid - 1);
    node->right = BuildTreeHelper(data, mid + 1, end);
    return node;
}
void BuildTreeA(TREE **root,int *data,int num){
    if(num == 0){
        *root = NULL;
        return;
    }
    qsort(data, num, sizeof(int), cmpfunc);
    *root = BuildTreeHelper(data, 0, num - 1);
}
int getHeight(TREE* node){
    if(node == NULL){
        return 0;
    }
    return node->height;
}
void updateHeight(TREE* node){
    node->height = 1 + fmax(getHeight(node->left), getHeight(node->right));
}
TREE* rightRotate(TREE* y){
    TREE* x = y->left;
    TREE* T2 = x->right;
    x->right = y;
    y->left = T2;
    updateHeight(y);
    updateHeight(x);
    return x;
}
TREE* leftRotate(TREE* x){
    TREE* y = x->right;
    TREE* T2 = y->left;
    y->left = x;
    x->right = T2;
    updateHeight(x);
    updateHeight(y);
    return y;
}
int getBalance(TREE* node){
    if(node == NULL){
        return 0;
    }
    return getHeight(node->left) - getHeight(node->right);
}
TREE* insertNode(TREE* node, int data){
    if(node == NULL){
        TREE* newNode = (TREE*)malloc(sizeof(TREE));
        newNode->data = data;
        newNode->left = NULL;
        newNode->right = NULL;
        newNode->height = 1;
        return newNode;
    }
    if(data < node->data){
        node->left = insertNode(node->left, data);
    }else if(data > node->data){
        node->right = insertNode(node->right, data);
    }else{
        return node;
    }
    updateHeight(node);
    int balance = getBalance(node);
    if(balance > 1 && data < node->left->data){
        return rightRotate(node);
    }
    if(balance < -1 && data > node->right->data){
        return leftRotate(node);
    }
    if(balance > 1 && data > node->left->data){
        node->left = leftRotate(node->left);
        return rightRotate(node);
    }
    if(balance < -1 && data < node->right->data){
        node->right = rightRotate(node->right);
        return leftRotate(node);
    }
    return node;
}
void BuildAVLTree(TREE **root,int *data,int num){
    *root = NULL;
    for(int i = 0; i < num; i++){
        *root = insertNode(*root, data[i]);
    }
}
TREE* DeleteNode(TREE* root, int data){
    if(root == NULL){return root;}
    if(data < root->data){
        root->left = DeleteNode(root->left, data);
    }else if(data > root->data){
        root->right = DeleteNode(root->right, data);
    }else{
        if((root->left == NULL) || (root->right == NULL)){
            TREE* temp = root->left ? root->left : root->right;
            if(temp == NULL){
                temp = root;
                root = NULL;
            }else{
                *root = *temp;
            }
            free(temp);
        }else{
            TREE* temp = root->right;
            while(temp->left != NULL){
                temp = temp->left;
            }
            root->data = temp->data;
            root->right = DeleteNode(root->right, temp->data);
        }
    }
    if(root == NULL){
        return root;
    }
    updateHeight(root);
    int balance = getBalance(root);
    if(balance > 1 && getBalance(root->left) >= 0){
        return rightRotate(root);
    }
    if(balance > 1 && getBalance(root->left) < 0){
        root->left = leftRotate(root->left);
        return rightRotate(root);
    }
    if(balance < -1 && getBalance(root->right) <= 0){
        return leftRotate(root);
    }
    if(balance < -1 && getBalance(root->right) > 0){
        root->right = rightRotate(root->right);
        return leftRotate(root);
    }
    return root;
}
TREE* SearchTree(TREE *T, int data){
    if(T == NULL || T->data == data){
        return T;
    }
    if(data < T->data){
        return SearchTree(T->left, data);
    }else{
        return SearchTree(T->right, data);
    }
}
TREE* SearchTreeLast(TREE *T, int data) {
    TREE* result = NULL;
    while (T != NULL) {
        if (T->data == data) {
            result = T;
        }
        if (data < T->data) {
            T = T->left;
        } else {
            T = T->right;
        }
    }
    return result;
}
void inorderSearch(TREE *T, int data, NodeList* list) {
    if (T == NULL) {
        return;
    }
    inorderSearch(T->left, data, list);
    if (T->data == data) {
        addNode(list, T);
    }
    inorderSearch(T->right, data, list);
}
void DeleteTree(TREE *T){
	if(T==NULL){return;}
	DeleteTree(T->left);
	DeleteTree(T->right);	
	free(T);
}
int power_of_two(int n){
    if(n<=0)return 0;
    int power=0;
    while(n>0){
        n=n>>1;
        power++;
    }
    return power-1;
} 
void DisplayTree(HDC hdc,TREE* T,int pos){
    if(T==NULL)return;
    int h, y;
    if(pos==0)y=0;
    else{
        h=power_of_two(pos);
        y=1<<h;
    }
    int x=pos-y;
    int w=1024/y;
    char buf[10];

    if(h){        
        if(pos%2==0){
            MoveToEx(hdc,150+x*w+w/2,10+h*30,NULL);
            LineTo(hdc,150+x*w+w-4,10+h*30-20);
        }else{
            MoveToEx(hdc,150+x*w+w/2,10+h*30,NULL);
            LineTo(hdc,150+x*w+4,10+h*30-20);
        }
    }
    Ellipse(hdc, 150+x*w+w/2-7,10+h*30-1,150+x*w+w/2+10,10+h*30+13);
    sprintf(buf,"%d",T->data);
    TextOut(hdc,150+x*w+w/2-4,10+h*30,buf,strlen(buf));    
    DisplayTree(hdc,T->left,pos*2);
    DisplayTree(hdc,T->right,pos*2+1);
}
void ShowTree(TREE *T){
	if(T==NULL){return;}
	ShowTree(T->left);
	printf("%d ",T->data);
	ShowTree(T->right);
}
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow){
	static TCHAR szAppName[]=TEXT("HelloWin");
	HWND hwnd;
	MSG msg;
	WNDCLASS wndclass;
	wndclass.style=CS_HREDRAW|CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(NULL,IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL,IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName=NULL;
	wndclass.lpszClassName=szAppName;
	if(!RegisterClass(&wndclass)){
		MessageBox(NULL,TEXT("This program requires Windows NT!"),szAppName,MB_ICONERROR);
		return 0;
	}
	hwnd = CreateWindow(szAppName,TEXT("The Hello Program"),WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,NULL,NULL,hInstance, NULL);
	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);
	while(GetMessage(&msg,NULL,0,0)){
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam){
	HDC hdc;
	PAINTSTRUCT ps;
	// static RECT rect;
	HINSTANCE hInstance;
	static int *data, num;
	static TREE *root=NULL;
    static HPEN *hPen;
    static HFONT hFont;
	switch(message){
		case WM_CREATE:
			hInstance = ((LPCREATESTRUCT)lParam)->hInstance;
			srand((unsigned)time(NULL));
			num=100;
			data=malloc(num*sizeof(int));
			PostMessage(hwnd, WM_KEYDOWN,VK_RETURN,0);
            hPen=malloc(32*sizeof(HPEN));
            for(int i=0;i<32;i++){
                hPen[i]=CreatePen(PS_SOLID,1,RGB(8*i,0,0));
            }
            hFont=CreateFont(-8,-4,0,0,0,0,0,0,0,0,0,0,0,0);
			break;
		case WM_PAINT:
			hdc = BeginPaint( hwnd, &ps );
            SelectObject(hdc, hFont);
			for(int i=0;i<num;i++){
				char buf[10];
				sprintf(buf,"%d ",data[i]);
				TextOut(hdc, 10+i/20*20,10+i%20*15,buf,strlen(buf));
			}
            DisplayTree(hdc,root,1);
			EndPaint( hwnd, &ps );
			break;
		case WM_KEYDOWN:
			switch(wParam){
				case VK_ESCAPE:
					PostMessage(hwnd, WM_DESTROY,0,0);
					break;
				case VK_RETURN:
					for(int i=0;i<num;i++){
						data[i]=10+rand()%80;
					}
                    BuildAVLTree(&root, data,num);
					InvalidateRect(hwnd, NULL, TRUE);
					break;	
				case VK_F3:
					BuildAVLTree(&root, data,num);
					break;
				case VK_LEFT:

					InvalidateRect(hwnd, NULL, TRUE);
					break;
			}
			return 0;
		case WM_DESTROY:
            DeleteObject(hFont);
            for(int i=0;i<32;i++)DeleteObject(hPen[i]);
            free(hPen);
			if(data)free(data);
			PostQuitMessage(0);
			break;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
} 