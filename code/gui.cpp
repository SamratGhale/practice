/**
   TODO
   1. Create a window with minimal features 
   2. Create a buffer struct offscreen_buffer
   3. Create functions that modify the buffer
**/

#include <windows.h>
#include <dwmapi.h>
#include <stdio.h>
#include <math.h>
#include <xinput.h>
#include "gui_platform.h"
#include "gui_math.h"
#include "gui.h"
#include "game.cpp"

//Global variables
global_variable bool Running = true;
global_variable offscreen_buffer BackBuffer;
//global_variable Node * Tree;
global_variable game_state global_game;
global_variable s64 GlobalPerfCountFrequency;

UINT DesiredSchedularMS = 1;
b32 SleepIsGrandular = (timeBeginPeriod(DesiredSchedularMS) == TIMERR_NOERROR);

internal LARGE_INTEGER GetWallClock(){
	LARGE_INTEGER Result;
	QueryPerformanceCounter(&Result);
	return Result;
}

//TODO: utils.cpp
u32 DenormalizeColor(f32 R, f32 G, f32 B){
	u32 Color = ((u8)roundf(R * 255.0f) << 16) |
	((u8)roundf(G * 255.0f) << 8)  |
	((u8)roundf(B * 255.0f) << 0);
	return Color;
}

internal void ProcessXInputButton(DWORD XInputButtonState, button_state *OldState, DWORD ButtonBit, button_state *NewState){
	
	NewState->EndedDown = ((XInputButtonState & ButtonBit) == ButtonBit);
	NewState->HalfTransitionCount = (OldState->EndedDown != NewState->EndedDown) ? 1 : 0;
	
}

inline f32 GetSecondsElapsed(LARGE_INTEGER Start, LARGE_INTEGER End){
	
	f32 Result = (f32)(End.QuadPart - Start.QuadPart)/(f32)GlobalPerfCountFrequency;
	return Result;
	
}

internal f32
ProcessStickValue(SHORT Value, SHORT DeadZoneThreshold){
	f32 Result = 0;
	
	if(Value < -DeadZoneThreshold){
		Result = (f32)(Value + DeadZoneThreshold) /(23768.0f - DeadZoneThreshold);
	}else if(Value > DeadZoneThreshold){
		Result = (f32)(Value - DeadZoneThreshold) /(23767.0f - DeadZoneThreshold);
	}
	return Result;
}

void DisplayBufferInWindow(offscreen_buffer *Buffer, HDC DeviceContext){
	
	StretchDIBits(DeviceContext,
								0, 0, Buffer->Width, Buffer->Height,
								0, 0, Buffer->Width, Buffer->Height,
								Buffer->Memory, &Buffer->Info,
								DIB_RGB_COLORS, SRCCOPY);
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
				
				if ((VKCode == VK_F4) && AltKeyWasDown){
					Running = false;
				}
				
				if(VKCode == VK_RIGHT && IsDown && !WasDown){
					u32 Color = ((u8)roundf(1 * 255.0f) << 16) |
					((u8)roundf(0 * 255.0f) << 8)  |
					((u8)roundf(0 * 255.0f) << 0);
					//Tree->set_last(Color);
				}
				else if(VKCode == VK_LEFT && IsDown && !WasDown){
					//Tree->remove_last();
				}
			}break;
			
			default:{
				TranslateMessage(&Message);
				DispatchMessage(&Message);
			}break;
			
		}
		
	}
}

//Just one color for all
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

internal void
ProcessKeyboardMessage(button_state * NewState, b32 IsDown){
	if(NewState->EndedDown != IsDown){
		NewState->EndedDown = IsDown;
		++NewState->HalfTransitionCount;
	}
}

internal void
ProcessPendingMessages(controller_input * KeyboardController){
	MSG Message;
	while (PeekMessageA(&Message, 0, 0, 0, PM_REMOVE))
	{
		
		// Bulletproof
		switch(Message.message){
			
			case WM_QUIT:
			{
				Running = false;
			}break;
			case WM_SYSKEYDOWN:
			case WM_SYSKEYUP:
			case WM_KEYDOWN:
			case WM_KEYUP:
			{
				bool IsDown = ((Message.lParam & (1 << 31)) == 0);
				
				bool WasDown = ((Message.lParam & (1 << 30)) != 0);
				
				u32 VKCode = (u32)Message.wParam;
				
				if (IsDown != WasDown)
				{
					if (VKCode == 'W')
					{
						ProcessKeyboardMessage(&KeyboardController->MoveUp, IsDown); 
					}
					else if (VKCode == 'A')
					{
						ProcessKeyboardMessage(&KeyboardController->MoveLeft, IsDown); 
					}
					else if (VKCode == 'S')
					{
						ProcessKeyboardMessage(&KeyboardController->MoveDown, IsDown); 
					}
					else if (VKCode == 'D')
					{
						ProcessKeyboardMessage(&KeyboardController->MoveRight, IsDown); 
					}
					else if (VKCode == 'Q')
					{
						ProcessKeyboardMessage(&KeyboardController->LeftShoulder, IsDown); 
					}
					else if (VKCode == 'E')
					{
						ProcessKeyboardMessage(&KeyboardController->RightShoulder, IsDown); 
					}
					else if (VKCode == VK_UP)
					{
						ProcessKeyboardMessage(&KeyboardController->ActionUp, IsDown); 
					}
					else if (VKCode == VK_DOWN)
					{
						ProcessKeyboardMessage(&KeyboardController->ActionDown, IsDown); 
					}
					else if (VKCode == VK_LEFT)
					{
						ProcessKeyboardMessage(&KeyboardController->ActionLeft, IsDown); 
					}
					else if (VKCode == VK_RIGHT)
					{
						ProcessKeyboardMessage(&KeyboardController->ActionRight, IsDown); 
					}
					else if (VKCode == VK_ESCAPE)
					{
						Running = false;
					}
					else if (VKCode == VK_SPACE)
					{
						ProcessKeyboardMessage(&KeyboardController->Start, IsDown);
					}
					else if (VKCode == VK_BACK)
					{
						ProcessKeyboardMessage(&KeyboardController->Back, IsDown);
					}
#if HANDMADE_INTERNAL
					else if (VKCode == 'P'){
						if(IsDown){
							GlobalPause = !GlobalPause;
						}
					}
#endif
					if(IsDown){
						b32 AltKeyWasDown = ((Message.lParam & (1 << 29)) != 0);
						if ((VKCode == VK_F4) && AltKeyWasDown){
							Running = false;
						}
					}
					
				}
			}break;
			
			default:
			{
				TranslateMessage(&Message);
				DispatchMessage(&Message);
			}break;
		}
	}
	
}


void OnPaint(HWND hWnd)
{
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
			DisplayBufferInWindow(&BackBuffer, DeviceContext);
			
			//Font
			
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
	global_game.tile_color            = DenormalizeColor(0, 0.1, 0);
	global_game.Player.color          = DenormalizeColor(.6, 0.3, 0);
	global_game.Wall.color            = DenormalizeColor(1, 1, 1);
	global_game.Player.size_in_tile   = {1, 1};
	global_game.Wall.size_in_tile   = {TILE_DIMENTION, TILE_DIMENTION/2};
	global_game.tile_offset_in_meters = 0.1f;
	
	LARGE_INTEGER PerfCountFrequencyResult;
	QueryPerformanceFrequency(&PerfCountFrequencyResult);
	GlobalPerfCountFrequency = PerfCountFrequencyResult.QuadPart;
	
	LARGE_INTEGER LastCounter = GetWallClock();
	f32 SecondsPerFrame       = 0.0333;
	
	game_input Input[2] = {};
	game_input * OldInput = &Input[0];
	game_input * NewInput = &Input[1];
	
	while(Running){
		
		HDC DeviceContext = GetDC(WindowHandle);
		
		
		controller_input * NewKeyboardController = &NewInput->Controllers[0];
		controller_input * OldKeyboardController =  &OldInput->Controllers[0];
		*NewKeyboardController = {};
		NewKeyboardController->IsConnected = true;
		NewInput->dtForFrame = 0.01667;
		
		for(int i = 0; i < ArrayCount(NewKeyboardController->Buttons); i++) {
			NewKeyboardController->Buttons[i].EndedDown = OldKeyboardController->Buttons[i].EndedDown;
		}
		
		
		POINT MouseP;
		GetCursorPos(&MouseP);
		ScreenToClient(WindowHandle, &MouseP);
		NewInput->MouseX = MouseP.x;
		NewInput->MouseY = MouseP.y;
		NewInput->MouseZ = 0;
		
		
		ProcessKeyboardMessage(&NewInput->MouseButtons[0], GetKeyState(VK_LBUTTON) & 1 << 15);
		ProcessKeyboardMessage(&NewInput->MouseButtons[1], GetKeyState(VK_MBUTTON) & 1 << 15);
		ProcessKeyboardMessage(&NewInput->MouseButtons[2], GetKeyState(VK_RBUTTON) & 1 << 15);
		ProcessKeyboardMessage(&NewInput->MouseButtons[3], GetKeyState(VK_XBUTTON1) & 1 << 15);
		ProcessKeyboardMessage(&NewInput->MouseButtons[4], GetKeyState(VK_XBUTTON2) & 1 << 15);
		
		XINPUT_STATE ControllerState;
		
		DWORD MaxControllerCount = XUSER_MAX_COUNT;
		
		
		for (DWORD ControllerIndex = 0;
		     ControllerIndex < MaxControllerCount ;
		     ++ControllerIndex)
		{
			DWORD OurControllerIndex = ControllerIndex + 1;
			controller_input *OldController = &OldInput->Controllers[OurControllerIndex];
			controller_input *NewController = &NewInput->Controllers[OurControllerIndex];
			
			XINPUT_STATE ControllerState;
			if (XInputGetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS)
			{
				
				XINPUT_GAMEPAD *Pad = &ControllerState.Gamepad;
				NewController->IsConnected = true;
				NewController->IsAnalog = OldController->IsAnalog;
				
				bool Up = Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP;
				bool Down = Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN;
				bool Left = Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT;
				bool Right = Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT;
				
				ProcessXInputButton(Pad->wButtons, &OldController->ActionDown,    XINPUT_GAMEPAD_A, &NewController->ActionDown);
				ProcessXInputButton(Pad->wButtons, &OldController->ActionRight,   XINPUT_GAMEPAD_B, &NewController->ActionRight);
				ProcessXInputButton(Pad->wButtons, &OldController->ActionLeft,    XINPUT_GAMEPAD_X, &NewController->ActionLeft);
				ProcessXInputButton(Pad->wButtons, &OldController->ActionUp,      XINPUT_GAMEPAD_Y, &NewController->ActionUp);
				ProcessXInputButton(Pad->wButtons, &OldController->LeftShoulder,  XINPUT_GAMEPAD_LEFT_SHOULDER, &NewController->LeftShoulder);
				ProcessXInputButton(Pad->wButtons, &OldController->RightShoulder, XINPUT_GAMEPAD_RIGHT_SHOULDER, &NewController->RightShoulder);
				ProcessXInputButton(Pad->wButtons, &OldController->Start,         XINPUT_GAMEPAD_START, &NewController->Start);
				ProcessXInputButton(Pad->wButtons, &OldController->Back ,         XINPUT_GAMEPAD_BACK, &NewController->Back);
				NewController->StickAverageX = ProcessStickValue(Pad->sThumbLX,    XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
				NewController->StickAverageY = ProcessStickValue(Pad->sThumbLY,    XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
				
				if((NewController->StickAverageX != 0.0f) || (NewController->StickAverageY != 0.0f)){
					NewController->IsAnalog = true;
				}
				
				if(Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP){
					NewController->StickAverageY = 1.0f;
					NewController->IsAnalog = false;
				}
				else if(Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN){
					NewController->StickAverageY = -1.0f;
					NewController->IsAnalog = false;
				}
				if(Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT){
					NewController->StickAverageX = -1.0f;
					NewController->IsAnalog = false;
				}
				else if(Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT){
					NewController->StickAverageX = 1.0f;
					NewController->IsAnalog = false;
				}
				f32 Threshold = 0.5f;
				ProcessXInputButton((NewController->StickAverageX >  Threshold)?1:0,  &NewController->MoveRight, 1, &OldController->MoveRight);
				ProcessXInputButton((NewController->StickAverageX < -Threshold)?1:0,  &NewController->MoveLeft,  1, &OldController->MoveLeft);
				ProcessXInputButton((NewController->StickAverageY >  Threshold)?1:0,  &NewController->MoveUp,    1, &OldController->MoveUp);
				ProcessXInputButton((NewController->StickAverageY < -Threshold)?1:0,  &NewController->MoveDown,  1, &OldController->MoveDown);
			}
			else
			{
				// NOTE: Controller unavailable
			}
			
		};
		ProcessPendingMessages(NewKeyboardController);
		
		char output[255];
		sprintf_s(output, sizeof(output), "x = %f y = %f\n", global_game.Player.Pos.X, global_game.Player.Pos.Y);
		OutputDebugStringA(output);
		
		
		for (int ControllerIndex = 0;
				 ControllerIndex < 5;
				 ++ControllerIndex){
			controller_input *Controller = &NewInput->Controllers[ControllerIndex];
			
			if(Controller->IsAnalog){
			}else{
				if(Controller->RightShoulder.EndedDown){
					PaintBuffer(&BackBuffer, 0,0,0);
				}
			}
		}
		
		DrawGrid(&BackBuffer, &global_game);
			RenderGame(&BackBuffer, &global_game, NewInput);
			DisplayBufferInWindow(&BackBuffer, DeviceContext);
			
		game_input * Temp = OldInput;
		OldInput = NewInput;
		NewInput = Temp;
		
		DwmFlush();
		
	}
	return 0;
}

/** Custom framerate code
    f32 SecondsElapsed = GetSecondsElapsed(LastCounter, GetWallClock());;
    
    if(SecondsElapsed < SecondsPerFrame){
    DWORD SleepMs = (DWORD)(1000.0f * (SecondsPerFrame - SecondsElapsed));
    
    if(SleepMs > 0){
    Sleep(SleepMs);
    }
    }
    
    LARGE_INTEGER WorkCounter = GetWallClock();
    s64 CounterElapsed = WorkCounter.QuadPart - LastCounter.QuadPart;
    f32 FPS = (f32)GlobalPerfCountFrequency / (f32)CounterElapsed;
    
    char DebugSoundBuffer[256];
    _snprintf_s(DebugSoundBuffer, sizeof(DebugSoundBuffer), "FPS: %3fs\n",FPS);
    
    OutputDebugStringA(DebugSoundBuffer);
    
    LastCounter = GetWallClock();
**/
