///////////////////////////////////
/*  Libraries                    */
///////////////////////////////////
#include <sys/stat.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/mount.h>

#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_gfxPrimitives.h>
#include <SDL/SDL_version.h>

///////////////////////////////////
/*  Joystick codes               */
///////////////////////////////////
#ifdef PLATFORM_LINUX
  #define GP2X_BUTTON_UP              (0)
  #define GP2X_BUTTON_DOWN            (4)
  #define GP2X_BUTTON_LEFT            (2)
  #define GP2X_BUTTON_RIGHT           (6)
  #define GP2X_BUTTON_UPLEFT          (1)
  #define GP2X_BUTTON_UPRIGHT         (7)
  #define GP2X_BUTTON_DOWNLEFT        (3)
  #define GP2X_BUTTON_DOWNRIGHT       (5)
  #define GP2X_BUTTON_CLICK           (18)
  #define GP2X_BUTTON_A               (12)
  #define GP2X_BUTTON_B               (13)
  #define GP2X_BUTTON_X               (14)
  #define GP2X_BUTTON_Y               (15)
  #define GP2X_BUTTON_L               (10)
  #define GP2X_BUTTON_R               (11)
  #define GP2X_BUTTON_START           (8)
  #define GP2X_BUTTON_SELECT          (9)
  #define GP2X_BUTTON_VOLUP           (16)
  #define GP2X_BUTTON_VOLDOWN         (17)
#endif // PLATFORM_LINUX

#ifdef PLATFORM_LINUX
    #define GCW_BUTTON_UP           SDLK_UP
    #define GCW_BUTTON_DOWN         SDLK_DOWN
    #define GCW_BUTTON_LEFT         SDLK_LEFT
    #define GCW_BUTTON_RIGHT        SDLK_RIGHT
    #define GCW_BUTTON_A            SDLK_LCTRL
    #define GCW_BUTTON_B            SDLK_LALT
    #define GCW_BUTTON_X            SDLK_LSHIFT
    #define GCW_BUTTON_Y            SDLK_SPACE
    #define GCW_BUTTON_L            SDLK_TAB
    #define GCW_BUTTON_R            SDLK_BACKSPACE
    #define GCW_BUTTON_SELECT       SDLK_ESCAPE
    #define GCW_BUTTON_START        SDLK_RETURN
    #define GCW_JOYSTICK_DEADZONE   1000
#endif // PLATFORM_LINUX

///////////////////////////////////
/*  Program modes                */
///////////////////////////////////
#define PROGRAM_MODE_MENU       1
#define PROGRAM_MODE_UNMOUNTING 2

#define TRUE  1
#define FALSE 0
///////////////////////////////////
/*  Structs                      */
///////////////////////////////////
struct joystick_state
{
  int left;
  int right;
  int up;
  int down;
  int pad_left;
  int pad_right;
  int pad_up;
  int pad_down;
  int button_a;
  int button_b;
  int button_x;
  int button_y;
  int button_l;
  int button_r;
  int button_back;
  int button_start;
  int escape;
  int any;
};

///////////////////////////////////
/*  Globals                      */
///////////////////////////////////
SDL_Surface *screen;   		    // screen to work
int done=0;
int program_mode=PROGRAM_MODE_MENU;
TTF_Font *font;                 // used font
SDL_Joystick *joystick;         // used joystick
joystick_state mainjoystick;
Uint8 *keys=SDL_GetKeyState(NULL);

std::vector<std::string> opk_list;

///////////////////////////////////
/*  Menu variables               */
///////////////////////////////////
int list_selection=0;
int list_init=0;
Uint32 umount_time;
std::string umount_opk;

///////////////////////////////////
/*  Function declarations        */
///////////////////////////////////
void process_events();
void process_joystick();
void read_mountedopks();

/*std::string getCurrentDateTime( std::string s )
{
    time_t now = time(0);
    struct tm  tstruct;
    char  buf[80];
    tstruct = *localtime(&now);
    if(s=="now")
        strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);
    else if(s=="date")
        strftime(buf, sizeof(buf), "%Y-%m-%d", &tstruct);
    return std::string(buf);
};

void Logger( std::string logMsg )
{

    std::string filePath = "/usr/local/home/log_"+getCurrentDateTime("date")+".txt";
    std::string now = getCurrentDateTime("now");
    std::ofstream ofs(filePath.c_str(), std::ios_base::out | std::ios_base::app );
    ofs << now << '\t' << logMsg << '\n';
    ofs.close();
}*/

///////////////////////////////////
/*  Functions                    */
///////////////////////////////////
void draw_text(SDL_Surface* dst, char* string, Sint16 x, Sint16 y, Uint8 fR, Uint8 fG, Uint8 fB)
{
  if(dst && string && font)
  {
    SDL_Color foregroundColor={fR,fG,fB};
    SDL_Surface *textSurface=TTF_RenderText_Blended(font,string,foregroundColor);
    if(textSurface)
    {
      SDL_Rect textLocation={x,y,0,0};
      SDL_BlitSurface(textSurface,NULL,dst,&textLocation);
      SDL_FreeSurface(textSurface);
    }
  }
}

///////////////////////////////////
/*  Get pixel from surface       */
///////////////////////////////////
Uint32 get_pixel(SDL_Surface* src, Uint32 x, Uint32 y)
{
  Uint32	color = 0;
	Uint8	*ubuff8;
	Uint16	*ubuff16;
	Uint32	*ubuff32;

	color = 0;

	switch(src->format->BytesPerPixel)
	{
		case 1:
			ubuff8 = (Uint8*) src->pixels;
			ubuff8 += (y * src->pitch) + x;
			color = *ubuff8;
			break;

		case 2:
			ubuff8 = (Uint8*) src->pixels;
			ubuff8 += (y * src->pitch) + (x*2);
			ubuff16 = (Uint16*) ubuff8;
			color = *ubuff16;
			break;

		case 3:
			ubuff8 = (Uint8*) src->pixels;
			ubuff8 += (y * src->pitch) + (x*3);
			color = 0;
			#if SDL_BYTEORDER == SDL_LIL_ENDIAN
				color |= ubuff8[2] << 16;
				color |= ubuff8[1] << 8;
				color |= ubuff8[0];
			#else
				color |= ubuff8[0] << 16;
				color |= ubuff8[1] << 8;
				color |= ubuff8[2];
			#endif
			break;

		case 4:
			ubuff8 = (Uint8*) src->pixels;
			ubuff8 += (y*src->pitch) + (x*4);
			ubuff32 = (Uint32*)ubuff8;
			color = *ubuff32;
			break;

		default:
			break;
	}
	return color;
}

///////////////////////////////////
/*  Draw pixel in surface        */
///////////////////////////////////
void set_pixel(SDL_Surface* src, int x, int y, SDL_Color& color)
{
	Uint8	*ubuff8;
	Uint16	*ubuff16;
	Uint32	*ubuff32;
  Uint32 c=SDL_MapRGB(src->format, color.r, color.g, color.b);

	switch(src->format->BytesPerPixel)
	{
		case 1:
			ubuff8 = (Uint8*) src->pixels;
			ubuff8 += (y * src->pitch) + x;
			*ubuff8 = (Uint8) c;
			break;

		case 2:
			ubuff8 = (Uint8*) src->pixels;
			ubuff8 += (y * src->pitch) + (x*2);
			ubuff16 = (Uint16*) ubuff8;
			*ubuff16 = (Uint16) c;
			break;

		case 3:
			ubuff8 = (Uint8*) src->pixels;
			ubuff8 += (y * src->pitch) + (x*3);
			#if SDL_BYTEORDER == SDL_LIL_ENDIAN
			ubuff8[0] = (Uint8) color.b;
			ubuff8[1] = (Uint8) color.g;
			ubuff8[2] = (Uint8) color.r;
			#else
			ubuff8[0] = (Uint8) color.r;
			ubuff8[1] = (Uint8) color.g;
			ubuff8[2] = (Uint8) color.b;
			#endif
			break;

		case 4:
			ubuff8 = (Uint8*) src->pixels;
			ubuff8 += (y*src->pitch) + (x*4);
			ubuff32 = (Uint32*)ubuff8;
			*ubuff32=(Uint32)c;
			break;

		default:
			break;
	}
}

void clear_joystick_state()
{
  mainjoystick.left=0;
  mainjoystick.right=0;
  mainjoystick.up=0;
  mainjoystick.down=0;
  mainjoystick.pad_left=0;
  mainjoystick.pad_right=0;
  mainjoystick.pad_up=0;
  mainjoystick.pad_down=0;
  mainjoystick.button_a=0;
  mainjoystick.button_b=0;
  mainjoystick.button_x=0;
  mainjoystick.button_y=0;
  mainjoystick.button_l=0;
  mainjoystick.button_r=0;
  mainjoystick.button_back=0;
  mainjoystick.button_start=0;
  mainjoystick.escape=0;
  mainjoystick.any=0;
}

void init_game()
{
  srand(time(NULL));
  joystick=SDL_JoystickOpen(0);
  SDL_ShowCursor(0);

  TTF_Init();
  font=TTF_OpenFont("data/pixantiqua.ttf", 12);

  read_mountedopks();
}

void end_game()
{
	SDL_FillRect(screen, NULL, 0x000000);

  if(SDL_JoystickOpened(0))
    SDL_JoystickClose(joystick);
}

void process_events()
{
  SDL_Event event;
  static int joy_pressed=FALSE;

  clear_joystick_state();
  while(SDL_PollEvent(&event))
  {
    switch(event.type)
    {
#ifdef PLATFORM_GP2X
      case SDL_JOYBUTTONDOWN:
        switch (event.jbutton.button)
        {
          case GP2X_BUTTON_LEFT:
            mainjoystick.pad_left=1;
            break;
          case GP2X_BUTTON_RIGHT:
            mainjoystick.pad_right=1;
            break;
          case GP2X_BUTTON_UP:
            mainjoystick.pad_up=1;
            break;
          case GP2X_BUTTON_DOWN:
            mainjoystick.pad_down=1;
            break;
          case GP2X_BUTTON_Y:
            mainjoystick.button_y=1;
            break;
          case GP2X_BUTTON_X:
            mainjoystick.button_a=1;
            break;
          case GP2X_BUTTON_B:
            mainjoystick.button_b=1;
            break;
          case GP2X_BUTTON_A:
            mainjoystick.button_x=1;
            break;
          case GP2X_BUTTON_START:
            mainjoystick.button_start=1;
            break;
          case GP2X_BUTTON_SELECT:
            mainjoystick.button_back=1;
            break;
        }
        mainjoystick.any=1;
        break;
#endif // PLATFORM_LINUX
#ifdef PLATFORM_WIN
      case SDL_KEYDOWN:
        switch(event.key.keysym.sym)
        {
          case SDLK_LEFT:
            mainjoystick.pad_left=1;
            break;
          case SDLK_RIGHT:
            mainjoystick.pad_right=1;
            break;
          case SDLK_UP:
            mainjoystick.pad_up=1;
            break;
          case SDLK_DOWN:
            mainjoystick.pad_down=1;
            break;
          case SDLK_a:
            mainjoystick.button_x=1;
            break;
          case SDLK_s:
            mainjoystick.button_y=1;
            break;
          case SDLK_RETURN:
          case SDLK_z:
            mainjoystick.button_a=1;
            break;
          case SDLK_x:
            mainjoystick.button_b=1;
            break;
          case SDLK_ESCAPE:
            mainjoystick.escape=1;
            break;
        }
        mainjoystick.any=1;
        break;
      case SDL_JOYBUTTONDOWN:
        switch (event.jbutton.button)
        {
          case PC_BUTTON_X:
            mainjoystick.button_x=1;
            break;
          case PC_BUTTON_Y:
            mainjoystick.button_y=1;
            break;
          case PC_BUTTON_A:
            mainjoystick.button_a=1;
            break;
          case PC_BUTTON_B:
            mainjoystick.button_b=1;
            break;
          case PC_BUTTON_BACK:
            mainjoystick.button_back=1;
            break;
          case PC_BUTTON_START:
            mainjoystick.button_start=1;
            break;
        }
        mainjoystick.any=1;
        break;
      case SDL_JOYAXISMOTION:
        switch(event.jaxis.axis)
        {
          case 0:
            if(event.jaxis.value<0)
            {
              mainjoystick.left=event.jaxis.value;
              mainjoystick.right=0;
              if(event.jaxis.value<-32000)
              {
                mainjoystick.pad_left=1;
                mainjoystick.any=1;
              }
            }
            else
            {
              mainjoystick.right=event.jaxis.value;
              mainjoystick.left=0;
              if(event.jaxis.value>32000)
              {
                mainjoystick.pad_right=1;
                mainjoystick.any=1;
              }
            }
            break;
          case 1:
            if(event.jaxis.value<0)
            {
              mainjoystick.up=event.jaxis.value;
              mainjoystick.down=0;
              if(event.jaxis.value<-32000)
              {
                mainjoystick.pad_up=1;
                mainjoystick.any=1;
              }
            }
            else
            {
              mainjoystick.down=event.jaxis.value;
              mainjoystick.up=0;
              if(event.jaxis.value>32000)
              {
                mainjoystick.pad_down=1;
                mainjoystick.any=1;
              }
            }
            break;
        }
        break;
      case SDL_JOYHATMOTION:
        switch(event.jhat.value)
        {
          case 1:
            mainjoystick.pad_up=1;
            break;
          case 2:
            mainjoystick.pad_right=1;
            break;
          case 4:
            mainjoystick.pad_down=1;
            break;
          case 8:
            mainjoystick.pad_left=1;
            break;
        }
        mainjoystick.any=1;
        break;
#endif // PLATFORM_WIN
#ifdef PLATFORM_LINUX
      case SDL_KEYDOWN:
        switch(event.key.keysym.sym)
        {
          case GCW_BUTTON_LEFT:
            mainjoystick.pad_left=1;
            break;
          case GCW_BUTTON_RIGHT:
            mainjoystick.pad_right=1;
            break;
          case GCW_BUTTON_UP:
            mainjoystick.pad_up=1;
            break;
          case GCW_BUTTON_DOWN:
            mainjoystick.pad_down=1;
            break;
          case GCW_BUTTON_X:
            mainjoystick.button_x=1;
            break;
          case GCW_BUTTON_Y:
            mainjoystick.button_y=1;
            break;
          case GCW_BUTTON_A:
            mainjoystick.button_a=1;
            break;
          case GCW_BUTTON_B:
            mainjoystick.button_b=1;
            break;
          case GCW_BUTTON_SELECT:
            mainjoystick.escape=1;
            break;
          case GCW_BUTTON_START:
            mainjoystick.button_start=1;
        }
        mainjoystick.any=1;
        break;
      /*case SDL_JOYBUTTONDOWN:
        switch (event.jbutton.button)
        {
          case PC_BUTTON_X:
            mainjoystick.button_x=1;
            break;
          case PC_BUTTON_Y:
            mainjoystick.button_y=1;
            break;
          case PC_BUTTON_A:
            mainjoystick.button_a=1;
            break;
          case PC_BUTTON_B:
            mainjoystick.button_b=1;
            break;
          case PC_BUTTON_BACK:
            mainjoystick.button_back=1;
            break;
          case PC_BUTTON_START:
            mainjoystick.button_start=1;
            break;
        }
        mainjoystick.any=1;
        break;*/
      case SDL_JOYAXISMOTION:
        if(joy_pressed && SDL_JoystickGetAxis(joystick,0)>-GCW_JOYSTICK_DEADZONE && SDL_JoystickGetAxis(joystick,0)<GCW_JOYSTICK_DEADZONE && SDL_JoystickGetAxis(joystick,1)>-GCW_JOYSTICK_DEADZONE && SDL_JoystickGetAxis(joystick,1)<GCW_JOYSTICK_DEADZONE)
        {
          joy_pressed=FALSE;
        }

        if(!joy_pressed)
        {
            switch(event.jaxis.axis)
            {
              case 0:
                if(event.jaxis.value<0)
                {
                  mainjoystick.left=event.jaxis.value;
                  mainjoystick.right=0;
                  if(event.jaxis.value<-GCW_JOYSTICK_DEADZONE)
                  {
                    mainjoystick.pad_left=1;
                    mainjoystick.any=1;
                    joy_pressed=TRUE;
                  }
                }
                else
                {
                  mainjoystick.right=event.jaxis.value;
                  mainjoystick.left=0;
                  if(event.jaxis.value>GCW_JOYSTICK_DEADZONE)
                  {
                    mainjoystick.pad_right=1;
                    mainjoystick.any=1;
                    joy_pressed=TRUE;
                  }
                }
                break;
              case 1:
                if(event.jaxis.value<0)
                {
                  mainjoystick.up=event.jaxis.value;
                  mainjoystick.down=0;
                  if(event.jaxis.value<-GCW_JOYSTICK_DEADZONE)
                  {
                    mainjoystick.pad_up=1;
                    mainjoystick.any=1;
                    joy_pressed=TRUE;
                  }
                }
                else
                {
                  mainjoystick.down=event.jaxis.value;
                  mainjoystick.up=0;
                  if(event.jaxis.value>GCW_JOYSTICK_DEADZONE)
                  {
                    mainjoystick.pad_down=1;
                    mainjoystick.any=1;
                    joy_pressed=TRUE;
                  }
                }
                break;
            }
        }
        break;
#endif // PLATFORM_LINUX
      }
  }
}

// process keyboard and joystick (no events), and save in mainjoystick variable
void process_joystick()
{
#ifdef PLATFORM_GP2X
  if(SDL_JoystickGetButton(joystick, GP2X_BUTTON_START))
    mainjoystick.button_start=1;
  else
    mainjoystick.button_start=0;
  if(SDL_JoystickGetButton(joystick, GP2X_BUTTON_SELECT))
    mainjoystick.button_back=1;
  else
    mainjoystick.button_back=0;
  if(SDL_JoystickGetButton(joystick, GP2X_BUTTON_LEFT) || SDL_JoystickGetButton(joystick, GP2X_BUTTON_UPLEFT) || SDL_JoystickGetButton(joystick, GP2X_BUTTON_DOWNLEFT))
    mainjoystick.pad_left=1;
  else
    mainjoystick.pad_left=0;
  if(SDL_JoystickGetButton(joystick, GP2X_BUTTON_RIGHT) || SDL_JoystickGetButton(joystick, GP2X_BUTTON_UPRIGHT) || SDL_JoystickGetButton(joystick, GP2X_BUTTON_DOWNRIGHT))
    mainjoystick.pad_right=1;
  else
    mainjoystick.pad_right=0;
  if(SDL_JoystickGetButton(joystick, GP2X_BUTTON_UP) || SDL_JoystickGetButton(joystick, GP2X_BUTTON_UPLEFT) || SDL_JoystickGetButton(joystick, GP2X_BUTTON_UPRIGHT))
    mainjoystick.pad_up=1;
  else
    mainjoystick.pad_up=0;
  if(SDL_JoystickGetButton(joystick, GP2X_BUTTON_DOWN) || SDL_JoystickGetButton(joystick, GP2X_BUTTON_DOWNLEFT) || SDL_JoystickGetButton(joystick, GP2X_BUTTON_DOWNRIGHT))
    mainjoystick.pad_down=1;
  else
    mainjoystick.pad_down=0;
  if(SDL_JoystickGetButton(joystick, GP2X_BUTTON_X))
    mainjoystick.button_a=1;
  else
    mainjoystick.button_a=0;
  if(SDL_JoystickGetButton(joystick, GP2X_BUTTON_B))
    mainjoystick.button_b=1;
  else
    mainjoystick.button_b=0;
  if(SDL_JoystickGetButton(joystick, GP2X_BUTTON_A))
    mainjoystick.button_x=1;
  else
    mainjoystick.button_x=0;
  if(SDL_JoystickGetButton(joystick, GP2X_BUTTON_Y))
    mainjoystick.button_y=1;
  else
    mainjoystick.button_y=0;
#endif // PLATFORM_LINUX
#ifdef PLATFORM_WIN
  if(keys[SDLK_ESCAPE])
    mainjoystick.escape=1;
  else
    mainjoystick.escape=0;
  if(keys[SDLK_z] || SDL_JoystickGetButton(joystick, PC_BUTTON_A))
    mainjoystick.button_a=1;
  else
    mainjoystick.button_a=0;
  if(keys[SDLK_x] || SDL_JoystickGetButton(joystick, PC_BUTTON_B))
    mainjoystick.button_b=1;
  else
    mainjoystick.button_b=0;
  if(keys[SDLK_a] || SDL_JoystickGetButton(joystick, PC_BUTTON_X))
    mainjoystick.button_x=1;
  else
    mainjoystick.button_x=0;
  if(keys[SDLK_s] || SDL_JoystickGetButton(joystick, PC_BUTTON_Y))
    mainjoystick.button_y=1;
  else
    mainjoystick.button_y=0;

  if(keys[SDLK_LEFT] || SDL_JoystickGetHat(joystick,0)&SDL_HAT_LEFT || (SDL_JoystickGetAxis(joystick,0)<-15000))// || SDL_JoystickGetHat(joystick,0)&SDL_HAT_LEFTUP || SDL_JoystickGetHat(joystick,0)&SDL_HAT_LEFTDOWN)
    mainjoystick.pad_left=1;
  else
    mainjoystick.pad_left=0;
  if(keys[SDLK_RIGHT] || SDL_JoystickGetHat(joystick,0)&SDL_HAT_RIGHT || (SDL_JoystickGetAxis(joystick,0)>15000))// || SDL_JoystickGetHat(joystick,0)&SDL_HAT_RIGHTUP || SDL_JoystickGetHat(joystick,0)&SDL_HAT_RIGHTDOWN)
    mainjoystick.pad_right=1;
  else
    mainjoystick.pad_right=0;
  if(keys[SDLK_UP] || SDL_JoystickGetHat(joystick,0)&SDL_HAT_UP || (SDL_JoystickGetAxis(joystick,1)<-15000))// || SDL_JoystickGetHat(joystick,0)&SDL_HAT_LEFTUP || SDL_JoystickGetHat(joystick,0)&SDL_HAT_RIGHTUP)
    mainjoystick.pad_up=1;
  else
    mainjoystick.pad_up=0;
  if(keys[SDLK_DOWN] || SDL_JoystickGetHat(joystick,0)&SDL_HAT_DOWN || (SDL_JoystickGetAxis(joystick,1)>15000))// || SDL_JoystickGetHat(joystick,0)&SDL_HAT_LEFTDOWN || SDL_JoystickGetHat(joystick,0)&SDL_HAT_RIGHTDOWN)
    mainjoystick.pad_down=1;
  else
    mainjoystick.pad_down=0;
  if(SDL_JoystickGetButton(joystick, PC_BUTTON_START))
    mainjoystick.button_start=1;
  else
    mainjoystick.button_start=0;
  if(SDL_JoystickGetButton(joystick, PC_BUTTON_BACK))
    mainjoystick.button_back=1;
  else
    mainjoystick.button_back=0;
  if(SDL_JoystickGetButton(joystick, PC_BUTTON_L))
    mainjoystick.button_l=1;
  else
    mainjoystick.button_l=0;
  if(SDL_JoystickGetButton(joystick, PC_BUTTON_R))
    mainjoystick.button_r=1;
  else
    mainjoystick.button_r=0;
#endif // PLATFORM_WIN
#ifdef PLATFORM_LINUX
  if(keys[GCW_BUTTON_SELECT])
    mainjoystick.escape=1;
  else
    mainjoystick.escape=0;
  if(keys[GCW_BUTTON_A])
    mainjoystick.button_a=1;
  else
    mainjoystick.button_a=0;
  if(keys[GCW_BUTTON_B])
    mainjoystick.button_b=1;
  else
    mainjoystick.button_b=0;
  if(keys[GCW_BUTTON_X])
    mainjoystick.button_x=1;
  else
    mainjoystick.button_x=0;
  if(keys[GCW_BUTTON_Y])
    mainjoystick.button_y=1;
  else
    mainjoystick.button_y=0;

  if(keys[GCW_BUTTON_LEFT] || (SDL_JoystickGetAxis(joystick,0)<-GCW_JOYSTICK_DEADZONE))
    mainjoystick.pad_left=1;
  else
    mainjoystick.pad_left=0;
  if(keys[GCW_BUTTON_RIGHT] || (SDL_JoystickGetAxis(joystick,0)>GCW_JOYSTICK_DEADZONE))
    mainjoystick.pad_right=1;
  else
    mainjoystick.pad_right=0;
  if(keys[GCW_BUTTON_UP] || (SDL_JoystickGetAxis(joystick,1)<-GCW_JOYSTICK_DEADZONE))
    mainjoystick.pad_up=1;
  else
    mainjoystick.pad_up=0;
  if(keys[GCW_BUTTON_DOWN] || (SDL_JoystickGetAxis(joystick,1)>GCW_JOYSTICK_DEADZONE))
    mainjoystick.pad_down=1;
  else
    mainjoystick.pad_down=0;
  if(keys[GCW_BUTTON_START])
    mainjoystick.button_start=1;
  else
    mainjoystick.button_start=0;
  if(keys[GCW_BUTTON_SELECT])
    mainjoystick.button_back=1;
  else
    mainjoystick.button_back=0;
  if(keys[GCW_BUTTON_L])
    mainjoystick.button_l=1;
  else
    mainjoystick.button_l=0;
  if(keys[GCW_BUTTON_R])
    mainjoystick.button_r=1;
  else
    mainjoystick.button_r=0;
#endif // PLATFORM_LINUX
}

void read_mountedopks()
{
  list_init=0;
  list_selection=0;
  opk_list.clear();

  DIR *dir;
  struct dirent *ent;
  if ((dir = opendir ("/mnt")) != NULL)
  {
    /* print all the files and directories within directory */
    while ((ent = readdir (dir)) != NULL)
    {
      if(ent->d_type==DT_DIR && std::string(ent->d_name)!="." && std::string(ent->d_name)!="..")
        opk_list.push_back(ent->d_name);
    }
    closedir (dir);
  }
}

void draw_menu()
{
  static int counter=0;

  SDL_FillRect(screen,NULL,SDL_MapRGB(screen->format,56,152,255));
  draw_text(screen,(char*)"Unmount OPK tool",10,15,255,255,255);

  for(int f=list_init; f<opk_list.size(); f++)
  {
    if(list_selection==f)
    {
      boxRGBA(screen,0,30+f*12,320,30+f*12+12,96,96,192,255);
      draw_text(screen,(char*)opk_list[f].c_str(),10,30+f*12,255,192,192);
    }
    else
      draw_text(screen,(char*)opk_list[f].c_str(),10,30+f*12,255,255,255);
  }

  draw_text(screen,(char*)"Press [Start] to exit.",10,225,255,255,255);

  // SDL version
  /*SDL_version compiled;
  SDL_VERSION(&compiled);
  char v[20];
  sprintf(v,"%d.%d.%d",compiled.major,compiled.minor,compiled.patch);
  draw_text(screen,v,280,0,192,192,192);*/
}

void update_menu()
{
  process_events();

  if(mainjoystick.button_start)
    done=TRUE;

  if(mainjoystick.pad_up && list_selection>0)
  {
    list_selection--;
  }
  if(mainjoystick.pad_down && list_selection<(opk_list.size()-1))
  {
    list_selection++;
  }
  if(mainjoystick.button_a && opk_list.size()>0)
  {
    umount2(std::string("/mnt/"+opk_list[list_selection]).c_str(),MNT_FORCE);
    umount_opk=opk_list[list_selection];
    umount_time=SDL_GetTicks();
    program_mode=PROGRAM_MODE_UNMOUNTING;
  }
}

void draw_unmounting()
{
  draw_menu();
  boxRGBA(screen,0,120-20,320,120+20,192,128,96,128);
  if(SDL_GetTicks()-umount_time>1000)
    draw_text(screen,(char*)std::string("Umounting "+umount_opk).c_str(),20,115,255,255,255);
  else
    draw_text(screen,(char*)std::string("Umounting "+umount_opk).c_str(),20,115,192,192,192);
}

void update_unmounting()
{
  process_events();

  if(SDL_GetTicks()-umount_time>1000)
  {
    if(mainjoystick.button_a)
    {
      read_mountedopks();
      program_mode=PROGRAM_MODE_MENU;
    }
  }
}

///////////////////////////////////
/*  Init                         */
///////////////////////////////////
int main(int argc, char *argv[])
{
  if(SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_VIDEO | SDL_INIT_AUDIO)<0)
		return 0;

  screen = SDL_SetVideoMode(320, 240, 16, SDL_SWSURFACE | SDL_DOUBLEBUF);
    if (screen==NULL)
      return 0;

  SDL_JoystickEventState(SDL_ENABLE);
  joystick=SDL_JoystickOpen(0);
  SDL_ShowCursor(0);

  init_game();

  const int GAME_FPS=60;
  Uint32 start_time;

  while(!done)
	{
    start_time=SDL_GetTicks();
    switch(program_mode)
    {
      case PROGRAM_MODE_MENU:
        update_menu();
        draw_menu();
        break;
      case PROGRAM_MODE_UNMOUNTING:
        update_unmounting();
        draw_unmounting();
        break;
      default:
        update_menu();
        draw_menu();
        break;
    }

    SDL_Flip(screen);

    // set FPS 60
    if(1000/GAME_FPS>SDL_GetTicks()-start_time)
      SDL_Delay(1000/GAME_FPS-(SDL_GetTicks()-start_time));
	}

  end_game();
  SDL_Quit();

	return 1;
}
