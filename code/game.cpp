#include <stdlib.h>

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

void ProcessPlayerMovement(v2 Direction,game_state * GameState, entity * Player, offscreen_buffer * Buffer){
	if(Direction.X != 0  || Direction.Y != 0){
		Player->transform.direction = Direction;
		Player->transform.active = true;
	}
	
	f32 tile_size_in_pixels = GameState->meter_to_pixel * GameState->tile_size_in_meters;
	
	u32 Color = Player->color;
	
	s32 MinX = 0;
	s32 MinY = 0;
	
	if(Player->transform.active){
		MinY += Player->transform.direction.Y * Player->transform.completed;
		MinX += Player->transform.direction.X * Player->transform.completed;
		Player->transform.completed+= 4;
		
		if(Player->transform.completed >= tile_size_in_pixels * Player->size_in_tile.Y){
			
			Player->Pos += Player->transform.direction * Player->size_in_tile.Y;
			MinX = 0;
			MinY = 0;
			Player->transform.active = false;
			Player->transform.completed = 0;
		}
	}
	
	v2 P = Player->Pos;
	
	f32 Padding = (Buffer->Width - Buffer->Height)/2;
	MinY += P.Y * tile_size_in_pixels ;
	MinX += P.X * tile_size_in_pixels + Padding;
	
	s32 MaxY = MinY + tile_size_in_pixels * Player->size_in_tile.Y + 2;
	s32 MaxX = MinX + tile_size_in_pixels * Player->size_in_tile.X + 2;
	
	DrawRectangle(Buffer, MinX, MinY, MaxX, MaxY, Color);
}


#define WAS_DOWN(b, C)(b.EndedDown && (b.HalfTransitionCount || C->LeftShoulder.EndedDown) ? 1 :0)

void RenderGame(offscreen_buffer * Buffer, game_state * GameState, game_input * Input){
	
	entity * Player = &GameState->Player;
	entity * Wall = &GameState->Wall;
	
	if(!GameState->Initilized){
		Player->Pos = {2, 2};
		Wall->Pos = {0, 0};
		Player->transform.active = false;
		Wall->transform.active = false;
		GameState->Initilized = true; 
	}
	v2 NewPlayer = {}; 
	v2 WallMovement = {};
	v2 ddPlayer = {};
	for (int ControllerIndex = 0;
			 ControllerIndex < 5;
			 ++ControllerIndex){
		controller_input *Controller = &Input->Controllers[ControllerIndex];
		
		if(!Player->transform.active){
			if(Controller->IsAnalog){
			}else{
				if(WAS_DOWN(Controller->MoveUp, Controller)){
					ddPlayer.Y = -1.0f;
				}
				if(WAS_DOWN(Controller->MoveDown, Controller)){
					ddPlayer.Y = 1.0f;
				}
				if(WAS_DOWN(Controller->MoveLeft, Controller)){
					ddPlayer.X = -1.0f;
				}
				if(WAS_DOWN(Controller->MoveRight, Controller)){
					ddPlayer.X = 1.0f;
				}
				if(Controller->ActionDown.EndedDown){
					WallMovement.Y = 1;
				}
				if(Controller->ActionUp.EndedDown){
					WallMovement.Y = -1;
				}
				NewPlayer = ddPlayer;
			}
		}
	}
	
	ProcessPlayerMovement(NewPlayer, GameState, &GameState->Player, Buffer);
	ProcessPlayerMovement(WallMovement, GameState, &GameState->Wall, Buffer);
	
}
