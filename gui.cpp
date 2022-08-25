/**
   TODO
   1. Create a window with minimal features 
   2. Create a buffer struct offscreen_buffer
   3. Create functions that modify the buffer
**/

#include <windows.h>
#include <dwmapi.h>
#include <math.h>
#include "gui.h"
#include "tree.cpp"

//Global variables
global_variable bool Running = true;
global_variable offscreen_buffer BackBuffer;
global_variable Node * Tree;
global_variable game_state global_game;


//Function definations
//TODO: move it into a defines.h

LRESULT CALLBACK WindowCallback(HWND Window, UINT Message,
				WPARAM WParam, LPARAM LParam);

void RenderGame(offscreen_buffer * Buffer, game_state * GameState);

void DrawGrid(offscreen_buffer * Buffer, game_state * GameState);

u32 DenormalizeColor(f32 R, f32 G, f32 B);

void ProcessWindowMessages();

void ResizeBuffer(offscreen_buffer * Buffer, int Width, int Height);

void DisplayBufferInWindow(offscreen_buffer *Buffer, HDC DeviceContext);

void DisplayTree(offscreen_buffer *Buffer, Node * tree);

void PaintBuffer(offscreen_buffer * Buffer, f32 R, f32 G, f32 B);


int CALLBACK WinMain(HINSTANCE Instance, HINSTANCE PrevInstance,
		     LPSTR CommandLine, int ShowCode){
    
  //Window init
  WNDCLASS WindowClass = {};
  WindowClass.style = CS_HREDRAW | CS_VREDRAW;
  WindowClass.lpfnWndProc = WindowCallback;
  WindowClass.hInstance = Instance;
  WindowClass.lpszClassName = "Practice";
    
  RegisterClass(&WindowClass);
    
  HWND WindowHandle = CreateWindowEx(0,
				     WindowClass.lpszClassName,
				     "Playing around",
				     WS_OVERLAPPEDWINDOW | WS_VISIBLE,
				     CW_USEDEFAULT, CW_USEDEFAULT,
				     CW_USEDEFAULT, CW_USEDEFAULT,
				     0, 0, Instance, 0 );
    
  //Window init finish

  RECT rect;
  GetWindowRect(WindowHandle, &rect);
  int width = rect.right - rect.left;
  int height = rect.bottom - rect.top;

  ResizeBuffer(&BackBuffer, width , height);


  u32 tiles[TILE_DIMENTION][TILE_DIMENTION] = {};

  //Game State initilize
  global_game.meter_to_pixel        = 30;
  f32 TileSizeInPixels = height /  TILE_DIMENTION;
  global_game.tile_size_in_meters   = TileSizeInPixels/global_game.meter_to_pixel;
  global_game.tile_dimention        = TILE_DIMENTION;
  global_game.tile_array            = (u32 *)tiles;
  global_game.tile_color            = DenormalizeColor(1, 0.1, 0);
  global_game.tile_offset_in_meters = 0.1f;
  global_game.head_x = 4;
  global_game.head_y = 4;

  while(Running){
    ProcessWindowMessages();
	
    HDC DeviceContext = GetDC(WindowHandle);


    //DisplayTree(&global_game, Tree);

    RenderGame(&BackBuffer, &global_game);


    DisplayBufferInWindow(&BackBuffer, DeviceContext);
	
    DwmFlush();
  }
  return 0;
}



void ProcessWindowMessages(){
  MSG Message;
  while(PeekMessageA(&Message, 0, 0, 0, PM_REMOVE)){
	
    switch(Message.message){
	    
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
    case WM_KEYDOWN:
    case WM_KEYUP:{
      bool IsDown = ((Message.lParam & (1 << 31)) == 0);
		
      bool WasDown = ((Message.lParam & (1 << 30)) != 0);
		
      u32 VKCode = (u32)Message.wParam;
      b32 AltKeyWasDown = ((Message.lParam & (1 << 29)) != 0);
		
      if(VKCode == 'E'){
	PaintBuffer(&BackBuffer, 1,0,1);
      }
		
      if ((VKCode == VK_F4) && AltKeyWasDown){
	Running = false;
      }

      if(VKCode == VK_RIGHT && IsDown && !WasDown){
	u32 Color = ((u8)roundf(1 * 255.0f) << 16) |
	  ((u8)roundf(0 * 255.0f) << 8)  |
	  ((u8)roundf(0 * 255.0f) << 0);
	Tree->set_last(Color);
      }
      else if(VKCode == VK_LEFT && IsDown && !WasDown){
	Tree->remove_last();
      }
    }break;
	    
    default:{
      TranslateMessage(&Message);
      DispatchMessage(&Message);
    }break;
	    
    }
	
  }
}

LRESULT CALLBACK WindowCallback(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam){
    
  LRESULT Result;
  switch(Message){
	
  case WM_PAINT:{
	    

    PAINTSTRUCT Paint;
	    
    HDC DeviceContext = BeginPaint(Window, &Paint);
	    
    RECT rect;
    GetWindowRect(Window, &rect);
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    f32 TileSizeInPixels = height /  TILE_DIMENTION;
    global_game.tile_size_in_meters   = TileSizeInPixels/global_game.meter_to_pixel;

    ResizeBuffer(&BackBuffer, width , height);
    PaintBuffer(&BackBuffer, 0,0,0);
    DrawGrid(&BackBuffer, &global_game);
    DisplayBufferInWindow(&BackBuffer, DeviceContext);
	    
    EndPaint(Window, &Paint);
	    
  }break;
  case WM_QUIT:{
    Running = false;
  }break;
  case WM_DESTROY:{
    Running = false;
  }break;

  default:
    {
      Result = DefWindowProc(Window, Message, WParam, LParam);
    }
    break;
  }
  return Result;
}

void ResizeBuffer(offscreen_buffer * Buffer, int Width, int Height){
    
  if (Buffer->Memory) {
    VirtualFree( Buffer->Memory, 0, MEM_RELEASE);
  }
    
  Buffer->Width  = Width;
  Buffer->Height = Height;
    
  Buffer->Info.bmiHeader.biSize        = sizeof(Buffer->Info.bmiHeader);
  Buffer->Info.bmiHeader.biCompression = BI_RGB;
  Buffer->Info.bmiHeader.biPlanes      = 1;
  Buffer->Info.bmiHeader.biBitCount    = 32;
  Buffer->Info.bmiHeader.biWidth       = Buffer->Width;
  Buffer->Info.bmiHeader.biHeight      = -Buffer->Height;
    
  Buffer->BytesPerPixel = 4;
  Buffer->Pitch = Width * Buffer->BytesPerPixel;
    
  int BitmapMemorySize = Buffer->Width * Buffer->Height * Buffer->BytesPerPixel;
    
  Buffer->Memory = VirtualAlloc(0, BitmapMemorySize,
				MEM_RESERVE | MEM_COMMIT,
				PAGE_READWRITE);
}

void DisplayBufferInWindow(offscreen_buffer *Buffer, HDC DeviceContext){
    
  StretchDIBits(DeviceContext,
		0, 0, Buffer->Width, Buffer->Height,
		0, 0, Buffer->Width, Buffer->Height,
		Buffer->Memory, &Buffer->Info,
		DIB_RGB_COLORS, SRCCOPY);
}

void DisplayTree(game_state *GameState, Node * tree){

  Node * curr = Tree;
  while(curr != NULL){
  }
}

//TODO: utils.cpp
u32 DenormalizeColor(f32 R, f32 G, f32 B){
  u32 Color = ((u8)roundf(R * 255.0f) << 16) |
    ((u8)roundf(G * 255.0f) << 8)  |
    ((u8)roundf(B * 255.0f) << 0);
  return Color;
}

void PaintBuffer(offscreen_buffer * Buffer, f32 R, f32 G, f32 B){
    
  u8 * Row = (u8*)Buffer->Memory; 
	
  u32 Color = DenormalizeColor(R, G, B);

  for( int y = 0; y < Buffer->Height ; y++){
	
    for( int x = 0; x < Buffer->Width; x++){
      u32 * Pixel = (u32*)Row + x;
	    
      *Pixel++ = Color;
    }
	
    Row += Buffer->Pitch;
  }
}

void DrawRectangle(offscreen_buffer * Buffer, u32 MinX,
		   u32 MinY, u32 MaxX, u32 MaxY, u32 Color){

  if(MinX <0){
    MinX = 0;
  }
  if(MinY < 0){
    MinY = 0;
  }
	
  if(MaxX > Buffer->Width){
    MaxX = Buffer->Width;
  }
	
  if(MaxY > Buffer->Height){
    MaxY = Buffer->Height;
  }

  //Pitch has bytePerPixel
  u8 *Row = (u8*)Buffer->Memory + MinY * Buffer->Pitch + MinX * Buffer->BytesPerPixel;

  for(u32 Y = MinY; Y < MaxY ; Y++){

    u32* Pixels = (u32 *)Row;

    for(u32 X = MinX; X < MaxX; X++){

      *Pixels++ = Color;
    }

    Row += Buffer->Pitch;
  }
	
}

void DrawGrid(offscreen_buffer * Buffer, game_state * GameState){

  f32 tile_size_in_pixels = GameState->meter_to_pixel * GameState->tile_size_in_meters;

  u32 Color = GameState->tile_color;

  f32 Padding = (Buffer->Width - Buffer->Height)/2;

    for(u32 X = 0; X < GameState->tile_dimention + 1; X++){

      u32 MinY = X * tile_size_in_pixels;
      u32 MinX = Padding;

      u32 MaxY = MinY + 2;
      u32 MaxX = Padding + tile_size_in_pixels * GameState->tile_dimention;
      DrawRectangle(Buffer, MinX, MinY, MaxX, MaxY, Color);
      DrawRectangle(Buffer, MinY + Padding, MinX - Padding, MaxY + Padding, MaxX - Padding, Color);
    }
}


void RenderGame(offscreen_buffer * Buffer, game_state * GameState){
  f32 tile_size_in_pixels = GameState->meter_to_pixel * GameState->tile_size_in_meters;

  u32 Color = GameState->tile_color;

  f32 Padding = (Buffer->Width - Buffer->Height)/2;

  for(u32 Y = 0; Y < GameState->tile_dimention; Y++){

    for(u32 X = 0; X < GameState->tile_dimention; X++){

      u32 MinY = Y * tile_size_in_pixels;
      u32 MinX = X * tile_size_in_pixels + Padding;

      u32 MaxY = MinY + tile_size_in_pixels;
      u32 MaxX = MinX + tile_size_in_pixels;

      if((Y + X % 2) % 2==0){
	//DrawRectangle(Buffer, MinX, MinY, MaxX, MaxY, Color);
      }else{
      }
    }
  }
}
