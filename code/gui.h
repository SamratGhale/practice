#if !defined(GUI_H)
#include <math.h>
#include <windows.h>
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

#define TILE_DIMENTION 20

struct entity_transform_info{
	b32 active;
	f32 completed;
	v2 direction;
};

struct entity{
	v2 Pos;
	v2 dPos;
	u32 color;
	v2 size_in_tile;
	entity_transform_info transform;
};

struct game_state{
	f32 meter_to_pixel;
	f32 tile_size_in_meters;
	u32 tile_color;
	u32 tile_dimention; 
	u32 * tile_array;
	entity Player;
	entity Wall;
	u32 tile_offset_in_meters;
	b32 Initilized;
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

struct game_input{
    button_state MouseButtons[5];
    f32 dtForFrame;
    s32 MouseX, MouseY, MouseZ;
    controller_input Controllers[5];
};

#define GUI_H
#endif
