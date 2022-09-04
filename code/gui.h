#if !defined(GUI_H)
#include <math.h>
#include <stdint.h>
#include <windows.h>

#define Pi32 3.14159265359f

#define internal static
#define local_persist static
#define global_variable static

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef s32 b32;

typedef float f32;
typedef double f64;

#define Kilobytes(Value) ((Value) * 1024LL)
#define Megabytes(Value) (Kilobytes(Value) * 1024LL)
#define Gigabytes(Value) (Megabytes(Value) * 1024LL)
#define Terabytes(Value) (Gigabytes(Value) * 1024LL)

#define ArrayCount(Array)(sizeof(Array)/sizeof((Array)[0]))

struct offscreen_buffer
{
  BITMAPINFO Info;
  void* Memory;
  int   Pitch;
  int   Width;
  int   Height;
  int   BytesPerPixel;
};

struct draw_state
{
  u32 Color;
};

#define TILE_DIMENTION 60

struct game_state{
  f32 meter_to_pixel;
  f32 tile_size_in_meters;
  u32 tile_color;
  u32 tile_dimention; 
  u32 * tile_array;
  u32 tile_offset_in_meters;
  u32 head_x;
  u32 head_y;
};

struct button_state{
  s32 HalfTransitionCount;
  b32 EndedDown;
};


struct controller_input{
  b32 IsConnected;
  b32 IsAnalog;
	
  f32 StickAverageX;
  f32 StickAverageY;
	
  union{
    button_state Buttons[12];
    struct{
      button_state MoveUp;
      button_state MoveDown;
      button_state MoveLeft;
      button_state MoveRight;
			
      button_state ActionUp;
      button_state ActionDown;
      button_state ActionLeft;
      button_state ActionRight;
			
      button_state LeftShoulder;
      button_state RightShoulder;
			
      button_state Back;
      button_state Start;
    };
  };
};

#define GUI_H
#endif
