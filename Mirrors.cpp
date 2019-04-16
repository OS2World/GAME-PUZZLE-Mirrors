#define INCL_WIN
#define INCL_GPI
#define INCL_DOSPROCESS
#define INCL_DOSERRORS
#define INCL_DOSDATETIME
#define INCL_DOSMODULEMGR
#define INCL_DOSMISC

#include <OS2.h>
#include <StdLib.h>
#include <String.h>
#include "Mciapi.cpp"

#define RUSSIAN         866

#define RC_ICON         1
#define RC_SPRITES      1
#define RC_BASES        501
#define RC_BITMAPS      1001

#define SOUND_MENU_ITEM 100
#define DIFF_MENU_ITEM  101
#define HELP_MENU_ITEM  102

#define WM_DRAW         WM_USER

#define WM_LEFT         WM_USER + 1
#define WM_RIGHT        WM_USER + 2
#define WM_DOWN         WM_USER + 3
#define WM_BOUNCE       WM_USER + 4
#define WM_MOUSE        WM_USER + 5
#define WM_GUIDE        WM_USER + 6

#define WM_PLAYSOUND    WM_USER + 100

#define SPRITE_WIDTH    22
#define SPRITE_HEIGHT   18
#define BORDER_WIDTH    10
#define GRID_WIDTH      1

#define MAX_SPRITE      48
#define MAX_BOX         8

#define FLYING          1
#define DARTING         2
#define THROWING        3
#define PACKING         4
#define DISAPPEARING    5

#define MAX_STEP        3
#define TIMER_RATE      25
#define STOP_POINT      10

#define SOUND_BEGIN     0
#define SOUND_GAME      1
#define SOUND_PAUSE     2
#define SOUND_MOUSE     3
#define SOUND_DELETE    4
#define SOUND_OVER      5
#define SOUND_END       6

#define MAX_SOUND       6

// ��騥 ��६����.
struct SYSTEM_METRICS
 {
  // ��࠭�, � ���ன ࠡ�⠥� �ਫ������.
  INT Code_page;
  // ������ �࠭�.
  INT X_Screen;
  INT Y_Screen;
 };
SYSTEM_METRICS System_metrics = { 0, 0, 0 };

// �ਫ������.
HAB Application = NULLHANDLE;

// ��।� ᮮ�饭�� ��⮪�.
HMQ Thread_message_queue = NULLHANDLE;

// �⢥�� �� ��⮪�.
struct THREAD_RESPONDS
 {
  // ��⮪ ᮧ���. 0 - ���, 1 - ��, -1 - �ந��諠 �訡��.
  INT Thread_is_created;
  // ����饭�� � ����ᮢ�� ���� �ਭ��.
  INT Draw_message_is_received;
  // ����饭�� �� ���稪� �६��� �ਭ��.
  INT Timer_message_is_received;
 };
THREAD_RESPONDS Thread_responds = { 0, 1, 1 };

// ����ࠦ����, ���஥ �ᯮ������ � ����.
struct BACKGROUND
 {
  // ����ࠦ����.
  HBITMAP Bitmap;
  // ��ਭ�.
  INT Bitmap_width;
  // ����.
  INT Bitmap_height;
  // ����ࠦ����, �����⮢������ � �����.
  HBITMAP Tile;
 };
BACKGROUND Background = { NULLHANDLE, 0, 0, NULLHANDLE };

// ����ன�� ��� ����.
struct SETTINGS
 {
  // ����ﭨ� ����. 0 - ���⠢��, 1 - ��砫� ����, 2 - ���, 3 - �����襭�� ����.
  INT Game_mode;
  // ���� �� �⪠. 0 - ���, 1 - ����.
  INT Grid_is_visible;
  // ��⠭������ �� ���. 0 - ���, 1 - ����.
  INT Game_is_paused;
  // ����� �� ����� ᫮��� �।����. 0 - ���, 1 - ��.
  INT Game_is_difficult;
  // ����祭 �� ���. 0 - ���, 1 - ��.
  INT Sound_is_enabled;
 };
SETTINGS Settings = { 0, 1, 0, 1, 1 };

// ��ப� ⥪��, ����� ����ࠦ����� � ����.
struct STRINGS_IN_WINDOW
 {
  // ��ப� ����.
  CHAR Menu_strings[ 3 ][ 20 ];
  // ��������.
  CHAR Title[ 25 ];
  // ���ᠭ�� ����.
  CHAR Description[ 6 ][ 50 ];
  // ���ࠡ��稪.
  CHAR Developer[ 50 ];
 };
STRINGS_IN_WINDOW Strings_in_window = { NULL, NULL, NULL, NULL };

// ������� ��ઠ� ������� �� ࠧ��஢ ����.
struct MIRROR_SIZE
 {
  // ��ਭ� ��ઠ��.
  INT Width;
  // ���� ��ઠ��.
  INT Height;
 };
MIRROR_SIZE Mirror_size = { 0, 0 };

// ��ઠ�� ࠧ�������� �� �������.
struct MIRROR_SQUARES
 {
  // �� �ᯮ������ � �⮬ ������? 0 - ��祣�, 1-8 - �騪, 9-48 - �।���.
  INT Object;
  // ����, �� ���ன ������ ���� ���ᮢ�� �।���. ����� ���� ����� ���.
  INT Y_Offset;
  // ����⢨� - �����, ��������, ��� �������.
  INT Action;
  // ���稪� - ࠡ����, �०�� 祬 �।��� �ॢ����� � �騪.
  INT Action_count; INT Change_point;
 };
MIRROR_SQUARES* Left_mirror = NULL; MIRROR_SQUARES* Right_mirror = NULL;

// ��窨 � ����, �� ����� ������ ���� ���ᮢ��� ��ઠ��.
// A B C D E F
//  �   �   �
// G�H I�J K�L
//  �������ͼ
struct MIRROR_POINTS
 {
  INT Ax; INT Ay; INT Bx; INT By; INT Cx; INT Cy;
  INT Dx; INT Dy; INT Ex; INT Ey; INT Fx; INT Fy;
  INT Gx; INT Gy; INT Hx; INT Hy; INT Ix; INT Iy;
  INT Jx; INT Jy; INT Kx; INT Ky; INT Lx; INT Ly;
 };
MIRROR_POINTS Mirror_points = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

// ����࠭�⢮ �⮡ࠦ���� � �����.
HPS Memory_space = NULLHANDLE;

// ����ࠦ���� �।��⮢.
struct SPRITE
 {
  // ����ࠦ����.
  HBITMAP Image;
  // �᭮�� ��� ����.
  HBITMAP Base;
 };
SPRITE Sprite[ MAX_SPRITE + 1 ];

// ������ ��।������� �।��⮢.
struct RATE
 {
  // ����� ᢥ��� ����.
  INT Flying_step;
  // ��६�⥫쭮� ��������.
  INT Darting_step;
  // �������.
  INT Throwing_step;
 };
RATE Rate = { MAX_STEP - 1, MAX_STEP * 3, MAX_STEP };

// ���.
struct SOUND
 {
  // ����� ��㪮��� 䠩���, ����� �ᯮ�짮���� � ���.
  CHAR Sound_file[ MAX_SOUND + 1 ][ 255 ];
  // ��㪨, ����㦥��� � ������. ��� ᮮ⢥������ ������ 䠩���.
  HSOUND Sound_in_memory[ MAX_SOUND + 1 ];
  // �맢����� �ਫ������: ��娢, ����� ᮤ�ন� ���.
  PID Sound_application;
  // ����뢠��, �� � �ਫ������ ���ந�������� ���.
  INT Sound_is_playing;
 };
SOUND Sound = { NULL, NULL, NULLHANDLE, 0 };

// �����頥� ��砩��� �᫮ �� 0 �� Max.
INT Rnd( INT Max );

// ��७��� ��㪮�� 䠩�� �� �६���� ��⠫��, ������ ����� 䠩���
VOID PrepareSoundFiles( VOID );

// ������ ��㪮�� 䠩�� �� �६����� ��⠫���.
VOID DeleteSoundFiles( VOID );

// �����⠢������ � ����� ����ࠦ����, ���஥ �ᯮ������ � ����.
VOID CreateBackgroundTile( VOID );

// ����� ����ࠦ���� �।���.
VOID DrawSprite( HPS Presentation_space, INT X, INT Y, SPRITE Sprite );

// ���뢠�� �।���, ����⠭������� ���� ���⨭�� � ����.
VOID HideSprite( HPS Presentation_space, INT X, INT Y );

// �ਭ����� ᮮ�饭��, ����� ��室�� � ���� �ਫ������. ����饭�� ��।����� � ��⮪.
MRESULT EXPENTRY WinProc( HWND Window, ULONG Message, MPARAM First_parameter, MPARAM Second_parameter );

// ��⮪ ��� �ᮢ����.
VOID ThreadProc( VOID );

// ��ࠡ��稪 ᮮ�饭��, ����� �뫨 ��।��� � ��⮪ �ᮢ����.
VOID MessageProcessing( ULONG Message, MPARAM First_parameter, MPARAM Second_parameter );

// �������� ��ப� � ���� ����.
VOID AddMenu( HWND Frame_window );

// �⠢�� �⬥⪨ � ��ப� ���� ��� ᭨���� ��.
VOID CheckMenuItems( HWND Frame_window );

// ������ ��ઠ�� ����묨.
VOID CleanMirrors( VOID );

// ��ᯮ������ �� �����孮�� ��ઠ� ��᪮�쪮 �।��⮢.
VOID ThrowObjects( VOID );

// ����� �� � ����.
VOID DrawImageInWindow( HWND Client_window );

// ������ ࠧ���� ��ઠ�.
VOID CalculateMirrorSize( RECTL Window_rectangle );

// ������ �窨, �� ����� ���� ���ᮢ��� ��ઠ��.
VOID CalculateMirrorPoints( RECTL Window_rectangle );

// ��ࠡ��뢠�� ᮮ�饭�� �� ���稪� �६���, ��।������ �।����.
VOID AnimateObjects( HWND Client_window );

// ������ ���.
INT DeleteRows( HWND Client_window );

// �஢����, ���� �� � ��ઠ��� "�����訥" �騪� � ��ᠥ� ��.
VOID DetectHangedBoxes( VOID );

// ��ᯮ������ �� �����孮�� ��ઠ� ����� �।��⮢.
INT ThrowObjectSet( HWND Client_window );

// ������ �� �।���� � �����孮�� ��ઠ�.
VOID DeleteAllObjects( VOID );

// ��।������ ����� �।��⮢.
VOID MoveObjects( HWND Client_window, INT Where );

// ��ࠡ��뢠�� ᮮ�饭�� �� ���.
VOID TranslateMouseMessages( INT X_Point, INT Y_Point );

// ��� �ਫ������ ���

INT main( VOID )
{
 // ��।��塞 �ਫ������ � ��⥬�.
 Application = WinInitialize( 0 );
 // �᫨ �� ᤥ���� �� 㤠���� - ��室.
 if( Application == NULLHANDLE )
  {
   // ��� - �訡��.
   WinAlarm( HWND_DESKTOP, WA_ERROR );
   // ��室.
   return 0;
  }

 // ������� ��⮪ ��� ��ࠡ�⪨ ᮮ�饭��
 TID Thread_ID = NULLHANDLE;
 APIRET Thread_is_created = DosCreateThread( &Thread_ID, (PFNTHREAD) ThreadProc, 0, 0, 8192 );

 // �᫨ ��⮪ ᮧ��� - ����, ���� � ��� �㤥� ᮧ���� ��।� ᮮ�饭��.
 if( Thread_is_created == NO_ERROR ) while( Thread_responds.Thread_is_created == 0 ) { DosSleep( 1 ); }

 // �᫨ ��⮪ ᮧ���� �� 㤠���� - ��室.
 if( Thread_is_created != NO_ERROR || Thread_responds.Thread_is_created == -1 )
  {
   // ��� - �訡��.
   WinAlarm( HWND_DESKTOP, WA_ERROR );
   // ��室.
   WinTerminate( Application ); return 0;
  }

 // ������� ��।� ᮮ�饭��.
 HMQ Message_queue = WinCreateMsgQueue( Application, 0 );
 // �᫨ ��।� ᮧ���� �� 㤠���� - ��室.
 if( Message_queue == NULLHANDLE )
  {
   // ��� - �訡��.
   WinAlarm( HWND_DESKTOP, WA_ERROR );
   // ��室.
   DosKillThread( Thread_ID ); WinTerminate( Application ); return 0;
  }

 // ��⠭�������� �ਮ��� �ਫ������.
 DosSetPriority( PRTYS_PROCESS, PRTYC_IDLETIME, PRTYD_MAXIMUM / 2, 0 );

 // ���������� ��࠭�, � ���ன ࠡ�⠥� �ਫ������.
 System_metrics.Code_page = WinQueryCp( Message_queue );

 // ���������� ���, ��� ����� ��।����� ���� �ਫ������.
 CHAR AppWindow_name[] = "Mirrors";

 // ��।��塞 ���� � ��⥬�.
 UINT Window_is_registered = WinRegisterClass( Application, AppWindow_name, (PFNWP) WinProc, 0, 0 );
 // �᫨ �� ᤥ���� �� 㤠���� - ��室.
 if( !Window_is_registered )
  {
   // ��� - �訡��.
   WinAlarm( HWND_DESKTOP, WA_ERROR );
   // ��室.
   DosKillThread( Thread_ID );
   WinDestroyMsgQueue( Message_queue ); WinTerminate( Application ); return 0;
  }

 // ��� �㤥� �룫拉�� ࠬ�� ����.
 ULONG Frame_style = CS_FRAME | WS_ANIMATE;
 // ��� �㤥� �룫拉�� ࠡ��� ������� ����.
 ULONG Client_style = 0;
 // ���� �� � ���� ��������� � ������.
 ULONG Window_style = FCF_TITLEBAR | FCF_SYSMENU | FCF_MINBUTTON | FCF_MAXBUTTON | FCF_SIZEBORDER | FCF_TASKLIST;

 // ��ப� ��������� - ������ �� ��࠭�.
 CHAR AppWindow_title[ 10 ] = "";
 INT Strings_offset = 0; if( System_metrics.Code_page != RUSSIAN ) Strings_offset = 100;
 WinLoadString( Application, NULLHANDLE, Strings_offset + 1, sizeof( AppWindow_title ), AppWindow_title );

 // ������� ����.
 HWND Client_window = NULLHANDLE;
 HWND Frame_window = WinCreateStdWindow( HWND_DESKTOP, Frame_style, &Window_style, AppWindow_name, AppWindow_title, Client_style, 0, 0, &Client_window );
 // �᫨ ���� ᮧ���� �� 㤠���� - ��室.
 if( Frame_window == NULLHANDLE )
  {
   // ��� - �訡��.
   WinAlarm( HWND_DESKTOP, WA_ERROR );
   // ��室.
   DosKillThread( Thread_ID );
   WinDestroyMsgQueue( Message_queue ); WinTerminate( Application ); return 0;
  }

 // ������ ࠧ��� �࠭�.
 System_metrics.X_Screen = WinQuerySysValue( HWND_DESKTOP, SV_CXSCREEN );
 System_metrics.Y_Screen = WinQuerySysValue( HWND_DESKTOP, SV_CYSCREEN );

 // ����塞 ࠧ���� ��ઠ�.
 RECTL Rectangle = { 0, 0, System_metrics.X_Screen, System_metrics.Y_Screen };
 CalculateMirrorSize( Rectangle );

 // �뤥�塞 ������ ��� ��ઠ�.
 Left_mirror = new MIRROR_SQUARES[ Mirror_size.Width * Mirror_size.Height ];
 Right_mirror = new MIRROR_SQUARES[ Mirror_size.Width * Mirror_size.Height ];

 // �᫨ ������ �뤥���� �� 㤠���� - ��室.
 if( Left_mirror == NULL || Right_mirror == NULL )
  {
   // ��� - �訡��.
   WinAlarm( HWND_DESKTOP, WA_ERROR );
   // ��室.
   WinDestroyWindow( Frame_window ); DosKillThread( Thread_ID );
   WinDestroyMsgQueue( Message_queue ); WinTerminate( Application ); return 0;
  }

 // ������� ����࠭�⢮ �⮡ࠦ���� � �����.
 DEVOPENSTRUC Display = { 0, "Display", NULL, 0, 0, 0, 0, 0, 0 }; SIZEL Size = { 0, 0 };
 HDC Device = DevOpenDC( Application, OD_MEMORY, "*", 5, (PDEVOPENDATA) &Display, NULLHANDLE );
 Memory_space = GpiCreatePS( Application, Device, &Size, PU_PELS | GPIA_ASSOC );
 DevCloseDC( Device );

 // �᫨ ����࠭�⢮ �⮡ࠦ���� ᮧ���� �� 㤠���� - ��室.
 if( Memory_space == GPI_ERROR )
  {
   // ��� - �訡��.
   WinAlarm( HWND_DESKTOP, WA_ERROR );
   // ��室.
   delete Left_mirror; delete Right_mirror;
   WinDestroyWindow( Frame_window ); DosKillThread( Thread_ID );
   WinDestroyMsgQueue( Message_queue ); WinTerminate( Application ); return 0;
  }

 // ����᪠�� ���稪 ��砩��� �ᥫ.
 INT Time = WinGetCurrentTime( Application ); srand( Time );

 // ������ ��ઠ�� ����묨 � �ᯮ������ �� �����孮�� ��᪮�쪮 �।��⮢.
 CleanMirrors(); ThrowObjects();

 // ����㦠�� ���⨭��, �⮡� ��⠭����� �� � ����� ���孥� 㣫� ����.
 HPOINTER Window_icon = WinLoadPointer( HWND_DESKTOP, NULL, RC_ICON );

 // �᫨ ��� ���� - ���������� ��, ���� �ᯮ��㥬 ������ ���⨭��.
 BYTE Icon_is_loaded = 0;
 if( Window_icon != NULLHANDLE ) Icon_is_loaded = 1;
 else Window_icon = WinQuerySysPointer( HWND_DESKTOP, SPTR_DISPLAY_PTRS, 0 );

 // ��⠭�������� ���⨭�� � ����� ���孥� 㣫� ����.
 WinSendMsg( Frame_window, WM_SETICON, (MPARAM) Window_icon, 0 );

 // ����㦠�� ��ப� ��� ����.
 for( INT Count = 2; Count <= 4; Count ++ )
  WinLoadString( Application, NULLHANDLE, Strings_offset + Count, sizeof( Strings_in_window.Menu_strings[ Count - 2 ] ), Strings_in_window.Menu_strings[ Count - 2 ] );

 // ������塞 ��ப� � ����, ���஥ ��뢠���� �� ����⨨ �� ���⨭��.
 AddMenu( Frame_window );

 // ����㦠�� ����ࠦ����.
 HPS Presentation_space = WinGetPS( Client_window );
 // ����㦠�� ����ࠦ���� �।��⮢. �।��� 0 - ���⮩ ������.
 Sprite[ 0 ].Image = NULLHANDLE;
 for( Count = 1; Count <= MAX_SPRITE; Count ++ )
  {
   // ����㦠�� ����ࠦ����.
   Sprite[ Count ].Image = GpiLoadBitmap( Presentation_space, NULL, RC_SPRITES + Count - 1, 0, 0 );
   // ����㦠�� �᭮��.
   Sprite[ Count ].Base =  GpiLoadBitmap( Presentation_space, NULL, RC_BASES + Count - 1, 0, 0 );
  }
 // ����㦠�� ���⨭��, ����� �㤥� �ᯮ�짮���� � ����.
 Background.Bitmap = GpiLoadBitmap( Presentation_space, NULL, RC_BITMAPS + Rnd( 1 ), 0, 0 );
 WinReleasePS( Presentation_space );

 // ���������� ࠧ���� ���⨭��.
 BITMAPINFOHEADER Bitmap_info; GpiQueryBitmapParameters( Background.Bitmap, &Bitmap_info );
 Background.Bitmap_width = Bitmap_info.cx; Background.Bitmap_height = Bitmap_info.cy;

 // �����⠢������ ���⨭�� � �����.
 CreateBackgroundTile();

 // ����㦠�� ��ப� - �������� ����.
 WinLoadString( Application, NULLHANDLE, Strings_offset + 5, sizeof( Strings_in_window.Title ), Strings_in_window.Title );
 // ���ᠭ�� � �ࠢ���.
 for( Count = 6; Count <= 11; Count ++ )
  WinLoadString( Application, NULLHANDLE, Strings_offset + Count, sizeof( Strings_in_window.Description[ Count - 6 ] ), Strings_in_window.Description[ Count - 6 ] );
 // ���ࠡ��稪.
 WinLoadString( Application, NULLHANDLE, Strings_offset + 12, sizeof( Strings_in_window.Developer ), Strings_in_window.Developer );

 // ������砥� MMOS2;
 INT MMOS2_is_loaded = LoadMMOS2();

 // ��७�ᨬ ��㪮�� 䠩�� �� �६���� ��⠫�� � ������ ����� 䠩���.
 if( MMOS2_is_loaded ) PrepareSoundFiles();

 // ������ �ᯮ������� ���� �� �࠭�.
 INT X_position = 1; INT Y_position = 1;
 INT X_size = System_metrics.X_Screen - 2;
 INT Y_size = System_metrics.Y_Screen - 2;
 WinSetWindowPos( Frame_window, HWND_TOP, X_position, Y_position, X_size, Y_size, SWP_SIZE | SWP_MOVE | SWP_ZORDER | SWP_NOADJUST );

 // ���� �㤥� 㢥��祭� �� ���� �࠭.
 WinSendMsg( Frame_window, WM_SYSCOMMAND, (MPARAM) SC_MAXIMIZE, 0 );

 // ��뢠�� ���� �ਫ������ � ������ ��� ��࠭��.
 WinShowWindow( Frame_window, 1 ); WinSetActiveWindow( HWND_DESKTOP, Frame_window );

 // �㤥� ����㦠���� ��� - ������塞 ���� �ਫ������.
 WinUpdateWindow( Client_window );

 // ���, ��������, �� �� �����⮢���. ���� 1/4 ᥪ㭤�.
 DosSleep( 250 );

 // ��� - ���⠢��.
 WinPostMsg( Client_window, WM_PLAYSOUND, (MPARAM) SOUND_BEGIN, 0 );

 // ����祭�� � ��।�� ᮮ�饭�� ���� �ਫ������ - ���� ��� �� �㤥� ������.
 QMSG Message = { 0, 0, 0, 0, 0, 0, 0 };
 while( WinGetMsg( Application, &Message, 0, 0, 0 ) ) WinDispatchMsg( Application, &Message );

 // ��⠭�������� ���稪 �६���.
 WinStopTimer( Application, Client_window, 1 );

 // ����뢠�� ����.
 WinDestroyWindow( Frame_window );

 // ��� - �����襭��.
 if( Settings.Sound_is_enabled ) PlayAudioFile( Sound.Sound_file[ SOUND_END ] );

 // ����塞 ��㪮�� 䠩�� �� �६����� ��⠫���.
 if( MMOS2_is_loaded ) DeleteSoundFiles();

 // �⪫�砥� MMOS2.
 if( MMOS2_is_loaded ) UnloadMMOS2();

 // ����뢠�� ����ࠦ����, ���஥ �ᯮ�짮������ � ����.
 if( Background.Bitmap != NULLHANDLE ) GpiDeleteBitmap( Background.Bitmap );
 if( Background.Tile != NULLHANDLE ) GpiDeleteBitmap( Background.Tile );

 // ����뢠�� ���⨭��, ����� �뫠 ��⠭������ � ����� ���孥� 㣫� ����.
 if( Icon_is_loaded ) WinDestroyPointer( Window_icon );

 // ����뢠�� ����࠭�⢮ �⮡ࠦ���� � �����.
 GpiDestroyPS( Memory_space );

 // �᢮������� ������ ��� ��ઠ�.
 delete Left_mirror; delete Right_mirror;

 // �����蠥� ࠡ��� ��⮪�.
 DosKillThread( Thread_ID );

 // ����뢠�� ��।� ᮮ�饭��.
 WinDestroyMsgQueue( Message_queue );

 // ��室.
 WinTerminate( Application ); return 0;
}

// ��� �����頥� ��砩��� �᫮ �� 0 �� Max ���

INT Rnd( INT Max )
{
 // �᫨ �᫮ ����� �����⨬��� ���祭�� - ������.
 if( Max > RAND_MAX - 1 ) return -1;

 // ����砥� ��砩��� �᫮ �� 0 �� 1.
 INT Integer_value = rand();

 // ��᫮ ������ ���� �� 0 �� Max.
 INT Divider = RAND_MAX / ( Max + 1 );

 // ����砥� �᫮.
 INT Result = Integer_value / Divider;
 if( Result > Max ) Result = Max;

 // �����頥� �᫮.
 return Result;
}

// ��� ��७��� ��㪮�� 䠩�� �� �६���� ��⠫��, ������ ����� 䠩��� ���

// ����� ᦠ�� � ��娢, ����� ����� ࠧ�������� ᠬ����⥫쭮.
// ����� ������ ���� ��⠭������ � ��६����� Sound.Sound_file[].
VOID PrepareSoundFiles( VOID )
{
 // ����� ��㪮��� 䠩��� � ��㪨 � ����� ����।�����.
 for( INT Count = 0; Count <= MAX_SOUND; Count ++ )
  {
   Sound.Sound_file[ Count ][ 0 ] = NULL;
   Sound.Sound_in_memory[ Count ] = NULLHANDLE;
  }

 // ������ ᢮� ࠡ�稩 ��⠫��.
 ULONG Current_drive = 0; ULONG Drive_map = 0;
 DosQueryCurrentDisk( &Current_drive, &Drive_map );
 CHAR Current_directory[ 255 ] = ""; ULONG Length = 250;
 Current_directory[ 0 ] = (CHAR) Current_drive + 64;
 Current_directory[ 1 ] = ':'; Current_directory[ 2 ] = '\\';
 DosQueryCurrentDir( 0, &Current_directory[ 3 ], &Length );

 // ������ �६���� ��⠫��. �� ������ ���� 㪠��� � ��६����� �।�.
 PSZ Temp_directory = "";
 INT Result = DosScanEnv( "TEMP", (PCSZ*) &Temp_directory );
 if( Result != 0 ) DosScanEnv( "TMP", (PCSZ*) &Temp_directory );
 if( Result != 0 ) DosScanEnv( "TEMPDIR", (PCSZ*) &Temp_directory );
 if( Result != 0 ) DosScanEnv( "TMPDIR", (PCSZ*) &Temp_directory );

 // �᫨ �६���� ��⠫�� �� 㪠��� - ������.
 if( Result != 0 ) return;

 // �᫨ �६���� ��⠫�� �����蠥��� �㪢�� "\" - �⠢�� ����� ��� ����� ��ப�.
 // �᫨ �� ��७� ��᪠ - ��⠥��� �㪢� ��᪠ � �����稥, ���ਬ��, "C:".
 INT End_of_string = strlen( Temp_directory ) - 1;
 if( Temp_directory[ End_of_string ] == '\\' ) Temp_directory[ End_of_string ] = NULL;

 // ���室�� �� �६���� ��⠫��.
 ULONG Temp_drive = (ULONG) Temp_directory[ 0 ] - 64;
 DosSetDefaultDisk( Temp_drive ); DosSetCurrentDir( Temp_directory );

 // ������ �������� ��㪮��� 䠩���.
 for( Count = 0; Count <= MAX_SOUND; Count ++ ) strcpy( Sound.Sound_file[ Count ], Temp_directory );
 strcat( Sound.Sound_file[ SOUND_BEGIN ], "\\MG_Begin.wav" );
 strcat( Sound.Sound_file[ SOUND_GAME ], "\\MG_Game.wav" );
 strcat( Sound.Sound_file[ SOUND_PAUSE ], "\\MG_Pause.wav" );
 strcat( Sound.Sound_file[ SOUND_MOUSE ], "\\MG_Mouse.wav" );
 strcat( Sound.Sound_file[ SOUND_DELETE ], "\\MG_Del.wav" );
 strcat( Sound.Sound_file[ SOUND_OVER ], "\\MG_Over.wav" );
 strcat( Sound.Sound_file[ SOUND_END ], "\\MG_End.wav" );

 // �� 䠩�� 㦥 ����� ���� �� �६����� ��⠫���. ����塞 ��.
 DeleteSoundFiles();

 // ������ ��� ��娢� � ��㪮�. �� �ᯮ����� � ��⠫��� �ਫ������.
 CHAR Sound_archive[ 255 ] = ""; strcpy( Sound_archive, Current_directory );
 // �� ����� ���� ��७� ��� ��⠫�� ��᪠.
 if( strlen( Sound_archive ) > 3 ) strcat( Sound_archive, "\\" );
 strcat( Sound_archive, "Mirrors.snd" );

 // ��뢠�� ��娢 � ��㪮� ��� �ਫ������. �� ࠧ���稢����� �� �६���� ��⠫��.
 CHAR Error_string[ 1 ]; RESULTCODES Return_codes = { 0, 0 };
 DosExecPgm( Error_string, sizeof( Error_string ), EXEC_BACKGROUND, NULL, NULL, &Return_codes, Sound_archive );

 // ���������� �맢����� �ਫ������.
 Sound.Sound_application = Return_codes.codeTerminate;

 // ���室�� � ᢮� ࠡ�稩 ��⠫��.
 DosSetDefaultDisk( Current_drive ); DosSetCurrentDir( Current_directory );

 // ������.
 return;
}

// ��� ������ ��㪮�� 䠩�� �� �६����� ��⠫��� ���

// ����� 䠩��� ��⠭������ � ��६����� Sound.Sound_file[].
VOID DeleteSoundFiles( VOID )
{
 // �᫨ ��㪮��� 䠩��� ��� - ������.
 if( Sound.Sound_file[ 0 ][ 0 ] == NULL ) return;

 // ��⠭�������� �맢����� �ਫ������, �� ��娢 � ��㪮�.
 DosKillProcess( DKP_PROCESS, Sound.Sound_application );

 // ����塞 ��㪮�� 䠩�� �� �६����� ��⠫���.
 for( INT Count = 0; Count <= MAX_SOUND; Count ++ )
  {
   // �᫨ ��� ����㦥� � ������ - �᢮������� ��.
   if( Sound.Sound_in_memory[ Count ] != NULLHANDLE ) DeleteSound( Sound.Sound_in_memory[ Count ] );

   // ����뢠�� ��� 䠩�� ᢮��⢮ "���쪮 ��� �⥭��".
   FILESTATUS3 File_status = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
   DosSetPathInfo( Sound.Sound_file[ Count ], FIL_STANDARD, &File_status, sizeof( File_status ), DSPI_WRTTHRU );

   // ����塞 䠩�.
   DosForceDelete( Sound.Sound_file[ Count ] );
  }

 // ������.
 return;
}

// ��� �����⠢������ � ����� ����ࠦ����, ���஥ �ᯮ������ � ���� ���

// ����ࠦ���� ��������� ���� ࠧ� - ⠪, �⮡� ��� ����� �뫮 ���ᮢ���, ��稭��
// � �� �窨. �� ���� ��� ⮣�, �⮡� ����⠭�������� ��� �� �������� �।��⮢.
VOID CreateBackgroundTile( VOID )
{
 // ������ ࠧ���� ����ࠦ����. �� ������ ���� ࠧ��� ���⨭�� � ���� + ����ﭨ�,
 // �� ���஥ �।��� ����� ��室��� �� ����, � ���� ࠧ��� ���⨭�� ��� �।���.
 INT Width = Background.Bitmap_width + SPRITE_WIDTH;
 INT Height = Background.Bitmap_height + SPRITE_HEIGHT;

 // ������ ��� ���ன�⢠.
 LONG Format[ 2 ]; GpiQueryDeviceBitmapFormats( Memory_space, 2, Format );

 // ����� ��� ⮣�, �⮡� ᮧ���� ����ࠦ����.
 BITMAPINFOHEADER2 Header;
 Header.cbFix = (ULONG) sizeof( BITMAPINFOHEADER2 ); Header.cx = Width; Header.cy = Height;
 Header.cPlanes = (SHORT) Format[ 0 ]; Header.cBitCount = (SHORT) Format[ 1 ]; Header.ulCompression = BCA_UNCOMP;
 Header.cbImage = ( ( ( Width * ( 1 << Header.cPlanes ) * ( 1 << Header.cBitCount ) ) + 31 ) / 32 ) * Height;
 Header.cxResolution = 70; Header.cyResolution = 70; Header.cclrUsed = 2; Header.cclrImportant = 0;
 Header.usUnits = BRU_METRIC; Header.usReserved = 0; Header.usRecording = BRA_BOTTOMUP; Header.usRendering = BRH_NOTHALFTONED;
 Header.cSize1 = 0; Header.cSize2 = 0; Header.ulColorEncoding = BCE_RGB; Header.ulIdentifier = 0;

 // ��������� ����ࠦ���� - ��⠥��� � �����.
 PBITMAPINFO2 Information = NULLHANDLE;
 DosAllocMem( (PPVOID) &Information, sizeof( BITMAPINFO2 ) + ( sizeof( RGB2 ) * ( 1 << Header.cPlanes ) * ( 1 << Header.cBitCount ) ), PAG_COMMIT | PAG_READ | PAG_WRITE );
 Information->cbFix = Header.cbFix; Information->cx = Header.cx; Information->cy = Header.cy;
 Information->cPlanes = Header.cPlanes; Information->cBitCount = Header.cBitCount; Information->ulCompression = BCA_UNCOMP;
 Information->cbImage = ( ( Width + 31 ) / 32 ) * Height; Information->cxResolution = 70; Information->cyResolution = 70;
 Information->cclrUsed = 2; Information->cclrImportant = 0; Information->usUnits = BRU_METRIC;
 Information->usReserved = 0; Information->usRecording = BRA_BOTTOMUP; Information->usRendering = BRH_NOTHALFTONED;
 Information->cSize1 = 0; Information->cSize2 = 0; Information->ulColorEncoding = BCE_RGB; Information->ulIdentifier = 0;

 // ������� ����ࠦ����.
 Background.Tile = GpiCreateBitmap( Memory_space, &Header, FALSE, NULL, Information );

 // ������塞 ��� ⠪ ��, ��� ������ ���� ��������� ����.
 if( Background.Bitmap != NULLHANDLE )
  {
   GpiSetBitmap( Memory_space, Background.Tile );
   for( INT X_Count = 0; X_Count <= Width; X_Count += Background.Bitmap_width )
    for( INT Y_Count = 0; Y_Count <= Height; Y_Count += Background.Bitmap_height )
     {
      // ���㥬 ���⨭��.
      POINTL Point = { X_Count, Y_Count };
      WinDrawBitmap( Memory_space, Background.Bitmap, NULL, &Point, 0, 0, DBM_NORMAL );
     }
  }

 // ������.
 return;
}

// ��� ����� ����ࠦ���� �।��� ���

// Presentation_space - ����࠭�⢮ �⮡ࠦ����, X � Y - �窠, Sprite - �।���.
VOID DrawSprite( HPS Presentation_space, INT X, INT Y, SPRITE Sprite )
{
 // �᫨ �� ���⮩ ������ - ������.
 if( Sprite.Image == NULLHANDLE ) return;

 // ���������� 梥� � ����࠭�⢥ �⮡ࠦ����.
 COLOR Old_color = GpiQueryColor( Presentation_space );
 COLOR Old_background_color = GpiQueryBackColor( Presentation_space );

 // ������ ���� 梥�.
 GpiSetColor( Presentation_space, CLR_WHITE );
 GpiSetBackColor( Presentation_space, CLR_BLACK );

 // ������ �ᯮ������� ���⨭��.
 POINTL Points[ 4 ] = { X, Y, X + SPRITE_WIDTH, Y + SPRITE_HEIGHT, 0, 0, SPRITE_WIDTH, SPRITE_HEIGHT };

 // ���㥬 �᭮��.
 GpiSetBitmap( Memory_space, Sprite.Base );
 GpiBitBlt( Presentation_space, Memory_space, 4, Points, ROP_SRCAND, BBO_IGNORE );

 // ���㥬 ���⨭��.
 GpiSetBitmap( Memory_space, Sprite.Image );
 GpiBitBlt( Presentation_space, Memory_space, 4, Points, ROP_SRCPAINT, BBO_IGNORE );

 // ����⠭�������� 梥� � ����࠭�⢥ �⮡ࠦ����.
 GpiSetColor( Presentation_space, Old_color );
 GpiSetBackColor( Presentation_space, Old_background_color );

 // ������.
 return;
}

// ��� ���뢠�� �।���, ����⠭������� ���� ���⨭�� � ���� ���

// Presentation_space - ����࠭�⢮ �⮡ࠦ����, X � Y - �窠.
VOID HideSprite( HPS Presentation_space, INT X, INT Y )
{
 // ������, ᪮�쪮 ����ࠦ���� ���⨭�� � ���� ���� ���ᮢ��� �� �窨 ( X, Y ).
 INT X_Bitmaps = X / Background.Bitmap_width;
 INT Y_Bitmaps = Y / Background.Bitmap_height;          //       �
 // ������ ���, � ���ன ��� �����稢�����.          //     **�***
 INT X_Node = X_Bitmaps * Background.Bitmap_width;      //     **�***
 INT Y_Node = Y_Bitmaps * Background.Bitmap_height;     // ����*���**���
 // ����塞 ����ﭨ� �� �⮩ �窨 �� ( X, Y ).     //     **�***
 INT X_Distance = X - X_Node;                           //       �
 INT Y_Distance = Y - Y_Node;                           // // // // // // // //

 // ���㥬 ���� ���⨭��, ����� �뫠 �����⮢���� � �����.
 POINTL Point = { X, Y };
 RECTL Part_of_bitmap = { X_Distance, Y_Distance, X_Distance + SPRITE_WIDTH, Y_Distance + SPRITE_HEIGHT };
 WinDrawBitmap( Presentation_space, Background.Tile, &Part_of_bitmap, &Point, 0, 0, DBM_NORMAL );

 // ������.
 return;
}

// ��� �ਭ����� ᮮ�饭��, ����� ��室�� � ���� �ਫ������ ���

// OS/2 ��뢠�� WinProc ��直� ࠧ, ����� ��� ���� ���� ᮮ�饭��.
// Client_window ��।���� ����, Message - ᮮ�饭��, *_parameter -
// �����, ����� ��।����� ����� � ᮮ�饭���.
MRESULT EXPENTRY WinProc( HWND Client_window, ULONG Message, MPARAM First_parameter, MPARAM Second_parameter )
{
 // ����饭�� � ⮬, �� ���� ������ ���� ����ᮢ���.
 if( Message == WM_PAINT )
  {
   // ����⠥� � ����࠭�⢥ �⮡ࠦ���� ����.
   RECTL Rectangle = { 0, 0, 0, 0 };
   HPS Presentation_space = WinBeginPaint( Client_window, NULLHANDLE, &Rectangle );
   // ����訢��� ���� ��� 梥⮬.
   WinFillRect( Presentation_space, &Rectangle, CLR_BLACK );
   // �����蠥� ࠡ��� � ����࠭�⢥ �⮡ࠦ���� ����.
   WinEndPaint( Presentation_space );

   // �᫨ �।��饥 ᮮ�饭�� �뫮 �ਭ��:
   if( Thread_responds.Draw_message_is_received )
    {
     // ����뢠�� ��६�����.
     Thread_responds.Draw_message_is_received = 0;

     // ��।��� ᮮ�饭�� � ��⮪. ��६����� First_parameter ��।���� ���� �ਫ������.
     WinPostQueueMsg( Thread_message_queue, WM_DRAW, (MPARAM) Client_window, 0 );
    }

   // ������.
   return 0;
  }

 // ����饭�� �� ���稪� �६���.
 if( Message == WM_TIMER )
  {
   // �᫨ �।��饥 ᮮ�饭�� �뫮 �ਭ��:
   if( Thread_responds.Timer_message_is_received )
    {
     // ����뢠�� ��६�����.
     Thread_responds.Timer_message_is_received = 0;

     // ��।��� ᮮ�饭�� � ��⮪. First_parameter - ���� �ਫ������, Second_parameter - ���稪 �६���.
     WinPostQueueMsg( Thread_message_queue, WM_TIMER, (MPARAM) Client_window, First_parameter );
    }

   // ������.
   return 0;
  }

 // ����⨥ ������.
 if( Message == WM_CHAR )
  {
   // ����ਬ, ����� ������ �����.
   SHORT Key = CHARMSG( &Message ) -> vkey;

   // �᫨ ����� 'F1' - �맮� �㪮����⢠.
   if( Key == VK_F1 ) WinPostQueueMsg( Thread_message_queue, WM_GUIDE, 0, 0 );

   // ��⠫�� ������.
   if( !( SHORT1FROMMP( First_parameter ) & KC_KEYUP ) )
    {
     // �᫨ �������� ���⠢�� - ���室�� � ����.
     if( Settings.Game_mode == 0 )
      if( Key != VK_F1 ) if( Key != VK_ESC ) if( Key != VK_INSERT ) if( Key != VK_PAUSE )
       {
        // ���室�� � ����.
        Settings.Game_mode = 1;

        // ��� - ���.
        WinPostMsg( Client_window, WM_PLAYSOUND, (MPARAM) SOUND_GAME, 0 );

        // ����᪠�� ���稪 �६���.
        WinStartTimer( Application, Client_window, 1, TIMER_RATE );

        // ���� ������ ���� ����ᮢ���. ��।��� ᮮ�饭�� � ��⮪.
        Thread_responds.Draw_message_is_received = 0;
        WinPostQueueMsg( Thread_message_queue, WM_DRAW, (MPARAM) Client_window, 0 );
       }

     // ��५��, �஡�� � Tab - ��।������� �।��⮢.
     if( Settings.Game_mode == 2 ) if( Key == VK_LEFT )
      WinPostQueueMsg( Thread_message_queue, WM_LEFT, (MPARAM) Client_window, 0 );

     if( Settings.Game_mode == 2 ) if( Key == VK_RIGHT )
      WinPostQueueMsg( Thread_message_queue, WM_RIGHT, (MPARAM) Client_window, 0 );

     if( Settings.Game_mode == 2 ) if( Key == VK_DOWN || Key == VK_SPACE )
      WinPostQueueMsg( Thread_message_queue, WM_DOWN, (MPARAM) Client_window, 0 );

     if( Settings.Game_mode == 2 ) if( Key == VK_TAB )
      WinPostQueueMsg( Thread_message_queue, WM_BOUNCE, (MPARAM) Client_window, 0 );

     // Pause - ��⠭���� ��� �த������� ����.
     if( Settings.Game_mode == 1 || Settings.Game_mode == 2 ) if( Key == VK_PAUSE )
      {
       // ��⠭���� ��� �த������� ����.
       if( Settings.Game_is_paused ) Settings.Game_is_paused = 0;
       else Settings.Game_is_paused = 1;

       // ��� - ��⠭���� ����.
       WinPostMsg( Client_window, WM_PLAYSOUND, (MPARAM) SOUND_PAUSE, 0 );
      }

     // �᫨ ����� Insert - �����뢠�� ��� ��뢠�� ���.
     if( Key == VK_INSERT )
      {
       // �����뢠�� ��� ��뢠�� ���.
       if( Settings.Grid_is_visible ) Settings.Grid_is_visible = 0;
       else Settings.Grid_is_visible = 1;

       // ��� - ��� �� ����⨨ ������ ���.
       WinPostMsg( Client_window, WM_PLAYSOUND, (MPARAM) SOUND_MOUSE, 0 );

       // ���� ������ ���� ����ᮢ���. ��।��� ᮮ�饭�� � ��⮪.
       Thread_responds.Draw_message_is_received = 0;
       WinPostQueueMsg( Thread_message_queue, WM_DRAW, (MPARAM) Client_window, 0 );
      }

     // �᫨ ����� 'Esc' - ��室.
     if( Key == VK_ESC )
      {
       // ������ ࠬ�� ���� �ਫ������.
       HWND Frame_window = WinQueryWindow( Client_window, QW_PARENT );
       // ���뢠�� ���� - �� �맢����� ᯨ᪥ ���� ࠡ�稩 �⮫ �� ����ᮢ뢠����.
       WinSetWindowPos( Frame_window, HWND_BOTTOM, 0, 0, 0, 0, SWP_SIZE | SWP_MOVE | SWP_ZORDER | SWP_HIDE | SWP_NOADJUST );
       // ����뢠�� ����.
       WinPostMsg( Client_window, WM_CLOSE, 0, 0 );
      }
    }

   // ������.
   return 0;
  }

 // ����⨥ ������ ���:
 if( Message == WM_BUTTON1DOWN || Message == WM_BUTTON1DBLCLK ||
     Message == WM_BUTTON2DOWN || Message == WM_BUTTON2DBLCLK ||
     Message == WM_BUTTON3DOWN || Message == WM_BUTTON3DBLCLK )
  {
   // �᫨ �������� ���⠢�� - ���室�� � ����.
   if( Settings.Game_mode == 0 ) if( Message == WM_BUTTON1DOWN || Message == WM_BUTTON1DBLCLK )
    {
     // ���室�� � ����.
     Settings.Game_mode = 1;

     // ��� - ���.
     WinPostMsg( Client_window, WM_PLAYSOUND, (MPARAM) SOUND_GAME, 0 );

     // ����᪠�� ���稪 �६���.
     WinStartTimer( Application, Client_window, 1, TIMER_RATE );

     // ���� ������ ���� ����ᮢ���. ��।��� ᮮ�饭�� � ��⮪.
     Thread_responds.Draw_message_is_received = 0;
     WinPostQueueMsg( Thread_message_queue, WM_DRAW, (MPARAM) Client_window, 0 );
    }

   // �᫨ �� �६� ���� ����� ����� ������ ��� - ��।������ �।����.
   if( Settings.Game_mode == 2 ) if( Message == WM_BUTTON1DOWN || Message == WM_BUTTON1DBLCLK )
    {
     // ���뫠�� ᮮ�饭�� � ��⮪, *parameter - �窠, � ���ன �ᯮ����� 㪠��⥫� ���.
     POINTS Point; Point.x = MOUSEMSG( &Message ) -> x; Point.y = MOUSEMSG( &Message ) -> y;
     WinPostQueueMsg( Thread_message_queue, WM_MOUSE, (MPARAM) Point.x, (MPARAM) Point.y );
    }

   // �᫨ ����� �ࠢ�� ������ ��� - �����뢠�� ��� ��뢠�� ���.
   if( Message == WM_BUTTON2DOWN || Message == WM_BUTTON2DBLCLK ||
       Message == WM_BUTTON3DOWN || Message == WM_BUTTON3DBLCLK )
    {
     // �����뢠�� ��� ��뢠�� ���.
     if( Settings.Grid_is_visible ) Settings.Grid_is_visible = 0;
     else Settings.Grid_is_visible = 1;

     // ��� - ����⨥ ������ ���.
     WinPostMsg( Client_window, WM_PLAYSOUND, (MPARAM) SOUND_MOUSE, 0 );

     // ���� ������ ���� ����ᮢ���. ��।��� ᮮ�饭�� � ��⮪.
     Thread_responds.Draw_message_is_received = 0;
     WinPostQueueMsg( Thread_message_queue, WM_DRAW, (MPARAM) Client_window, 0 );
    }

   // ��������, ���� ������ ���� ��࠭��. ��।��� ᮮ�饭�� OS/2 �� ��ࠡ���.
   WinDefWindowProc( Client_window, Message, First_parameter, Second_parameter );

   // ������.
   return 0;
  }

 // �롮� ��ப� ����.
 if( Message == WM_COMMAND )
  {
   // ����ਬ, ����� ��ப� ��࠭�.
   SHORT Item = LOUSHORT( First_parameter );

   // ��⠭���� ��㪠.
   if( Item == SOUND_MENU_ITEM )
    {
     // ���塞 ����ன��.
     if( Settings.Sound_is_enabled )
      {
       // ��� - ��� �� ����⨨ ������ ���.
       WinSendMsg( Client_window, WM_PLAYSOUND, (MPARAM) SOUND_MOUSE, 0 );

       // �⪫�砥� ���.
       Settings.Sound_is_enabled = 0;
      }
     else
      {
       // ����砥� ���.
       Settings.Sound_is_enabled = 1;
      }

     // ������ ࠬ�� ���� �ਫ������.
     HWND Frame_window = WinQueryWindow( Client_window, QW_PARENT );
     // �⬥砥� ��ப� ���� ��� ᭨���� �⬥⪨.
     CheckMenuItems( Frame_window );
    }

   // ��⠭���� ᫮�����.
   if( Item == DIFF_MENU_ITEM )
    {
     // ���塞 ����ன��.
     if( Settings.Game_is_difficult ) Settings.Game_is_difficult = 0;
     else Settings.Game_is_difficult = 1;

     // ������ ࠬ�� ���� �ਫ������.
     HWND Frame_window = WinQueryWindow( Client_window, QW_PARENT );
     // �⬥砥� ��ப� ���� ��� ᭨���� �⬥⪨.
     CheckMenuItems( Frame_window );
    }

   // �맮� �㪮����⢠.
   if( Item == HELP_MENU_ITEM ) WinPostQueueMsg( Thread_message_queue, WM_GUIDE, 0, 0 );

   // ��� - ��� �� ����⨨ ������ ���.
   WinPostMsg( Client_window, WM_PLAYSOUND, (MPARAM) SOUND_MOUSE, 0 );

   // ������.
   return 0;
  }

 // ���ந�������� ��㪠. First_parameter ��।���� ���, ����� ���� ���ந�����.
 if( Message == WM_PLAYSOUND )
  {
   // �᫨ ��� �� ���ந�������� � �� �६� � �� ����祭 - ���ந������ ���.
   if( !Sound.Sound_is_playing ) if( Settings.Sound_is_enabled )
    {
     // ����㦠�� ��� � ������ � ���������� ���.
     INT Playing_sound = (INT) First_parameter;
     Sound.Sound_in_memory[ Playing_sound ] = LoadSound( Sound.Sound_file[ Playing_sound ] );

     // ���ந������ ���.
     INT Success = PlaySound( Sound.Sound_in_memory[ Playing_sound ], Client_window );

     // �᫨ ��� ���ந�������� - ���������� ��.
     if( Success ) Sound.Sound_is_playing = 1;
    }

   // ������.
   return 0;
  }

 // ����饭�� � ⮬, �� ��� ���ந������. Second_parameter ��।���� ���.
 if( Message == MM_MCINOTIFY )
  {
   // �᢮������� ������.
   HSOUND Played_sound = SHORT1FROMMP( Second_parameter );
   DeleteSound( Played_sound );

   // ����������, �� ��� ����� �� �������� ������.
   for( INT Count = 0; Count <= MAX_SOUND; Count ++ )
    if( Sound.Sound_in_memory[ Count ] == Played_sound )
     Sound.Sound_in_memory[ Count ] = NULLHANDLE;

   // ����������, �� ��� ����� �� ���ந��������.
   Sound.Sound_is_playing = 0;

   // ������.
   return 0;
  }

 // ���ᮢ�� ���� - �����頥� "��".
 if( Message == WM_ERASEBACKGROUND ) return (MRESULT) 1;

 // ��㣮� ᮮ�饭�� - �� ��ࠡ��뢠����.
 return WinDefWindowProc( Client_window, Message, First_parameter, Second_parameter );
}

// ��� ��⮪ ��� �ᮢ���� ���

// ��⮪ ࠡ�⠥� ��� �⤥�쭮� �ਫ������ � ����� �⤥���� ��।� ᮮ�饭��.
// � ��⮪ ��।����� ᮮ�饭��, ����� WinProc() ����砥� ��� ���� �ਫ������.
// ��⮪ �����蠥��� �� �����襭�� ࠡ��� �ਫ������.
VOID ThreadProc( VOID )
{
 // ��।��塞 ��⮪ � ��⥬�.
 HAB Thread = WinInitialize( 0 );
 // �᫨ �� ᤥ���� �� 㤠���� - ��室.
 if( Thread == NULLHANDLE )
  {
   // �� ᮧ����� ��⮪� �ந��諠 �訡��.
   Thread_responds.Thread_is_created = -1;
   // ��室.
   return;
  }

 // ������� ��।� ᮮ�饭�� ��� ��⮪�.
 Thread_message_queue = WinCreateMsgQueue( Thread, 0 );
 // �᫨ ��।� ᮧ���� �� 㤠���� - ��室.
 if( Thread_message_queue == NULLHANDLE )
  {
   // �����蠥� ࠡ��� ��⮪�.
   WinTerminate( Thread );

   // �� ᮧ����� ��⮪� �ந��諠 �訡��.
   Thread_responds.Thread_is_created = -1;
   // ��室.
   return;
  }

 // ��⮪ ᮧ��� �ᯥ譮.
 Thread_responds.Thread_is_created = 1;

 // ����祭�� � ��ࠡ�⪠ ᮮ�饭��, ��室��� � ��⮪.
 QMSG Message = { 0, 0, 0, 0, 0, 0, 0 };
 while( 1 )
  {
   // �롨ࠥ� ��।��� ᮮ�饭��.
   WinGetMsg( Thread, &Message, 0, 0, 0 );

   // ��ࠡ��뢠�� ᮮ�饭��.
   MessageProcessing( Message.msg, (MPARAM) Message.mp1, (MPARAM) Message.mp2 );
  }
}

// ��� ��ࠡ��稪 ᮮ�饭��, ����� �뫨 ��।��� � ��⮪ ���

// Message ��।���� ᮮ�饭��, *parameter - �������⥫�� ᢥ�����.
VOID MessageProcessing( ULONG Message, MPARAM First_parameter, MPARAM Second_parameter )
{
 // �᫨ ��室�� ᮮ�饭�� � ����ᮢ�� ����:
 if( Message == WM_DRAW )
  {
   // ����饭�� �뫮 �ਭ��.
   Thread_responds.Draw_message_is_received = 1;

   // ���㥬 �� � ����.
   HWND Client_window = (HWND) First_parameter;
   DrawImageInWindow( Client_window );

   // ������.
   return;
  }

 // �᫨ ��室�� ᮮ�饭�� �� ���稪� �६���:
 if( Message == WM_TIMER )
  {
   // ����饭�� �뫮 �ਭ��.
   Thread_responds.Timer_message_is_received = 1;

   // �᫨ ��� �� ��⠭������ - ��।������ �।����.
   if( !Settings.Game_is_paused )
    {
     HWND Client_window = (HWND) First_parameter;
     AnimateObjects( Client_window );
    }

   // ������.
   return;
  }

 // �᫨ ����� ��५��:
 if( Message >= WM_LEFT ) if( Message <= WM_BOUNCE )
  {
   // �᫨ ��� �� ��⠭������ - ��।������ �।����.
   if( !Settings.Game_is_paused )
    {
     // ��।������ �।����.
     HWND Client_window = (HWND) First_parameter;
     MoveObjects( Client_window, Message );
    }

   // ������.
   return;
  }

 // �᫨ ����� ������ ���:
 if( Message == WM_MOUSE )
  {
   // ��ࠡ��뢠�� ᮮ�饭��.
   if( !Settings.Game_is_paused )
    {
     INT X_Point = (INT) First_parameter; INT Y_Point = (INT) Second_parameter;
     TranslateMouseMessages( X_Point, Y_Point );
    }

   // ������.
   return;
  }

 // �맮� �㪮����⢠:
 if( Message == WM_GUIDE )
  {
   // ��뢠�� �㪮����⢮. ����� �㪮����⢠ ������ �� ��࠭�.
   CHAR Parameters[ 255 ]; Parameters[ 0 ] = 0;
   if( System_metrics.Code_page == RUSSIAN )
    strcpy( &Parameters[ 1 ], "/C View.exe Mirrors.inf ��ઠ��" );
   else
    strcpy( &Parameters[ 1 ], "/C View.exe Mirrors.inf Mirrors" );

   CHAR Error_string[ 1 ]; RESULTCODES Return_codes;
   DosExecPgm( Error_string, sizeof( Error_string ), EXEC_ASYNC, Parameters, NULL, &Return_codes, "Cmd.exe" );

   // ������.
   return;
  }
}

// ��� �������� ��ப� � ���� ���� ���

// Frame_window ��।���� ࠬ�� ����.
VOID AddMenu( HWND Frame_window )
{
 // ������ ���� ���⨭�� � ����� ���孥� 㣫� ����.
 HWND Picture_window = WinWindowFromID( Frame_window, FID_SYSMENU );

 // �᫨ ���⨭�� ��� - ������.
 if( Picture_window == NULLHANDLE ) return;

 // ������ ���� ����.
 SHORT Menu_ID = SHORT1FROMMP( WinSendMsg( Picture_window, MM_ITEMIDFROMPOSITION, MPFROMSHORT( 0 ), 0 ) );
 MENUITEM Menu_item = { 0, 0, 0, 0, 0, 0 }; WinSendMsg( Picture_window, MM_QUERYITEM, MPFROMSHORT( Menu_ID ), MPFROMP( &Menu_item ) );
 HWND Menu_window = Menu_item.hwndSubMenu;

 // ������ ࠧ����⥫��� �����.
 Menu_item.iPosition = MIT_END;
 Menu_item.afStyle = MIS_SEPARATOR;
 Menu_item.afAttribute = 0;
 Menu_item.id = -1;
 Menu_item.hwndSubMenu = NULLHANDLE;
 Menu_item.hItem = 0;

 // ������塞 ࠧ����⥫��� ����� � ����.
 WinSendMsg( Menu_window, MM_INSERTITEM, MPFROMP( &Menu_item ), 0 );

 // ������塞 ��ப� - ���.
 Menu_item.afStyle = MIS_TEXT;
 Menu_item.id = SOUND_MENU_ITEM;
 WinSendMsg( Menu_window, MM_INSERTITEM, MPFROMP( &Menu_item ), Strings_in_window.Menu_strings[ 0 ] );

 // ������塞 ��ப� - ����� ᫮��� �।����.
 Menu_item.id = DIFF_MENU_ITEM;
 WinSendMsg( Menu_window, MM_INSERTITEM, MPFROMP( &Menu_item ), Strings_in_window.Menu_strings[ 1 ] );

 // ������塞 ��ப� - �ࠢ��.
 Menu_item.id = HELP_MENU_ITEM;
 WinSendMsg( Menu_window, MM_INSERTITEM, MPFROMP( &Menu_item ), Strings_in_window.Menu_strings[ 2 ] );

 // �⠢�� �⬥⪨ � ��ப� ����.
 CheckMenuItems( Frame_window );

 // ������.
 return;
}

// ��� �⠢�� �⬥⪨ � ��ப� ���� ��� ᭨���� �� ���

// Frame_window ��।���� ࠬ�� ����.
VOID CheckMenuItems( HWND Frame_window )
{
 // ������ ���� ���⨭�� � ����� ���孥� 㣫� ����.
 HWND Picture_window = WinWindowFromID( Frame_window, FID_SYSMENU );

 // �᫨ ���⨭�� ��� - ������.
 if( Picture_window == NULLHANDLE ) return;

 // ������ ���� ����.
 SHORT Menu_ID = SHORT1FROMMP( WinSendMsg( Picture_window, MM_ITEMIDFROMPOSITION, MPFROMSHORT( 0 ), 0 ) );
 MENUITEM Menu_item = { 0, 0, 0, 0, 0, 0 }; WinSendMsg( Picture_window, MM_QUERYITEM, MPFROMSHORT( Menu_ID ), MPFROMP( &Menu_item ) );
 HWND Menu_window = Menu_item.hwndSubMenu;

 // �⠢�� �⬥�� � ��ப� "���".
 INT Attribute = 0; if( Settings.Sound_is_enabled ) Attribute = MIA_CHECKED;
 WinSendMsg( Menu_window, MM_SETITEMATTR, (MPARAM) SOUND_MENU_ITEM, MPFROM2SHORT( MIA_CHECKED, Attribute ) );

 // �⠢�� �⬥�� � ��ப� "������ �।����".
 Attribute = 0; if( Settings.Game_is_difficult ) Attribute = MIA_CHECKED;
 WinSendMsg( Menu_window, MM_SETITEMATTR, (MPARAM) DIFF_MENU_ITEM, MPFROM2SHORT( MIA_CHECKED, Attribute ) );

 // ������.
 return;
}

// ��� ������ ��ઠ�� ����묨 ���

VOID CleanMirrors( VOID )
{
 // ������ ��ઠ�� ����묨.
 for( INT Count = 0; Count < Mirror_size.Width * Mirror_size.Height; Count ++ )
  {
   // ������� ��祣� �� ᮤ�ঠ�.
   Left_mirror[ Count ].Object = 0; Right_mirror[ Count ].Object = 0;
  }

 // ������.
 return;
}

// ��ᯮ������ �� �����孮�� ��ઠ� ��᪮�쪮 �।��⮢.

VOID ThrowObjects( VOID )
{
 // ����⢨� �믮������ ��� ࠧ� - ��� ������ � �ࠢ��� ��ઠ��.
 for( INT Mirrors_count = 1; Mirrors_count <= 2; Mirrors_count ++ )
  {
   // �롨ࠥ� ��ઠ��.
   MIRROR_SQUARES* Mirror = NULL;
   if( Mirrors_count == 1 ) Mirror = Left_mirror; else Mirror = Right_mirror;

   // ������塞 ���� ������� ���� �����孮��.
   for( INT Count = 0; Count < Mirror_size.Width * Mirror_size.Height / 10; Count ++ )
    {
     // ��ᯮ������ �।����. ���� ��ᥬ� �ய�᪠��, �� ����� � �騪�.
     INT Position = Rnd( Mirror_size.Width * Mirror_size.Height / 1.05 );
     Mirror[ Position ].Object = Rnd( MAX_SPRITE - MAX_BOX - 1 ) + MAX_BOX + 1;
     Mirror[ Position ].Y_Offset = 0;
     Mirror[ Position ].Action = THROWING;
    }
  }

 // ������.
 return;
}

// ��� ����� �� � ���� ���

// Client_window ��।���� ���� �ਫ������.
VOID DrawImageInWindow( HWND Client_window )
{
 // ������ ࠧ���� ���� �ਫ������.
 RECTL Rectangle = { 0, 0, 0, 0 }; WinQueryWindowRect( Client_window, &Rectangle );

 // ����⠥� � ����࠭�⢥ �⮡ࠦ���� ����.
 HPS Presentation_space = WinGetPS( Client_window );

 // �᫨ ���� ���⨭�� - �ᯮ��㥬 �� ��� ���������� ����.
 if( Background.Bitmap != NULLHANDLE )
  {
   // ������塞 ����.
   for( INT X_Count = 0; X_Count <= Rectangle.xRight + Background.Bitmap_width; X_Count += Background.Bitmap_width )
    for( INT Y_Count = 0; Y_Count <= Rectangle.yTop + Background.Bitmap_height; Y_Count += Background.Bitmap_height )
     {
      // ���㥬 ���⨭��.
      POINTL Point = { X_Count, Y_Count };
      WinDrawBitmap( Presentation_space, Background.Bitmap, NULL, &Point, 0, 0, DBM_NORMAL );
     }
  }
 // � �᫨ ���⨭�� ��� - ����訢��� ���� ��� 梥⮬.
 else WinFillRect( Presentation_space, &Rectangle, CLR_BLACK );

 // ����塞 �窨, �� ����� ���� ���ᮢ��� ��ઠ��.
 CalculateMirrorPoints( Rectangle );

 // �஢���� �����.
 GpiSetColor( Presentation_space, CLR_CYAN );
 POINTL A_Point = { Mirror_points.Ax, Mirror_points.Ay }; GpiMove( Presentation_space, &A_Point );
 POINTL G_Point = { Mirror_points.Gx, Mirror_points.Gy }; GpiLine( Presentation_space, &G_Point );
 GpiSetColor( Presentation_space, CLR_PALEGRAY );
 POINTL L_Point = { Mirror_points.Lx, Mirror_points.Ly }; GpiLine( Presentation_space, &L_Point );
 POINTL F_Point = { Mirror_points.Fx, Mirror_points.Fy }; GpiLine( Presentation_space, &F_Point );

 POINTL B_Point = { Mirror_points.Bx, Mirror_points.By }; GpiMove( Presentation_space, &B_Point );
 POINTL H_Point = { Mirror_points.Hx, Mirror_points.Hy }; GpiLine( Presentation_space, &H_Point );
 GpiSetColor( Presentation_space, CLR_CYAN );
 POINTL I_Point = { Mirror_points.Ix, Mirror_points.Iy }; GpiLine( Presentation_space, &I_Point );
 POINTL C_Point = { Mirror_points.Cx, Mirror_points.Cy }; GpiLine( Presentation_space, &C_Point );

 GpiSetColor( Presentation_space, CLR_PALEGRAY );
 POINTL D_Point = { Mirror_points.Dx, Mirror_points.Dy }; GpiMove( Presentation_space, &D_Point );
 POINTL J_Point = { Mirror_points.Jx, Mirror_points.Jy }; GpiLine( Presentation_space, &J_Point );
 GpiSetColor( Presentation_space, CLR_CYAN );
 POINTL K_Point = { Mirror_points.Kx, Mirror_points.Ky }; GpiLine( Presentation_space, &K_Point );
 POINTL E_Point = { Mirror_points.Ex, Mirror_points.Ey }; GpiLine( Presentation_space, &E_Point );

 // ���㥬 �㡨�� � ���. ����⢨� �믮������ ��� ࠧ� - ��� ������ � �ࠢ��� ��ઠ��.
 for( INT Mirrors_count = 1; Mirrors_count <= 2; Mirrors_count ++ )
  {
   // �롨ࠥ� ��ઠ�� � �窨.
   MIRROR_SQUARES* Mirror = NULL;
   INT Y_First_point = 0; INT Y_Last_point = 0;
   INT X_First_point = 0; INT X_Last_point = 0;
   if( Mirrors_count == 1 )
    {
     Mirror = Left_mirror;
     Y_First_point = Mirror_points.Hy; Y_Last_point = Mirror_points.By;
     X_First_point = Mirror_points.Hx; X_Last_point = Mirror_points.Ix;
    }
   else
    {
     Mirror = Right_mirror;
     Y_First_point = Mirror_points.Jy; Y_Last_point = Mirror_points.Dy;
     X_First_point = Mirror_points.Jx; X_Last_point = Mirror_points.Kx;
    }

   // ���㥬 �㡨�� � ���.
   INT Position = 0;
   for( INT Y_Count = Y_First_point + GRID_WIDTH; Y_Count <= Y_Last_point - SPRITE_HEIGHT; Y_Count += SPRITE_HEIGHT + GRID_WIDTH )
    for( INT X_Count = X_First_point + GRID_WIDTH; X_Count <= X_Last_point - GRID_WIDTH - SPRITE_WIDTH; X_Count += SPRITE_WIDTH + GRID_WIDTH )
     {
      // ���㥬 �।���.
      DrawSprite( Presentation_space, X_Count, Y_Count + Mirror[ Position ].Y_Offset, Sprite[ Mirror[ Position ].Object ] );

      // ���㥬 ���.
      if( Settings.Grid_is_visible ) if( X_Count - 1 != Mirror_points.Hx ) if( Y_Count - 1 != Mirror_points.Hy )
       { POINTL Point = { X_Count - 1, Y_Count - 1 }; GpiSetPel( Presentation_space, &Point ); }

      // ���室�� � ᫥���饬� �।����.
      Position ++;
     }
  }

 // �᫨ ���� �������� ���⠢��:
 if( Settings.Game_mode == 0 )
  {
   // ������ ࠬ�� ���� �ਫ������.
   HWND Frame_window = WinQueryWindow( Client_window, QW_PARENT );
   // ������ ��������� ���� �ਫ������.
   HWND Titlebar_window = WinWindowFromID( Frame_window, FID_TITLEBAR );

   // ����⠥� � ����࠭�⢥ �⮡ࠦ���� ��������� ����.
   HPS Titlebar_space = WinGetPS( Titlebar_window );

   // ������, ����� ���� ᥩ�� �ᯮ������ � ��������� ����.
   FONTMETRICS Titlebar_font_metrics;
   GpiQueryFontMetrics( Titlebar_space, sizeof( FONTMETRICS ), &Titlebar_font_metrics );

   // �����蠥� ࠡ��� � ����࠭�⢥ �⮡ࠦ���� ���������.
   WinReleasePS( Titlebar_space );

   // ���� ��� �ᮢ���� �㤥� ⠪�� ��, ��� � � ��������� ����.
   FATTRS Font_attributes;
   Font_attributes.usRecordLength = sizeof( FATTRS );
   Font_attributes.fsSelection = Titlebar_font_metrics.fsSelection;
   Font_attributes.lMatch = Titlebar_font_metrics.lMatch;
   strcpy( Font_attributes.szFacename, Titlebar_font_metrics.szFacename );
   Font_attributes.idRegistry = Titlebar_font_metrics.idRegistry;
   Font_attributes.usCodePage = Titlebar_font_metrics.usCodePage;
   Font_attributes.lMaxBaselineExt = Titlebar_font_metrics.lMaxBaselineExt;
   Font_attributes.lAveCharWidth = Titlebar_font_metrics.lAveCharWidth;
   Font_attributes.fsType = Titlebar_font_metrics.fsType;
   Font_attributes.fsFontUse = 0;

   // ������� ����.
   GpiCreateLogFont( Presentation_space, NULL, 1, &Font_attributes );
   // �롨ࠥ� ���� � ����࠭�⢥ �⮡ࠦ���� ����.
   GpiSetCharSet( Presentation_space, 1 );

   // ������ ����� ��ப� ��� ����, ����� �ᯮ������ � ����.
   RECTL Letter_rectangle = { 0, 0, 1, 1 };
   WinDrawText( Presentation_space, -1, "Mirrors game. (C) Sergey Posokhov, Moscow. E-mail: abc@posokhov.msk.ru.", &Letter_rectangle, 0, 0, DT_QUERYEXTENT );
   INT Letter_height = Letter_rectangle.yTop - Letter_rectangle.yBottom;

   // ������ �ᯮ������� ��ப ⥪��.
   Rectangle.xLeft += 25; Rectangle.xRight -= 25;
   Rectangle.yTop -= Rectangle.yTop / 5; Rectangle.yBottom = Rectangle.yTop - Letter_height;

   // �������� ����.
   WinDrawText( Presentation_space, -1, Strings_in_window.Title, &Rectangle, 0, 0, DT_TEXTATTRS | DT_CENTER | DT_VCENTER );

   // �ய�᪠�� ��᪮�쪮 ��ப.
   Rectangle.yTop -= Letter_height * 3; Rectangle.yBottom -= Letter_height * 3;

   // ���ᠭ�� � �ࠢ��� ����.
   GpiSetColor( Presentation_space, CLR_WHITE );
   for( INT Count = 0; Count <= 5; Count ++ )
    {
     WinDrawText( Presentation_space, -1, Strings_in_window.Description[ Count ], &Rectangle, 0, 0, DT_TEXTATTRS | DT_CENTER | DT_VCENTER | DT_WORDBREAK );
     Rectangle.yTop -= Letter_height * 1.25; Rectangle.yBottom -= Letter_height * 1.25;
    }

   // �ய�᪠�� ��᪮�쪮 ��ப.
   Rectangle.yTop -= Letter_height * 2; Rectangle.yBottom -= Letter_height * 2;

   // ���ࠡ��稪.
   GpiSetColor( Presentation_space, CLR_CYAN );
   WinDrawText( Presentation_space, -1, Strings_in_window.Developer, &Rectangle, 0, 0, DT_TEXTATTRS | DT_CENTER | DT_VCENTER | DT_WORDBREAK );
  }

 // �����蠥� ࠡ��� � ����࠭�⢥ �⮡ࠦ���� ����.
 WinReleasePS( Presentation_space );

 // ������.
 return;
}

// ��� ������ ࠧ���� ��ઠ� ���

// Window_rectangle ��।���� ࠧ���� ����.
VOID CalculateMirrorSize( RECTL Window_rectangle )
{
 // ��⠭�������� ࠧ���� ��ઠ�.
 Mirror_size.Width = ( Window_rectangle.xRight - BORDER_WIDTH * 3 - Window_rectangle.xRight / 2 ) / ( SPRITE_WIDTH + GRID_WIDTH ) / 2;
 Mirror_size.Height = ( Window_rectangle.yTop - BORDER_WIDTH - Window_rectangle.yTop / 4 ) / ( SPRITE_HEIGHT + GRID_WIDTH );

 // ������ ��ઠ�� ������ ���� >= 3x2.
 if( Mirror_size.Width <= 3 ) Mirror_size.Width = 3;
 if( Mirror_size.Height <= 2 ) Mirror_size.Height = 2;

 // ������.
 return;
}

// ��� ������ �窨, �� ����� ���� ���ᮢ��� ��ઠ�� ���

// Window_rectangle ��।���� ࠧ���� ����.
VOID CalculateMirrorPoints( RECTL Window_rectangle )
{
 // ����塞 ࠧ���� ��ઠ��. � �ਭ� �ਡ������� ���祭�� GRID_WIDTH, ��⮬� ��
 // � ��� �ࠢ� ���� �������� �窨, ᮮ⢥�����騥 �ਭ� �⪨.
 INT Mirror_width = Mirror_size.Width * ( SPRITE_WIDTH + GRID_WIDTH ) + GRID_WIDTH;
 INT Mirror_height = Mirror_size.Height * ( SPRITE_HEIGHT + GRID_WIDTH );

 // ��⠭�������� �窨.
 Mirror_points.Ax = ( Window_rectangle.xRight - Mirror_width * 2 - BORDER_WIDTH * 3 ) / 2;
 Mirror_points.Ay = Window_rectangle.yTop - ( Window_rectangle.yTop - Mirror_height - BORDER_WIDTH ) / 2;
 Mirror_points.Bx = Mirror_points.Ax + BORDER_WIDTH;
 Mirror_points.By = Mirror_points.Ay;
 Mirror_points.Cx = Mirror_points.Bx + Mirror_width;
 Mirror_points.Cy = Mirror_points.Ay;
 Mirror_points.Fx = Window_rectangle.xRight - Mirror_points.Ax;
 Mirror_points.Fy = Mirror_points.Ay;
 Mirror_points.Ex = Mirror_points.Fx - BORDER_WIDTH;
 Mirror_points.Ey = Mirror_points.Ay;
 Mirror_points.Dx = Mirror_points.Ex - Mirror_width;
 Mirror_points.Dy = Mirror_points.Ay;
 Mirror_points.Gx = Mirror_points.Ax;
 Mirror_points.Gy = Window_rectangle.yTop - Mirror_points.Ay;
 Mirror_points.Hx = Mirror_points.Bx;
 Mirror_points.Hy = Mirror_points.Gy + BORDER_WIDTH;
 Mirror_points.Ix = Mirror_points.Cx;
 Mirror_points.Iy = Mirror_points.Hy;
 Mirror_points.Jx = Mirror_points.Dx;
 Mirror_points.Jy = Mirror_points.Hy;
 Mirror_points.Kx = Mirror_points.Ex;
 Mirror_points.Ky = Mirror_points.Hy;
 Mirror_points.Lx = Mirror_points.Fx;
 Mirror_points.Ly = Mirror_points.Gy;

 // ������.
 return;
}

// ��� ��ࠡ��뢠�� ᮮ�饭�� �� ���稪� �६���, ��।������ �।���� ���

// Client_window ��।���� ���� �ਫ������, Timer - ���稪 �६���.
VOID AnimateObjects( HWND Client_window )
{
 // ����뢠��, �� ����騥 �।���� ������ ��⠭�������.
 INT Flying_is_done = 0;
 // ����뢠��, �� �騪� ���� �ਢ��� � ��������.
 INT Objects_must_be_shaked = 0;
 // ����뢠��, �� �⮫��� �騪�� ���� ��⠭�����.
 INT Column_must_be_stopped = 0; INT Stopping_position = 0;

 // ����⠥� � ����࠭�⢥ �⮡ࠦ���� ����.
 HPS Presentation_space = WinGetPS( Client_window );

 // ����⢨� �믮������ ��� ࠧ� - ��� ������ � �ࠢ��� ��ઠ��.
 for( INT Mirrors_count = 1; Mirrors_count <= 2; Mirrors_count ++ )
  {
   // �롨ࠥ� ��ઠ�� � �窨.
   MIRROR_SQUARES* Mirror = NULL;
   INT Y_First_point = 0; INT Y_Last_point = 0;
   INT X_First_point = 0; INT X_Last_point = 0;
   if( Mirrors_count == 1 )
    {
     Mirror = Left_mirror;
     Y_First_point = Mirror_points.Hy; Y_Last_point = Mirror_points.By;
     X_First_point = Mirror_points.Hx; X_Last_point = Mirror_points.Ix;
    }
   else
    {
     Mirror = Right_mirror;
     Y_First_point = Mirror_points.Jy; Y_Last_point = Mirror_points.Dy;
     X_First_point = Mirror_points.Jx; X_Last_point = Mirror_points.Kx;
    }

   // ��।������ �।���� � ��࠭��� ��ઠ��.
   INT Position = 0;
   for( INT Y_Count = Y_First_point + GRID_WIDTH; Y_Count <= Y_Last_point - SPRITE_HEIGHT; Y_Count += SPRITE_HEIGHT + GRID_WIDTH )
    for( INT X_Count = X_First_point + GRID_WIDTH; X_Count <= X_Last_point - GRID_WIDTH - SPRITE_WIDTH; X_Count += SPRITE_WIDTH + GRID_WIDTH )
     {
      // �᫨ �������� �� �뫮 �����襭� - ��।������ �।����.
      if( !Flying_is_done )
       {
        // �᫨ ��� �।��� ������ ����⢨� "��������" - ��।������ �।���.
        if( Mirror[ Position ].Object != 0 )
         if( Mirror[ Position ].Action == FLYING || Mirror[ Position ].Action == DARTING || Mirror[ Position ].Action == THROWING )
          {
           // ���������� ���, � ���ன �ᯮ����� �।���.
           INT Previous_Y = Y_Count + Mirror[ Position ].Y_Offset;

           // ����塞 ����� �ᯮ������� �।���.
           INT Next_position = Position - Mirror_size.Width;

           // �᫨ ����� �ᯮ����� �騪, ����� ������ ��祧���� - �� ��।������ �।���.
           if( Next_position >= 0 )
            if( Mirror[ Next_position ].Object != 0 )
             if( Mirror[ Next_position ].Action == DISAPPEARING ) continue;

           // �᫨ ����� ���� ��-�, �� �������� ��������� �।��� - �� �������� ��������.
           if( Next_position >= 0 ) if( Mirror[ Next_position ].Object != 0 )
            {
             if( Mirror[ Position ].Action == THROWING )
              {
               if( Mirror[ Next_position ].Action == FLYING || Mirror[ Next_position ].Action == DARTING )
                Mirror[ Position ].Action = FLYING;
              }
             else if( Mirror[ Position ].Action == FLYING || Mirror[ Position ].Action == DARTING )
              {
               if( Mirror[ Next_position ].Action == THROWING )
                Mirror[ Position ].Action = THROWING;
              }
            }

           // ������ ����� �ᯮ������� �।���.
           INT Step = 0;
           if( Mirror[ Position ].Action == FLYING ) Step = Rate.Flying_step;
           if( Mirror[ Position ].Action == DARTING ) Step = Rate.Darting_step;
           if( Mirror[ Position ].Action == THROWING ) Step = Rate.Throwing_step;
           Mirror[ Position ].Y_Offset -= Step;

           // �᫨ �।��� ���⨣ ��� �⠪��� - �� ��⠭����������.
           if( Next_position < 0 ) if( Mirror[ Position ].Y_Offset <= 0 )
            {
             // �᫨ ���� ��� - �������� ������ ���� �����襭�.
             if( Settings.Game_mode == 2 )
              if( Mirror[ Position ].Action == FLYING || Mirror[ Position ].Action == DARTING ) Flying_is_done = 1;

             // �᫨ �� �������� �騪, �������� ��� - ��⠭�������� ��� �ࠧ�.
             if( Mirror[ Position ].Object <= MAX_BOX )
              {
               // ��⠭�������� �騪.
               Mirror[ Position ].Y_Offset = 0;
               Mirror[ Position ].Action = 0;
               // ��㣨� �।���� ⮦� ������ ���� ��⠭������.
               Column_must_be_stopped = 1; Stopping_position = Position;
              }
             // ���� - ��⠭�������� �।���, � �� ���⥯���� �ॢ�頥��� � �騪.
             else
              {
               // ��⠭�������� �।���.
               Mirror[ Position ].Y_Offset = 0;
               Mirror[ Position ].Action = PACKING;
               Mirror[ Position ].Action_count = 0;
               Mirror[ Position ].Change_point = Rnd( STOP_POINT - STOP_POINT / 2 ) + STOP_POINT / 2;
              }
            }

           // �᫨ �।��� �஫�⥫ ������, � � ᫥���饬 ������ ��-� ����:
           if( Next_position >= 0 )
            if( Mirror[ Position ].Y_Offset <= 0 ) if( Mirror[ Next_position ].Object != 0 )
             {
              // �᫨ ����� ���� ��-� ����������� - �।��� ��⠭����������.
              if( Mirror[ Next_position ].Action == 0 || Mirror[ Next_position ].Action == PACKING )
               {
                // �᫨ ���� ��� - �������� ������ ���� �����襭�.
                if( Settings.Game_mode == 2 )
                 if( Mirror[ Position ].Action == FLYING || Mirror[ Position ].Action == DARTING ) Flying_is_done = 1;

                // �᫨ �� �������� �騪, �������� ��� - ��⠭�������� ��� �ࠧ�.
                if( Mirror[ Position ].Object <= MAX_BOX )
                 {
                  // ��⠭�������� �騪.
                  Mirror[ Position ].Y_Offset = 0;
                  Mirror[ Position ].Action = 0;
                  // ��㣨� �।���� � �⮬ �⮫���� ⮦� ������ ���� ��⠭������.
                  Column_must_be_stopped = 1; Stopping_position = Position;
                 }
                // ���� - ��⠭�������� �।���, � �� ���⥯���� �ॢ�頥��� � �騪.
                else
                 {
                  // ��⠭�������� �।���.
                  Mirror[ Position ].Y_Offset = 0;
                  Mirror[ Position ].Action = PACKING;
                  Mirror[ Position ].Action_count = 0;
                  Mirror[ Position ].Change_point = Rnd( STOP_POINT - STOP_POINT / 2 ) + STOP_POINT / 2;
                 }
               }
             }

           // ���뢠�� �।��� � �窥, � ���ன �� �� �ᯮ�����.
           HideSprite( Presentation_space, X_Count, Previous_Y );
           // ���㥬 �।��� � ����� �窥.
           DrawSprite( Presentation_space, X_Count, Y_Count + Mirror[ Position ].Y_Offset, Sprite[ Mirror[ Position ].Object ] );

           // �᫨ �।��� �஫�⥫ �������� ������ - ��७�ᨬ ��� � ��㣮� ������.
           if( Next_position >= 0 )
            if( Mirror[ Position ].Y_Offset <= ( -1 ) * SPRITE_HEIGHT / 2 )
             {
              // ��७�ᨬ �㤠 �।���.
              Mirror[ Next_position ].Object = Mirror[ Position ].Object;
              Mirror[ Next_position ].Y_Offset = Mirror[ Position ].Y_Offset + SPRITE_HEIGHT + 1;
              Mirror[ Next_position ].Action = Mirror[ Position ].Action;

              // ��࠭�� ������ �⠭������ �����.
              Mirror[ Position ].Object = 0; Mirror[ Position ].Action = 0;
             }
          }
       }

      // �᫨ �।��� �� ��⠭����� - �� �ॢ�頥��� � �騪.
      if( Mirror[ Position ].Object != 0 ) if( Mirror[ Position ].Action == PACKING )
       {
        // ������ �ன� �����஥ �६�, ��� �⮣� ���� ���稪.
        Mirror[ Position ].Action_count ++;
        // ���砫� �।��� �ॢ�頥��� � �騪.
        if( Mirror[ Position ].Action_count == Mirror[ Position ].Change_point )
         {
          // �।��� �ॢ�頥��� � �騪.
          Mirror[ Position ].Object = Rnd( MAX_BOX - 1 ) + 1;
          // ���뢠�� �।���.
          HideSprite( Presentation_space, X_Count, Y_Count );
          // ���㥬 �।��� ������.
          DrawSprite( Presentation_space, X_Count, Y_Count, Sprite[ Mirror[ Position ].Object ] );
         }
        // ��᫥ �⮣� �騪 �⠭������ ����������.
        if( Mirror[ Position ].Action_count == STOP_POINT )
         Mirror[ Position ].Action = 0;
       }

      // �᫨ �।��� ������ ��祧����:
      if( Mirror[ Position ].Object != 0 ) if( Mirror[ Position ].Action == DISAPPEARING )
       {
        // ������ �ன� �����஥ �६�, ��� �⮣� ���� ���稪.
        Mirror[ Position ].Action_count ++;
        // ��᫥ �⮣� �।��� ��祧���.
        if( Mirror[ Position ].Action_count == Mirror[ Position ].Change_point )
         {
          // �।��� ��祧���.
          Mirror[ Position ].Object = 0;
          Mirror[ Position ].Action = 0;
          // ���뢠�� �।���.
          HideSprite( Presentation_space, X_Count, Y_Count );
          // �᫨ ���� ��� - �騪� ���� �ਢ��� � ��������.
          if( Settings.Game_mode == 2 ) Objects_must_be_shaked = 1;
         }
       }

      // �᫨ ���� ��⠭����� �⮫��� �騪�� - ������ ��.
      if( Column_must_be_stopped )
       {
        // ��⠭�������� �⮫��� �騪��.
        for( INT Count = Stopping_position; Count < Mirror_size.Width * Mirror_size.Height; Count += Mirror_size.Width )
         if( Mirror[ Count ].Object != 0 ) if( Mirror[ Count ].Object <= MAX_BOX )
          if( Mirror[ Count ].Action == THROWING )
           {
            // ���뢠�� �।��� � �窥, � ���ன �� �� �ᯮ�����.
            INT V_Count = Count / Mirror_size.Width;
            INT H_Count = Count - V_Count * Mirror_size.Width;
            INT X = X_First_point + GRID_WIDTH + ( SPRITE_WIDTH + GRID_WIDTH ) * H_Count;
            INT Y = Y_First_point + GRID_WIDTH + ( SPRITE_HEIGHT + GRID_WIDTH ) * V_Count + Mirror[ Count ].Y_Offset;
            HideSprite( Presentation_space, X, Y );

            // ��⠭�������� �।���.
            Mirror[ Count ].Action = 0; Mirror[ Count ].Y_Offset = 0;

            // ���㥬 �।��� ������.
            Y = Y_First_point + GRID_WIDTH + ( SPRITE_HEIGHT + GRID_WIDTH ) * V_Count;
            DrawSprite( Presentation_space, X, Y, Sprite[ Mirror[ Count ].Object ] );
           }

        // �⮫��� �騪�� ��⠭�����.
        Column_must_be_stopped = 0;
       }

      // ���室�� � ᫥���饬� �।����.
      Position ++;
     }

    // �᫨ �������� �뫮 �����襭� - ��⠭�������� �� ����騥 �।����.
    if( Flying_is_done )
     {
      // ��⠭�������� ����騥 �।���� � ��࠭��� ��ઠ��.
      INT Position = 0;
      for( INT Y_Count = Y_First_point + GRID_WIDTH; Y_Count <= Y_Last_point - SPRITE_HEIGHT; Y_Count += SPRITE_HEIGHT + GRID_WIDTH )
       for( INT X_Count = X_First_point + GRID_WIDTH; X_Count <= X_Last_point - GRID_WIDTH - SPRITE_WIDTH; X_Count += SPRITE_WIDTH + GRID_WIDTH )
        {
         // �᫨ ��� �।��� ������ ����⢨� "�����" - ��⠭�������� ���.
         if( Mirror[ Position ].Object != 0 )
          if( Mirror[ Position ].Action == FLYING || Mirror[ Position ].Action == DARTING )
           {
            // ���뢠�� �।��� � �窥, � ���ன �� �� �ᯮ�����.
            HideSprite( Presentation_space, X_Count, Y_Count + Mirror[ Position ].Y_Offset );

            // ��⠭�������� �।���.
            Mirror[ Position ].Y_Offset = 0;
            Mirror[ Position ].Action = PACKING;
            Mirror[ Position ].Action_count = 0;
            Mirror[ Position ].Change_point = Rnd( STOP_POINT - STOP_POINT / 2 ) + STOP_POINT / 2;

            // ���㥬 �।��� ������.
            DrawSprite( Presentation_space, X_Count, Y_Count, Sprite[ Mirror[ Position ].Object ] );
           }

         // ���室�� � ᫥���饬� �।����.
         Position ++;
        }
     }
  }

 // �����蠥� ࠡ��� � ����࠭�⢥ �⮡ࠦ���� ����.
 WinReleasePS( Presentation_space );

 // �᫨ �騪� ���� �ਢ��� � �������� - ������ ��.
 if( Objects_must_be_shaked )
  for( INT Count = 0; Count < Mirror_size.Width * Mirror_size.Height; Count ++ )
   {
    if( Left_mirror[ Count ].Object != 0 ) if( Left_mirror[ Count ].Action == 0 )
     Left_mirror[ Count ].Action = THROWING;
    if( Right_mirror[ Count ].Object != 0 ) if( Right_mirror[ Count ].Action == 0 )
     Right_mirror[ Count ].Action = THROWING;
   }

 // �᫨ ���� "�����訥" �騪� - ��� ������ �த������ ������.
 DetectHangedBoxes();

 // �஢��塞, ���� �� �� ����騥 ��� �������騥�� �।����.
 INT Objects_is_stopped = 1; INT Objects_is_throwed = 1;
 INT Objects_is_packed = 1; INT Objects_is_disappeared = 1;
 for( INT Count = 0; Count < Mirror_size.Width * Mirror_size.Height; Count ++ )
  {
   if( Left_mirror[ Count ].Object != 0 )
    {
     if( Left_mirror[ Count ].Action == FLYING ) Objects_is_stopped = 0;
     if( Left_mirror[ Count ].Action == DARTING ) Objects_is_stopped = 0;
     if( Left_mirror[ Count ].Action == THROWING ) Objects_is_throwed = 0;
     if( Left_mirror[ Count ].Action == PACKING ) Objects_is_packed = 0;
     if( Left_mirror[ Count ].Action == DISAPPEARING ) Objects_is_disappeared = 0;
    }
   if( Right_mirror[ Count ].Object != 0 )
    {
     if( Right_mirror[ Count ].Action == FLYING ) Objects_is_stopped = 0;
     if( Right_mirror[ Count ].Action == DARTING ) Objects_is_stopped = 0;
     if( Right_mirror[ Count ].Action == THROWING ) Objects_is_throwed = 0;
     if( Right_mirror[ Count ].Action == PACKING ) Objects_is_packed = 0;
     if( Right_mirror[ Count ].Action == DISAPPEARING ) Objects_is_disappeared = 0;
    }
  }

 // �᫨ ��� ⮫쪮 ��稭����� � ������� �।��⮢ ��� - 㤠�塞 ���, � �᫨ ��� ��
 // �뫨 㤠����, �ਢ���� �� �騪� � �������� �� ࠧ � ���室�� � ����.
 if( Settings.Game_mode == 1 )
  if( Objects_is_throwed ) if( Objects_is_packed ) if( Objects_is_disappeared )
   {
    // ����塞 ���.
    INT Rows_was_deleted = DeleteRows( Client_window );

    // �᫨ ��� �� �뫨 㤠���� - ���室�� � ����.
    if( !Rows_was_deleted ) Settings.Game_mode = 2;
   }

 // �᫨ ���� ��� - �����㥬 �� ����室�����.
 if( Settings.Game_mode == 2 )
  {
   // �᫨ �� ����⢨� �����襭� - 㤠�塞 ��� � ��ᠥ� ���� �।����.
   if( Objects_is_stopped ) if( Objects_is_throwed ) if( Objects_is_packed ) if( Objects_is_disappeared )
    {
     // ����塞 ���.
     DeleteRows( Client_window );
     // ��ᠥ� ���� �।���� � 㧭���, ����� �� �த������ ����.
     INT Game_can_be_continued = ThrowObjectSet( Client_window );

     // �᫨ ��� ������ ���� �����襭� - �����蠥� ��.
     if( !Game_can_be_continued )
      {
       // �� �।���� ������ ��祧����.
       DeleteAllObjects();
       // ��� �����蠥���.
       Settings.Game_mode = 3;

       // ��� - ��� �����蠥���.
       WinPostMsg( Client_window, WM_PLAYSOUND, (MPARAM) SOUND_OVER, 0 );

       // ������.
       return;
      }
    }

   // �᫨ ���� ⮫쪮 ����騥 �।���� - ��������, ��� ���� ᭮�� 㤠����.
   if( Objects_is_throwed ) if( Objects_is_packed ) if( Objects_is_disappeared )
    DeleteRows( Client_window );
  }

 // �᫨ ��� �����蠥��� � �� �।���� ��祧�� - ���室�� � ���⠢��.
 if( Settings.Game_mode == 3 ) if( Objects_is_disappeared )
  {
   // ���室�� � ���⠢��.
   Settings.Game_mode = 0;

   // ��⠭�������� ���稪 �६���.
   WinStopTimer( Application, Client_window, 1 );

   // ������ ��ઠ�� ����묨 � �ᯮ������ �� �����孮�� ��᪮�쪮 �।��⮢.
   CleanMirrors(); ThrowObjects();

   // ���� ������ ���� ����ᮢ���. ��।��� ᮮ�饭�� � ��⮪.
   Thread_responds.Draw_message_is_received = 0;
   WinPostQueueMsg( Thread_message_queue, WM_DRAW, (MPARAM) Client_window, 0 );
  }

 // ������.
 return;
}

// ��� ������ ��� ���

// Client_window ��।���� ���� �ਫ������.
// �����頥� 1 ��� 0 � ����ᨬ��� �� ⮣�, �뫨 㤠���� ���, ��� ���.
INT DeleteRows( HWND Client_window )
{
 // ����뢠��, �� ��� �뫨 㤠����.
 INT Rows_was_deleted = 0;

 // ��ᬠ�ਢ��� ��ઠ��.
 for( INT V_Count = 0; V_Count < Mirror_size.Height; V_Count ++ )
  {
   // ��� ������ ���� �� ����묨 � ��ઠ��묨.
   INT Rows_is_empty = 1; INT Rows_is_alike = 0;

   // �஢��塞 ��� �� �������������.
   for( INT H_Count = 0; H_Count < Mirror_size.Width; H_Count ++ )
    if( Left_mirror[ V_Count * Mirror_size.Width + H_Count ].Object != 0 )
     if( Left_mirror[ V_Count * Mirror_size.Width + H_Count ].Object <= MAX_BOX )
      Rows_is_empty = 0;

   // �஢��塞 ��� �� ��ઠ�쭮���.
   if( !Rows_is_empty )
    {
     // ��ᬠ�ਢ��� ��ઠ�� �� ��� � �।���.
     Rows_is_alike = 1;
     for( H_Count = 0; H_Count < Mirror_size.Width; H_Count ++ )
      {
       // � ��ઠ�� ᫥�� ��-� ����:
       INT Left_object = 0;
        if( Left_mirror[ V_Count * Mirror_size.Width + H_Count ].Object != 0 )
         if( Left_mirror[ V_Count * Mirror_size.Width + H_Count ].Object <= MAX_BOX )
          if( Left_mirror[ V_Count * Mirror_size.Width + H_Count ].Action == 0 )
           Left_object = 1;
       // � ��ઠ�� �ࠢ� ��-� ����:
       INT Right_object = 0;
        if( Right_mirror[ ( V_Count * Mirror_size.Width + Mirror_size.Width - 1 ) - H_Count ].Object != 0 )
         if( Right_mirror[ ( V_Count * Mirror_size.Width + Mirror_size.Width - 1 ) - H_Count ].Object <= MAX_BOX )
          if( Right_mirror[ ( V_Count * Mirror_size.Width + Mirror_size.Width - 1 ) - H_Count ].Action == 0 )
           Right_object = 1;
       // �᫨ ᮢ������� ��� - ���������� ��.
       if( Left_object != Right_object ) Rows_is_alike = 0;
      }
    }

   // �᫨ ��� ��ઠ���:
   if( Rows_is_alike )
    for( H_Count = 0; H_Count < Mirror_size.Width; H_Count ++ )
     {
      // �騪� ��祧���.
      INT Position = V_Count * Mirror_size.Width + H_Count;
      Left_mirror[ Position ].Action = DISAPPEARING;
      Left_mirror[ Position ].Action_count = 0;
      Left_mirror[ Position ].Change_point = Rnd( STOP_POINT ) + STOP_POINT / 2;

      Right_mirror[ Position ].Action = DISAPPEARING;
      Right_mirror[ Position ].Action_count = 0;
      Right_mirror[ Position ].Change_point = Rnd( STOP_POINT ) + STOP_POINT / 2;

      // ��� �뫨 㤠����.
      Rows_was_deleted = 1;
     }
  }

 // ��� - ��� �뫨 㤠����.
 if( Rows_was_deleted ) WinPostMsg( Client_window, WM_PLAYSOUND, (MPARAM) SOUND_DELETE, 0 );

 // ������.
 return Rows_was_deleted;
}

// ��� �஢����, ���� �� � ��ઠ��� "�����訥" �騪� � ��ᠥ� �� ���

VOID DetectHangedBoxes( VOID )
{
 // �஢��塞, ���� �� � ��ઠ�� "�����訥" �騪�.
 for( INT Mirrors_count = 1; Mirrors_count <= 2; Mirrors_count ++ )
  {
   // �롨ࠥ� ��ઠ��.
   MIRROR_SQUARES* Mirror = NULL;
   if( Mirrors_count == 1 ) Mirror = Left_mirror; else Mirror = Right_mirror;

   for( INT V_Count = 0; V_Count < Mirror_size.Height; V_Count ++ )
    for( INT H_Count = 0; H_Count < Mirror_size.Width; H_Count ++ )
     {
      // �஢��塞 �।���.
      INT Position = V_Count * Mirror_size.Width + H_Count;

      // ����ਬ, ���� �� ��-����� �����.
      if( Position >= Mirror_size.Width )
       if( Mirror[ Position ].Object != 0 )
        if( Mirror[ Position ].Action == 0 || Mirror[ Position ].Action == PACKING )
         {
          // ������ �� �஢�ન.
          INT Results[ 3 ] = { 0, 0, 0 };

          // ����ਬ, ���� �� ��-����� �����.
          if( Mirror[ Position - Mirror_size.Width ].Object == 0 ) Results[ 0 ] = 1;

          // ����ਬ, ���� �� ��-����� ����� ᫥��. �।��� ����� ���� �ᯮ����� ����� �࠭��� ��ઠ��.
          if( H_Count == 0 ) Results[ 1 ] = 1;
          else if( Mirror[ Position - Mirror_size.Width - 1 ].Object == 0 ) Results[ 1 ] = 1;

          // ����ਬ, ���� �� ��-����� ����� �ࠢ�. �।��� ����� ���� �ᯮ����� ����� �࠭��� ��ઠ��.
          if( H_Count == Mirror_size.Width - 1 ) Results[ 2 ] = 1;
          else if( Mirror[ Position - Mirror_size.Width + 1 ].Object == 0 ) Results[ 2 ] = 1;

          // �᫨ �� ��� ���� �⢥� "��� �।��⮬ ��祣� ���", � �� ������.
          if( Results[ 0 ] + Results[ 1 ] + Results[ 2 ] == 3 )
           if( Mirror[ Position ].Action == PACKING ) Mirror[ Position ].Action = FLYING;
            else Mirror[ Position ].Action = THROWING;
         }
     }
  }

 // ������.
 return;
}

// ��� ��ᯮ������ �� �����孮�� ��ઠ� ����� �।��⮢ ���

// Client_window ��।���� ���� �ਫ������. �����頥� 1 ��� 0.
INT ThrowObjectSet( HWND Client_window )
{
 // �롨ࠥ� ��ઠ��.
 MIRROR_SQUARES* Mirror = NULL;
 if( Rnd( 1 ) == 1 ) Mirror = Left_mirror; else Mirror = Right_mirror;

 // �롨ࠥ� ���� �।���. ���� ��ᥬ� �ய�᪠�� - �� ����� � �騪�.
 INT Position = 0; INT Object = Rnd( MAX_SPRITE - MAX_BOX - 1 ) + MAX_BOX + 1;
 // ����� ����� ࠧ�� ������ �।��⮢.
 INT Object_set = 0;
 if( Settings.Game_is_difficult ) Object_set = Rnd( 15 ); else Object_set = Rnd( 10 );
 switch( Object_set )
  {
   case 0:
    // �롨ࠥ� ���.
    Position = Mirror_size.Height * Mirror_size.Width - Rnd( Mirror_size.Width - 1 ) - 1;
    // �᫨ � �⮩ �窥 ��-� ���� - ������.
    if( Mirror[ Position ].Object != 0 ) return 0;
    // ��ᯮ������ �� �����孮�� ��ઠ�� ����� �।��⮢.
    Mirror[ Position ].Object = Object;                              //
    Mirror[ Position ].Y_Offset = 0;                                 //   X
    Mirror[ Position ].Action = FLYING;                              //
   break;

   case 1:
    // �롨ࠥ� ���.
    Position = Mirror_size.Height * Mirror_size.Width - Rnd( Mirror_size.Width - 2 ) - 2;
    // �᫨ � �窠�, ����� ���� ���������, ��-� ���� - ������.
    if( Mirror[ Position ].Object != 0 ) return 0;
    if( Mirror[ Position + 1 ].Object != 0 ) return 0;
    // ��ᯮ������ �� �����孮�� ��ઠ�� ����� �।��⮢.
    Mirror[ Position ].Object = Object;
    Mirror[ Position ].Y_Offset = 0;                                 //
    Mirror[ Position ].Action = FLYING;                              //   X X
    Mirror[ Position + 1 ].Object = Object;                          //
    Mirror[ Position + 1 ].Y_Offset = 0;
    Mirror[ Position + 1 ].Action = FLYING;
   break;

   case 2:
    // �롨ࠥ� ���.
    Position = Mirror_size.Height * Mirror_size.Width - Rnd( Mirror_size.Width - 1 ) - 1;
    // �᫨ � �窠�, ����� ���� ���������, ��-� ���� - ������.
    if( Mirror[ Position ].Object != 0 ) return 0;
    if( Mirror[ Position - Mirror_size.Width ].Object != 0 ) return 0;
    // ��ᯮ������ �� �����孮�� ��ઠ�� ����� �।��⮢.
    Mirror[ Position ].Object = Object;                              //
    Mirror[ Position ].Y_Offset = 0;                                 //   X
    Mirror[ Position ].Action = FLYING;                              //   X
    Mirror[ Position - Mirror_size.Width ].Object = Object;          //
    Mirror[ Position - Mirror_size.Width ].Y_Offset = 0;
    Mirror[ Position - Mirror_size.Width ].Action = FLYING;
   break;

   case 3:
    // �롨ࠥ� ���.
    Position = Mirror_size.Height * Mirror_size.Width - Rnd( Mirror_size.Width - 2 ) - 2;
    // �᫨ � �窠�, ����� ���� ���������, ��-� ���� - ������.
    if( Mirror[ Position ].Object != 0 ) return 0;
    if( Mirror[ Position - Mirror_size.Width + 1 ].Object != 0 ) return 0;
    // ��ᯮ������ �� �����孮�� ��ઠ�� ����� �।��⮢.
    Mirror[ Position ].Object = Object;                              //
    Mirror[ Position ].Y_Offset = 0;                                 //   X
    Mirror[ Position ].Action = FLYING;                              //     X
    Mirror[ Position - Mirror_size.Width + 1 ].Object = Object;      //
    Mirror[ Position - Mirror_size.Width + 1 ].Y_Offset = 0;
    Mirror[ Position - Mirror_size.Width + 1 ].Action = FLYING;
   break;

   case 4:
    // �롨ࠥ� ���.
    Position = Mirror_size.Height * Mirror_size.Width - Rnd( Mirror_size.Width - 2 ) - 2;
    // �᫨ � �窠�, ����� ���� ���������, ��-� ���� - ������.
    if( Mirror[ Position + 1 ].Object != 0 ) return 0;
    if( Mirror[ Position - Mirror_size.Width ].Object != 0 ) return 0;
    // ��ᯮ������ �� �����孮�� ��ઠ�� ����� �।��⮢.
    Mirror[ Position + 1 ].Object = Object;                          //
    Mirror[ Position + 1 ].Y_Offset = 0;                             //     X
    Mirror[ Position + 1 ].Action = FLYING;                          //   X
    Mirror[ Position - Mirror_size.Width ].Object = Object;          //
    Mirror[ Position - Mirror_size.Width ].Y_Offset = 0;
    Mirror[ Position - Mirror_size.Width ].Action = FLYING;
   break;

   case 5:
    // �롨ࠥ� ���.
    Position = Mirror_size.Height * Mirror_size.Width - Rnd( Mirror_size.Width - 2 ) - 2;
    // �᫨ � �窠�, ����� ���� ���������, ��-� ���� - ������.
    if( Mirror[ Position ].Object != 0 ) return 0;
    if( Mirror[ Position - Mirror_size.Width ].Object != 0 ) return 0;
    if( Mirror[ Position - Mirror_size.Width + 1 ].Object != 0 ) return 0;
    // ��ᯮ������ �� �����孮�� ��ઠ�� ����� �।��⮢.
    Mirror[ Position ].Object = Object;
    Mirror[ Position ].Y_Offset = 0;
    Mirror[ Position ].Action = FLYING;                              //
    Mirror[ Position - Mirror_size.Width ].Object = Object;          //   X
    Mirror[ Position - Mirror_size.Width ].Y_Offset = 0;             //   X X
    Mirror[ Position - Mirror_size.Width ].Action = FLYING;          //
    Mirror[ Position - Mirror_size.Width + 1 ].Object = Object;
    Mirror[ Position - Mirror_size.Width + 1 ].Y_Offset = 0;
    Mirror[ Position - Mirror_size.Width + 1 ].Action = FLYING;
   break;

   case 6:
    // �롨ࠥ� ���.
    Position = Mirror_size.Height * Mirror_size.Width - Rnd( Mirror_size.Width - 2 ) - 2;
    // �᫨ � �窠�, ����� ���� ���������, ��-� ���� - ������.
    if( Mirror[ Position ].Object != 0 ) return 0;
    if( Mirror[ Position + 1 ].Object != 0 ) return 0;
    if( Mirror[ Position - Mirror_size.Width + 1 ].Object != 0 ) return 0;
    // ��ᯮ������ �� �����孮�� ��ઠ�� ����� �।��⮢.
    Mirror[ Position ].Object = Object;
    Mirror[ Position ].Y_Offset = 0;
    Mirror[ Position ].Action = FLYING;                              //
    Mirror[ Position + 1 ].Object = Object;                          //   X X
    Mirror[ Position + 1 ].Y_Offset = 0;                             //     X
    Mirror[ Position + 1 ].Action = FLYING;                          //
    Mirror[ Position - Mirror_size.Width + 1 ].Object = Object;
    Mirror[ Position - Mirror_size.Width + 1 ].Y_Offset = 0;
    Mirror[ Position - Mirror_size.Width + 1 ].Action = FLYING;
   break;

   case 7:
    // �롨ࠥ� ���.
    Position = Mirror_size.Height * Mirror_size.Width - Rnd( Mirror_size.Width - 2 ) - 2;
    // �᫨ � �窠�, ����� ���� ���������, ��-� ���� - ������.
    if( Mirror[ Position + 1 ].Object != 0 ) return 0;
    if( Mirror[ Position - Mirror_size.Width ].Object != 0 ) return 0;
    if( Mirror[ Position - Mirror_size.Width + 1 ].Object != 0 ) return 0;
    // ��ᯮ������ �� �����孮�� ��ઠ�� ����� �।��⮢.
    Mirror[ Position + 1 ].Object = Object;
    Mirror[ Position + 1 ].Y_Offset = 0;                             //
    Mirror[ Position + 1 ].Action = FLYING;                          //
    Mirror[ Position - Mirror_size.Width ].Object = Object;          //     X
    Mirror[ Position - Mirror_size.Width ].Y_Offset = 0;             //   X X
    Mirror[ Position - Mirror_size.Width ].Action = FLYING;          //
    Mirror[ Position - Mirror_size.Width + 1 ].Object = Object;      //
    Mirror[ Position - Mirror_size.Width + 1 ].Y_Offset = 0;
    Mirror[ Position - Mirror_size.Width + 1 ].Action = FLYING;
   break;

   case 8:
    // �롨ࠥ� ���.
    Position = Mirror_size.Height * Mirror_size.Width - Rnd( Mirror_size.Width - 2 ) - 2;
    // �᫨ � �窠�, ����� ���� ���������, ��-� ���� - ������.
    if( Mirror[ Position ].Object != 0 ) return 0;
    if( Mirror[ Position + 1 ].Object != 0 ) return 0;
    if( Mirror[ Position - Mirror_size.Width ].Object != 0 ) return 0;
    // ��ᯮ������ �� �����孮�� ��ઠ�� ����� �।��⮢.
    Mirror[ Position ].Object = Object;
    Mirror[ Position ].Y_Offset = 0;                             //
    Mirror[ Position ].Action = FLYING;                          //
    Mirror[ Position + 1 ].Object = Object;                      //       X X
    Mirror[ Position + 1 ].Y_Offset = 0;                         //       X
    Mirror[ Position + 1 ].Action = FLYING;                      //
    Mirror[ Position - Mirror_size.Width ].Object = Object;      //
    Mirror[ Position - Mirror_size.Width ].Y_Offset = 0;
    Mirror[ Position - Mirror_size.Width ].Action = FLYING;
   break;

   case 9:
    // �롨ࠥ� ���.
    Position = Mirror_size.Height * Mirror_size.Width - Rnd( Mirror_size.Width - 1 ) - 1;
    // �᫨ � �窠�, ����� ���� ���������, ��-� ���� - ������.
    if( Mirror[ Position ].Object != 0 ) return 0;
    if( Mirror[ Position - Mirror_size.Width ].Object != 0 ) return 0;
    if( Mirror[ Position - Mirror_size.Width * 2 ].Object != 0 ) return 0;
    // ��ᯮ������ �� �����孮�� ��ઠ�� ����� �।��⮢.
    Mirror[ Position ].Object = Object;
    Mirror[ Position ].Y_Offset = 0;
    Mirror[ Position ].Action = FLYING;                          //
    Mirror[ Position - Mirror_size.Width ].Object = Object;      //       X
    Mirror[ Position - Mirror_size.Width ].Y_Offset = 0;         //       X
    Mirror[ Position - Mirror_size.Width ].Action = FLYING;      //       X
    Mirror[ Position - Mirror_size.Width * 2 ].Object = Object;  //
    Mirror[ Position - Mirror_size.Width * 2 ].Y_Offset = 0;
    Mirror[ Position - Mirror_size.Width * 2 ].Action = FLYING;
   break;

   case 10:
    // �롨ࠥ� ���.
    Position = Mirror_size.Height * Mirror_size.Width - Rnd( Mirror_size.Width - 3 ) - 3;
    // �᫨ � �窠�, ����� ���� ���������, ��-� ���� - ������.
    if( Mirror[ Position ].Object != 0 ) return 0;
    if( Mirror[ Position + 1 ].Object != 0 ) return 0;
    if( Mirror[ Position + 2 ].Object != 0 ) return 0;
    // ��ᯮ������ �� �����孮�� ��ઠ�� ����� �।��⮢.
    Mirror[ Position ].Object = Object;
    Mirror[ Position ].Y_Offset = 0;
    Mirror[ Position ].Action = FLYING;                          //
    Mirror[ Position + 1 ].Object = Object;                      //
    Mirror[ Position + 1 ].Y_Offset = 0;                         //       X X X
    Mirror[ Position + 1 ].Action = FLYING;                      //
    Mirror[ Position + 2 ].Object = Object;                      //
    Mirror[ Position + 2 ].Y_Offset = 0;
    Mirror[ Position + 2 ].Action = FLYING;
   break;

   case 11:
    // �롨ࠥ� ���.
    Position = Mirror_size.Height * Mirror_size.Width - Rnd( Mirror_size.Width - 3 ) - 3;
    // �᫨ � �窠�, ����� ���� ���������, ��-� ���� - ������.
    if( Mirror[ Position ].Object != 0 ) return 0;
    if( Mirror[ Position - Mirror_size.Width + 1 ].Object != 0 ) return 0;
    if( Mirror[ Position - Mirror_size.Width * 2 ].Object != 0 ) return 0;
    if( Mirror[ Position - Mirror_size.Width * 2 + 2 ].Object != 0 ) return 0;
    // ��ᯮ������ �� �����孮�� ��ઠ�� ����� �।��⮢.
    Mirror[ Position ].Object = Object;
    Mirror[ Position ].Y_Offset = 0;
    Mirror[ Position ].Action = FLYING;
    Mirror[ Position - Mirror_size.Width + 1 ].Object = Object;  //
    Mirror[ Position - Mirror_size.Width + 1 ].Y_Offset = 0;     //       X
    Mirror[ Position - Mirror_size.Width + 1 ].Action = FLYING;  //         X
    Mirror[ Position - Mirror_size.Width * 2 ].Object = Object;  //       X   X
    Mirror[ Position - Mirror_size.Width * 2 ].Y_Offset = 0;     //
    Mirror[ Position - Mirror_size.Width * 2 ].Action = FLYING;
    Mirror[ Position - Mirror_size.Width * 2 + 2 ].Object = Object;
    Mirror[ Position - Mirror_size.Width * 2 + 2 ].Y_Offset = 0;
    Mirror[ Position - Mirror_size.Width * 2 + 2 ].Action = FLYING;
   break;

   case 12:
    // �롨ࠥ� ���.
    Position = Mirror_size.Height * Mirror_size.Width - Rnd( Mirror_size.Width - 3 ) - 3;
    // �᫨ � �窠�, ����� ���� ���������, ��-� ���� - ������.
    if( Mirror[ Position ].Object != 0 ) return 0;
    if( Mirror[ Position + 2 ].Object != 0 ) return 0;
    if( Mirror[ Position - Mirror_size.Width + 1 ].Object != 0 ) return 0;
    if( Mirror[ Position - Mirror_size.Width * 2 + 2 ].Object != 0 ) return 0;
    // ��ᯮ������ �� �����孮�� ��ઠ�� ����� �।��⮢.
    Mirror[ Position ].Object = Object;
    Mirror[ Position ].Y_Offset = 0;
    Mirror[ Position ].Action = FLYING;
    Mirror[ Position + 2 ].Object = Object;                      //
    Mirror[ Position + 2 ].Y_Offset = 0;                         //       X   X
    Mirror[ Position + 2 ].Action = FLYING;                      //         X
    Mirror[ Position - Mirror_size.Width + 1 ].Object = Object;  //           X
    Mirror[ Position - Mirror_size.Width + 1 ].Y_Offset = 0;     //
    Mirror[ Position - Mirror_size.Width + 1 ].Action = FLYING;
    Mirror[ Position - Mirror_size.Width * 2 + 2 ].Object = Object;
    Mirror[ Position - Mirror_size.Width * 2 + 2 ].Y_Offset = 0;
    Mirror[ Position - Mirror_size.Width * 2 + 2 ].Action = FLYING;
   break;

   case 13:
    // �롨ࠥ� ���.
    Position = Mirror_size.Height * Mirror_size.Width - Rnd( Mirror_size.Width - 3 ) - 3;
    // �᫨ � �窠�, ����� ���� ���������, ��-� ���� - ������.
    if( Mirror[ Position + 2 ].Object != 0 ) return 0;
    if( Mirror[ Position - Mirror_size.Width + 1 ].Object != 0 ) return 0;
    if( Mirror[ Position - Mirror_size.Width * 2 ].Object != 0 ) return 0;
    if( Mirror[ Position - Mirror_size.Width * 2 + 2 ].Object != 0 ) return 0;
    // ��ᯮ������ �� �����孮�� ��ઠ�� ����� �।��⮢.
    Mirror[ Position + 2 ].Object = Object;
    Mirror[ Position + 2 ].Y_Offset = 0;
    Mirror[ Position + 2 ].Action = FLYING;
    Mirror[ Position - Mirror_size.Width + 1 ].Object = Object;  //
    Mirror[ Position - Mirror_size.Width + 1 ].Y_Offset = 0;     //           X
    Mirror[ Position - Mirror_size.Width + 1 ].Action = FLYING;  //         X
    Mirror[ Position - Mirror_size.Width * 2 ].Object = Object;  //       X   X
    Mirror[ Position - Mirror_size.Width * 2 ].Y_Offset = 0;     //
    Mirror[ Position - Mirror_size.Width * 2 ].Action = FLYING;
    Mirror[ Position - Mirror_size.Width * 2 + 2 ].Object = Object;
    Mirror[ Position - Mirror_size.Width * 2 + 2 ].Y_Offset = 0;
    Mirror[ Position - Mirror_size.Width * 2 + 2 ].Action = FLYING;
   break;

   case 14:
    // �롨ࠥ� ���.
    Position = Mirror_size.Height * Mirror_size.Width - Rnd( Mirror_size.Width - 3 ) - 3;
    // �᫨ � �窠�, ����� ���� ���������, ��-� ���� - ������.
    if( Mirror[ Position ].Object != 0 ) return 0;
    if( Mirror[ Position + 2 ].Object != 0 ) return 0;
    if( Mirror[ Position - Mirror_size.Width + 1 ].Object != 0 ) return 0;
    if( Mirror[ Position - Mirror_size.Width * 2 ].Object != 0 ) return 0;
    // ��ᯮ������ �� �����孮�� ��ઠ�� ����� �।��⮢.
    Mirror[ Position ].Object = Object;
    Mirror[ Position ].Y_Offset = 0;
    Mirror[ Position ].Action = FLYING;
    Mirror[ Position + 2 ].Object = Object;                      //
    Mirror[ Position + 2 ].Y_Offset = 0;                         //       X   X
    Mirror[ Position + 2 ].Action = FLYING;                      //         X
    Mirror[ Position - Mirror_size.Width + 1 ].Object = Object;  //       X
    Mirror[ Position - Mirror_size.Width + 1 ].Y_Offset = 0;     //
    Mirror[ Position - Mirror_size.Width + 1 ].Action = FLYING;
    Mirror[ Position - Mirror_size.Width * 2 ].Object = Object;
    Mirror[ Position - Mirror_size.Width * 2 ].Y_Offset = 0;
    Mirror[ Position - Mirror_size.Width * 2 ].Action = FLYING;
   break;

   case 15:
    // �롨ࠥ� ���.
    Position = Mirror_size.Height * Mirror_size.Width - Rnd( Mirror_size.Width - 3 ) - 3;
    // �᫨ � �窠�, ����� ���� ���������, ��-� ���� - ������.
    if( Mirror[ Position ].Object != 0 ) return 0;
    if( Mirror[ Position + 2 ].Object != 0 ) return 0;
    if( Mirror[ Position - Mirror_size.Width + 1 ].Object != 0 ) return 0;
    if( Mirror[ Position - Mirror_size.Width * 2 ].Object != 0 ) return 0;
    if( Mirror[ Position - Mirror_size.Width * 2 + 2 ].Object != 0 ) return 0;
    // ��ᯮ������ �� �����孮�� ��ઠ�� ����� �।��⮢.
    Mirror[ Position ].Object = Object;
    Mirror[ Position ].Y_Offset = 0;
    Mirror[ Position ].Action = FLYING;
    Mirror[ Position + 2 ].Object = Object;
    Mirror[ Position + 2 ].Y_Offset = 0;                         //
    Mirror[ Position + 2 ].Action = FLYING;                      //       X   X
    Mirror[ Position - Mirror_size.Width + 1 ].Object = Object;  //         X
    Mirror[ Position - Mirror_size.Width + 1 ].Y_Offset = 0;     //       X   X
    Mirror[ Position - Mirror_size.Width + 1 ].Action = FLYING;  //
    Mirror[ Position - Mirror_size.Width * 2 ].Object = Object;
    Mirror[ Position - Mirror_size.Width * 2 ].Y_Offset = 0;
    Mirror[ Position - Mirror_size.Width * 2 ].Action = FLYING;
    Mirror[ Position - Mirror_size.Width * 2 + 2 ].Object = Object;
    Mirror[ Position - Mirror_size.Width * 2 + 2 ].Y_Offset = 0;
    Mirror[ Position - Mirror_size.Width * 2 + 2 ].Action = FLYING;
   break;
  }

 // ����ᮢ뢠�� �� �।����.
 HPS Presentation_space = WinGetPS( Client_window );

 // ����⢨� �믮������ ��� ࠧ� - ��� ������ � �ࠢ��� ��ઠ��.
 for( INT Mirrors_count = 1; Mirrors_count <= 2; Mirrors_count ++ )
  {
   // �롨ࠥ� ��ઠ�� � �窨.
   MIRROR_SQUARES* Mirror = NULL;
   INT Y_First_point = 0; INT Y_Last_point = 0;
   INT X_First_point = 0; INT X_Last_point = 0;
   if( Mirrors_count == 1 )
    {
     Mirror = Left_mirror;
     Y_First_point = Mirror_points.Hy; Y_Last_point = Mirror_points.By;
     X_First_point = Mirror_points.Hx; X_Last_point = Mirror_points.Ix;
    }
   else
    {
     Mirror = Right_mirror;
     Y_First_point = Mirror_points.Jy; Y_Last_point = Mirror_points.Dy;
     X_First_point = Mirror_points.Jx; X_Last_point = Mirror_points.Kx;
    }

   // ����ᮢ뢠�� �।���� � ��࠭��� ��ઠ��.
   INT Position = 0;
   for( INT Y_Count = Y_First_point + GRID_WIDTH; Y_Count <= Y_Last_point - SPRITE_HEIGHT; Y_Count += SPRITE_HEIGHT + GRID_WIDTH )
    for( INT X_Count = X_First_point + GRID_WIDTH; X_Count <= X_Last_point - GRID_WIDTH - SPRITE_WIDTH; X_Count += SPRITE_WIDTH + GRID_WIDTH )
      {
       // ����ᮢ뢠�� �।���.
       if( Mirror[ Position ].Object != 0 )
        DrawSprite( Presentation_space, X_Count, Y_Count + Mirror[ Position ].Y_Offset, Sprite[ Mirror[ Position ].Object ] );

       // ���室�� � ��㣮�� �।����.
       Position ++;
      }
  }

 // �����蠥� ࠡ��� � ����࠭�⢥ �⮡ࠦ���� ����.
 WinReleasePS( Presentation_space );

 // ������.
 return 1;
}

// ��� ������ �� �।���� � �����孮�� ��ઠ� ���

VOID DeleteAllObjects( VOID )
{
 // ����⢨� �믮������ ��� ࠧ� - ��� ������ � �ࠢ��� ��ઠ��.
 for( INT Mirrors_count = 1; Mirrors_count <= 2; Mirrors_count ++ )
  {
   // �롨ࠥ� ��ઠ��.
   MIRROR_SQUARES* Mirror = NULL;
   if( Mirrors_count == 1 ) Mirror = Left_mirror; else Mirror = Right_mirror;

   // �� �।���� ������ ��祧����.
   for( INT Count = 0; Count < Mirror_size.Width * Mirror_size.Height; Count ++ )
    {
     Mirror[ Count ].Action = DISAPPEARING;
     Mirror[ Count ].Action_count = 0;
     Mirror[ Count ].Change_point = Rnd( STOP_POINT ) + STOP_POINT / 2;
    }
  }

 // ������.
 return;
}

// ��� ��।������ ����� �।��⮢ ���

// Client_window - ���� �ਫ������, Where - ���ࠢ����� ��������.
VOID MoveObjects( HWND Client_window, INT Where )
{
 // �᫨ �।���� ���� ��।������ ����� - ������ ��.
 if( Where == WM_LEFT )
  {
   // ����뢠��, �� ��।������� ��������.
   INT Movement_is_possible = 1;

   // ����⢨� �믮������ ��� ࠧ� - ��� ������ � �ࠢ��� ��ઠ��.
   for( INT Mirrors_count = 1; Mirrors_count <= 2; Mirrors_count ++ )
    {
     // �롨ࠥ� ��ઠ��.
     MIRROR_SQUARES* Mirror = NULL;
     if( Mirrors_count == 1 ) Mirror = Left_mirror; else Mirror = Right_mirror;

     // ��ᬠ�ਢ��� ��ઠ�� � �஢��塞, �������� �� ��।�������.
     for( INT V_Count = 0; V_Count < Mirror_size.Height; V_Count ++ )
      for( INT H_Count = 0; H_Count < Mirror_size.Width; H_Count ++ )
       {
        // �᫨ �।���� ���⨣�� ������ ��� ��ઠ�� - ��।������� ����������.
        if( H_Count == 0 )
         {
          INT Position = V_Count * Mirror_size.Width;
          if( Mirror[ Position ].Object != 0 )
           if( Mirror[ Position ].Action == FLYING || Mirror[ Position ].Action == DARTING || Mirror[ Position ].Action == PACKING )
            Movement_is_possible = 0;
         }
        // �᫨ ᫥�� ���� ���������� �।��� - ��।������� ����������.
        else
         {
          // �᫨ � ������ ���� ����騩 �।���:
          INT Position = V_Count * Mirror_size.Width + H_Count;
          if( Mirror[ Position ].Object != 0 )
           if( Mirror[ Position ].Action == FLYING || Mirror[ Position ].Action == DARTING || Mirror[ Position ].Action == PACKING )
            {
             // ����ਬ, �� �ᯮ������ ᫥��.
             INT Left_position = Position - 1;
             if( Left_position >= 0 )
              if( Mirror[ Left_position ].Object != 0 )
               if( Mirror[ Left_position ].Action != FLYING )
                if( Mirror[ Left_position ].Action != DARTING )
                 if( Mirror[ Left_position ].Action != PACKING  )
                  Movement_is_possible = 0;

             // �᫨ �।��� �� �஫�⥫ ������ - ᬮ�ਬ �� �, �� ᫥�� ������.
             if( Mirror[ Position ].Y_Offset > Rate.Flying_step )
              {
               Left_position = Position + Mirror_size.Width - 1;
               if( Left_position < Mirror_size.Width * Mirror_size.Height )
                if( Mirror[ Left_position ].Object != 0 )
                 if( Mirror[ Left_position ].Action != FLYING )
                  if( Mirror[ Left_position ].Action != DARTING )
                   if( Mirror[ Left_position ].Action != PACKING )
                    Movement_is_possible = 0;
              }

             // � �᫨ �� �஫�⥫ ������ - ᬮ�ਬ �� �, �� ᫥�� �����.
             if( Mirror[ Position ].Y_Offset < Rate.Flying_step )
              {
               Left_position = Position - Mirror_size.Width - 1;
               if( Left_position >= 0 )
                if( Mirror[ Left_position ].Object != 0 )
                 if( Mirror[ Left_position ].Action != FLYING )
                  if( Mirror[ Left_position ].Action != DARTING )
                   if( Mirror[ Left_position ].Action != PACKING )
                    Movement_is_possible = 0;
              }
            }
         }
       }
     }

   // ��।������ �।����, �᫨ �� ��������.
   if( Movement_is_possible )
    {
     // ����⠥� � ����࠭�⢥ �⮡ࠦ���� ����.
     HPS Presentation_space = WinGetPS( Client_window );

     // ����⢨� �믮������ ��� ࠧ� - ��� ������ � �ࠢ��� ��ઠ��.
     for( INT Mirrors_count = 1; Mirrors_count <= 2; Mirrors_count ++ )
      {
       // �롨ࠥ� ��ઠ�� � �窨.
       MIRROR_SQUARES* Mirror = NULL;
       INT Y_First_point = 0; INT Y_Last_point = 0;
       INT X_First_point = 0; INT X_Last_point = 0;
       if( Mirrors_count == 1 )
        {
         Mirror = Left_mirror;
         Y_First_point = Mirror_points.Hy; Y_Last_point = Mirror_points.By;
         X_First_point = Mirror_points.Hx; X_Last_point = Mirror_points.Ix;
        }
       else
        {
         Mirror = Right_mirror;
         Y_First_point = Mirror_points.Jy; Y_Last_point = Mirror_points.Dy;
         X_First_point = Mirror_points.Jx; X_Last_point = Mirror_points.Kx;
        }

       // ��।������ �।����.
       INT Position = 0;
       for( INT Y_Count = Y_First_point + GRID_WIDTH; Y_Count <= Y_Last_point - SPRITE_HEIGHT; Y_Count += SPRITE_HEIGHT + GRID_WIDTH )
        for( INT X_Count = X_First_point + GRID_WIDTH; X_Count <= X_Last_point - GRID_WIDTH - SPRITE_WIDTH; X_Count += SPRITE_WIDTH + GRID_WIDTH )
         {
          // �᫨ �।��� ����� ��।������ - ������ ��.
          if( Mirror[ Position ].Object != 0 )
           if( Mirror[ Position ].Action == FLYING || Mirror[ Position ].Action == DARTING || Mirror[ Position ].Action == PACKING )
            {
             // ���������� ���, � ���ன �ᯮ����� �।���.
             INT Previous_X = X_Count;

             // ������ ����� �ᯮ������� �।���.
             INT New_X = X_Count - GRID_WIDTH - SPRITE_WIDTH;

             // ���뢠�� �।��� � �窥, � ���ன �� �� �ᯮ�����.
             HideSprite( Presentation_space, Previous_X, Y_Count + Mirror[ Position ].Y_Offset );
             // ���㥬 �।��� � ����� �窥.
             DrawSprite( Presentation_space, New_X, Y_Count + Mirror[ Position ].Y_Offset, Sprite[ Mirror[ Position ].Object ] );

             // ��७�ᨬ �।��� � ��㣮� ������.
             INT Next_position = Position - 1;
             Mirror[ Next_position ].Object = Mirror[ Position ].Object;
             Mirror[ Next_position ].Y_Offset = Mirror[ Position ].Y_Offset;
             Mirror[ Next_position ].Action = Mirror[ Position ].Action;
             Mirror[ Next_position ].Action_count = Mirror[ Position ].Action_count;
             Mirror[ Next_position ].Change_point = Mirror[ Position ].Change_point;

             // ��࠭�� ������ �⠭������ �����.
             Mirror[ Position ].Object = 0; Mirror[ Position ].Action = 0;
            }

          // ���室�� � ᫥���饬� �।����.
          Position ++;
         }
      }

     // �����蠥� ࠡ��� � ����࠭�⢥ �⮡ࠦ���� ����.
     WinReleasePS( Presentation_space );
    }
  }

 // �᫨ �।���� ���� ��।������ ��ࠢ� - ������ ��.
 if( Where == WM_RIGHT )
  {
   // ����뢠��, �� ��।������� ��������.
   INT Movement_is_possible = 1;

   // ����⢨� �믮������ ��� ࠧ� - ��� ������ � �ࠢ��� ��ઠ��.
   for( INT Mirrors_count = 1; Mirrors_count <= 2; Mirrors_count ++ )
    {
     // �롨ࠥ� ��ઠ��.
     MIRROR_SQUARES* Mirror = NULL;
     if( Mirrors_count == 1 ) Mirror = Left_mirror; else Mirror = Right_mirror;

     // ��ᬠ�ਢ��� ��ઠ�� � �஢��塞, �������� �� ��।�������.
     for( INT V_Count = 0; V_Count < Mirror_size.Height; V_Count ++ )
      for( INT H_Count = 0; H_Count < Mirror_size.Width; H_Count ++ )
       {
        // �᫨ �।���� ���⨣�� �ࠢ��� ��� ��ઠ�� - ��।������� ����������.
        if( H_Count == Mirror_size.Width - 1 )
         {
          INT Position = V_Count * Mirror_size.Width + Mirror_size.Width - 1;
          if( Mirror[ Position ].Object != 0 )
           if( Mirror[ Position ].Action == FLYING || Mirror[ Position ].Action == DARTING || Mirror[ Position ].Action == PACKING )
            Movement_is_possible = 0;
         }
        // �᫨ �ࠢ� ���� ���������� �।��� - ��।������� ����������.
        else
         {
          // �᫨ � ������ ���� ����騩 �।���:
          INT Position = V_Count * Mirror_size.Width + H_Count;
          if( Mirror[ Position ].Object != 0 )
           if( Mirror[ Position ].Action == FLYING || Mirror[ Position ].Action == DARTING || Mirror[ Position ].Action == PACKING )
            {
             // ����ਬ, �� �ᯮ������ �ࠢ�.
             INT Right_position = Position + 1;
             if( Right_position < Mirror_size.Width * Mirror_size.Height )
              if( Mirror[ Right_position ].Object != 0 )
               if( Mirror[ Right_position ].Action != FLYING )
                if( Mirror[ Right_position ].Action != DARTING )
                 if( Mirror[ Right_position ].Action != PACKING )
                  Movement_is_possible = 0;

             // �᫨ �।��� �� �஫�⥫ ������ - ᬮ�ਬ �� �, �� �ࠢ� ������.
             if( Mirror[ Position ].Y_Offset > Rate.Flying_step )
              {
               Right_position = Position + Mirror_size.Width + 1;
               if( Right_position < Mirror_size.Width * Mirror_size.Height )
                if( Mirror[ Right_position ].Object != 0 )
                 if( Mirror[ Right_position ].Action != FLYING )
                  if( Mirror[ Right_position ].Action != DARTING )
                   if( Mirror[ Right_position ].Action != PACKING )
                    Movement_is_possible = 0;
              }

             // � �᫨ �� �஫�⥫ ������ - ᬮ�ਬ �� �, �� �ࠢ� �����.
             if( Mirror[ Position ].Y_Offset < Rate.Flying_step )
              {
               Right_position = Position - Mirror_size.Width + 1;
               if( Right_position >= 0 )
                if( Mirror[ Right_position ].Object != 0 )
                 if( Mirror[ Right_position ].Action != FLYING )
                  if( Mirror[ Right_position ].Action != DARTING )
                   if( Mirror[ Right_position ].Action != PACKING )
                    Movement_is_possible = 0;
              }
            }
         }
       }
    }

   // ��।������ �।����, �᫨ �� ��������.
   if( Movement_is_possible )
    {
     // ����⠥� � ����࠭�⢥ �⮡ࠦ���� ����.
     HPS Presentation_space = WinGetPS( Client_window );

     // ����⢨� �믮������ ��� ࠧ� - ��� ������ � �ࠢ��� ��ઠ��.
     for( INT Mirrors_count = 1; Mirrors_count <= 2; Mirrors_count ++ )
      {
       // �롨ࠥ� ��ઠ�� � �窨.
       MIRROR_SQUARES* Mirror = NULL;
       INT Y_First_point = 0; INT Y_Last_point = 0;
       INT X_First_point = 0; INT X_Last_point = 0;
       if( Mirrors_count == 1 )
        {
         Mirror = Left_mirror;
         Y_First_point = Mirror_points.Hy; Y_Last_point = Mirror_points.By;
         X_First_point = Mirror_points.Hx; X_Last_point = Mirror_points.Ix;
        }
       else
        {
         Mirror = Right_mirror;
         Y_First_point = Mirror_points.Jy; Y_Last_point = Mirror_points.Dy;
         X_First_point = Mirror_points.Jx; X_Last_point = Mirror_points.Kx;
        }

       // ��।������ �।����.
       INT Position = Mirror_size.Width * Mirror_size.Height - 1;
       for( INT Y_Count = Y_Last_point - SPRITE_HEIGHT - GRID_WIDTH; Y_Count >= Y_First_point; Y_Count -= SPRITE_HEIGHT + GRID_WIDTH )
        for( INT X_Count = X_Last_point - GRID_WIDTH - SPRITE_WIDTH; X_Count >= X_First_point + GRID_WIDTH; X_Count -= SPRITE_WIDTH + GRID_WIDTH )
         {
          // �᫨ �।��� ����� ��।������ - ������ ��.
          if( Mirror[ Position ].Object != 0 )
           if( Mirror[ Position ].Action == FLYING || Mirror[ Position ].Action == DARTING || Mirror[ Position ].Action == PACKING )
            {
             // ���������� ���, � ���ன �ᯮ����� �।���.
             INT Previous_X = X_Count;

             // ������ ����� �ᯮ������� �।���.
             INT New_X = X_Count + GRID_WIDTH + SPRITE_WIDTH;

             // ���뢠�� �।��� � �窥, � ���ன �� �� �ᯮ�����.
             HideSprite( Presentation_space, Previous_X, Y_Count + Mirror[ Position ].Y_Offset );
             // ���㥬 �।��� � ����� �窥.
             DrawSprite( Presentation_space, New_X, Y_Count + Mirror[ Position ].Y_Offset, Sprite[ Mirror[ Position ].Object ] );

             // ��७�ᨬ �।��� � ��㣮� ������.
             INT Next_position = Position + 1;
             Mirror[ Next_position ].Object = Mirror[ Position ].Object;
             Mirror[ Next_position ].Y_Offset = Mirror[ Position ].Y_Offset;
             Mirror[ Next_position ].Action = Mirror[ Position ].Action;
             Mirror[ Next_position ].Action_count = Mirror[ Position ].Action_count;
             Mirror[ Next_position ].Change_point = Mirror[ Position ].Change_point;

             // ��࠭�� ������ �⠭������ �����.
             Mirror[ Position ].Object = 0; Mirror[ Position ].Action = 0;
            }

          // ���室�� � ᫥���饬� �।����.
          Position --;
         }
      }

     // �����蠥� ࠡ��� � ����࠭�⢥ �⮡ࠦ���� ����.
     WinReleasePS( Presentation_space );
    }
  }

 // �᫨ �।���� ���� �᪮��� - ������ ��.
 if( Where == WM_DOWN )
  for( INT Count = 0; Count < Mirror_size.Width * Mirror_size.Height; Count ++ )
   {
    if( Left_mirror[ Count ].Object != 0 ) if( Left_mirror[ Count ].Action == FLYING )
     Left_mirror[ Count ].Action = DARTING;
    if( Right_mirror[ Count ].Object != 0 ) if( Right_mirror[ Count ].Action == FLYING )
     Right_mirror[ Count ].Action = DARTING;
   }

 // �᫨ �।���� ���� ��७��� � ��㣮� ��ઠ�� - ������ ��.
 if( Where == WM_BOUNCE )
  {
   // ����뢠��, �� ��।������� ��������.
   INT Bounce_is_possible = 1;

   // ����⢨� �믮������ ��� ࠧ� - ��� ������ � �ࠢ��� ��ઠ��.
   for( INT Mirrors_count = 1; Mirrors_count <= 2; Mirrors_count ++ )
    {
     // �롨ࠥ� ��ઠ��.
     MIRROR_SQUARES* Mirror = NULL; MIRROR_SQUARES* Bounce_mirror = NULL;
     if( Mirrors_count == 1 )
      { Mirror = Left_mirror; Bounce_mirror = Right_mirror; }
     else
      { Mirror = Right_mirror; Bounce_mirror = Left_mirror; }

     // ��ᬠ�ਢ��� ��ઠ�� � �஢��塞, �������� �� ��।�������.
     for( INT V_Count = 0; V_Count < Mirror_size.Height; V_Count ++ )
      for( INT H_Count = 0; H_Count < Mirror_size.Width; H_Count ++ )
       {
        // �᫨ � ��㣮� ��ઠ�� ���� ���������� �।��� - ��।������� ����������.
        INT Position = V_Count * Mirror_size.Width + H_Count;
        if( Mirror[ Position ].Object != 0 )
         if( Mirror[ Position ].Action == FLYING || Mirror[ Position ].Action == DARTING || Mirror[ Position ].Action == PACKING )
          {
           // ����ਬ, �� �ᯮ������ � ��㣮� ��ઠ��.
           INT Mirrored_position = ( V_Count * Mirror_size.Width + Mirror_size.Width - 1 ) - H_Count;
           INT Bounce_position = Mirrored_position;
           if( Bounce_mirror[ Bounce_position ].Object != 0 )
            if( Bounce_mirror[ Bounce_position ].Action != FLYING )
             if( Bounce_mirror[ Bounce_position ].Action != DARTING )
              if( Bounce_mirror[ Bounce_position ].Action != PACKING  )
               Bounce_is_possible = 0;

           // �᫨ �।��� �� �஫�⥫ ������ - ᬮ�ਬ �� �, �� � ��㣮� ��ઠ�� ������.
           if( Mirror[ Position ].Y_Offset > Rate.Flying_step )
            {
             Bounce_position = Mirrored_position + Mirror_size.Width;
             if( Bounce_position < Mirror_size.Width * Mirror_size.Height )
              if( Bounce_mirror[ Bounce_position ].Object != 0 )
               if( Bounce_mirror[ Bounce_position ].Action != FLYING )
                if( Bounce_mirror[ Bounce_position ].Action != DARTING )
                 if( Bounce_mirror[ Bounce_position ].Action != PACKING )
                  Bounce_is_possible = 0;
            }

           // � �᫨ �� �஫�⥫ ������ - ᬮ�ਬ �� �, �� � ��㣮� ��ઠ�� �����.
           if( Mirror[ Position ].Y_Offset < Rate.Flying_step )
            {
             Bounce_position = Mirrored_position - Mirror_size.Width;
             if( Bounce_position >= 0 )
              if( Bounce_mirror[ Bounce_position ].Object != 0 )
               if( Bounce_mirror[ Bounce_position ].Action != FLYING )
                if( Bounce_mirror[ Bounce_position ].Action != DARTING )
                 if( Bounce_mirror[ Bounce_position ].Action != PACKING )
                  Bounce_is_possible = 0;
            }
          }
       }
     }

   // ��७�ᨬ �।����, �᫨ �� ��������.
   if( Bounce_is_possible )
    {
     // ����⠥� � ����࠭�⢥ �⮡ࠦ���� ����.
     HPS Presentation_space = WinGetPS( Client_window );

     // ����⢨� �믮������ ��� ࠧ� - ��� ������ � �ࠢ��� ��ઠ��.
     INT Bounce_is_performed = 0;
     for( INT Mirrors_count = 1; Mirrors_count <= 2; Mirrors_count ++ )
      {
       // �롨ࠥ� ��ઠ�� � �窨.
       MIRROR_SQUARES* Mirror = NULL; MIRROR_SQUARES* Bounce_mirror = NULL;
       INT Y_First_point = 0; INT X_First_point = 0;
       if( Mirrors_count == 1 )
        {
         Mirror = Left_mirror; Bounce_mirror = Right_mirror;
         Y_First_point = Mirror_points.Hy; X_First_point = Mirror_points.Hx;
        }
       else
        {
         Mirror = Right_mirror; Bounce_mirror = Left_mirror;
         Y_First_point = Mirror_points.Jy; X_First_point = Mirror_points.Jx;
        }

       // ��७�ᨬ �।����.
       if( !Bounce_is_performed )
        for( INT V_Count = 0; V_Count < Mirror_size.Height; V_Count ++ )
         for( INT H_Count = 0; H_Count < Mirror_size.Width; H_Count ++ )
          {
           // �᫨ �।��� ����� ��७��� - ������ ��.
           INT Position = V_Count * Mirror_size.Width + H_Count;
           if( Mirror[ Position ].Object != 0 )
            if( Mirror[ Position ].Action == FLYING || Mirror[ Position ].Action == DARTING )
             {
              // ���뢠�� �।��� � �窥, � ���ன �� �� �ᯮ�����.
              INT X = X_First_point + GRID_WIDTH + ( SPRITE_WIDTH + GRID_WIDTH ) * H_Count;
              INT Y = Y_First_point + GRID_WIDTH + ( SPRITE_HEIGHT + GRID_WIDTH ) * V_Count + Mirror[ Position ].Y_Offset;
              HideSprite( Presentation_space, X, Y );

              // ��७�ᨬ �।��� � ��㣮� ��ઠ��.
              INT Bounce_position = ( V_Count * Mirror_size.Width + Mirror_size.Width - 1 ) - H_Count;
              Bounce_mirror[ Bounce_position ].Object = Mirror[ Position ].Object;
              Bounce_mirror[ Bounce_position ].Y_Offset = Mirror[ Position ].Y_Offset;
              Bounce_mirror[ Bounce_position ].Action = Mirror[ Position ].Action;

              // ��࠭�� ������ �⠭������ �����.
              Mirror[ Position ].Object = 0; Mirror[ Position ].Action = 0;

              // ��ன ࠧ �⮣� ������ �� ����.
              Bounce_is_performed = 1;
             }
          }
      }

     // �����蠥� ࠡ��� � ����࠭�⢥ �⮡ࠦ���� ����.
     WinReleasePS( Presentation_space );
    }
  }

 // ������.
 return;
}

// ��� ��ࠡ��뢠�� ᮮ�饭�� �� ��� ���

VOID TranslateMouseMessages( INT X_Point, INT Y_Point )
{
 // ��窨 ����, ����� ��।����� �ᯮ������� ����� �।��⮢.
 INT X_Left_brink = 0; INT X_Right_brink = 0; INT X_Center = 0;
 // ������� ��ઠ��, ����� ��।����� �ᯮ������� ����� �।��⮢.
 INT Left_H_Count = Mirror_size.Width; INT Right_H_Count = 0;

 // ����⢨� �믮������ ��� ࠧ� - ��� ������ � �ࠢ��� ��ઠ��.
 for( INT Mirrors_count = 1; Mirrors_count <= 2; Mirrors_count ++ )
  {
   // �롨ࠥ� ��ઠ�� � �窨.
   MIRROR_SQUARES* Mirror = NULL;
   INT X_First_point = 0;
   if( Mirrors_count == 1 )
    { Mirror = Left_mirror; X_First_point = Mirror_points.Hx; }
   else
    { Mirror = Right_mirror; X_First_point = Mirror_points.Jx; }

   // ������ ������ ��ઠ��, � ���஬ �ᯮ����� ����� �।��⮢.
   for( INT V_Count = 0; V_Count < Mirror_size.Height; V_Count ++ )
    for( INT H_Count = 0; H_Count < Mirror_size.Width; H_Count ++ )
     {
      // �᫨ � ��ઠ�� ���� ����騩 �।��� - ���������� ��� �ᯮ�������.
      INT Position = V_Count * Mirror_size.Width + H_Count;
      if( Mirror[ Position ].Object != 0 )
       if( Mirror[ Position ].Action == FLYING || Mirror[ Position ].Action == DARTING )
        {
         // ���������� �ᯮ������� �।��⮢ � ��ઠ��.
         if( H_Count < Left_H_Count ) Left_H_Count = H_Count;
         if( H_Count > Right_H_Count ) Right_H_Count = H_Count;
         // ����塞 �窨 ����, ����� ��।����� �ᯮ������� ����� �।��⮢.
         X_Left_brink = X_First_point + GRID_WIDTH + ( SPRITE_WIDTH + GRID_WIDTH ) * Left_H_Count;
         X_Right_brink = X_First_point + GRID_WIDTH + ( SPRITE_WIDTH + GRID_WIDTH ) * Right_H_Count + SPRITE_WIDTH;
         X_Center = ( X_Left_brink + X_Right_brink ) / 2;
        }
     }
  }

 // ������ ���� �ਫ������ - ��� ������ ���� ��࠭��.
 HWND Client_window = WinQueryFocus( HWND_DESKTOP );

 // �᫨ 㪠��⥫� ��� �ᯮ����� ��� ��ઠ��� - ��祣� �� ������, ������.
 if( Y_Point > Mirror_points.Ay - SPRITE_HEIGHT - GRID_WIDTH )
  if( X_Point > Mirror_points.Ax ) if( X_Point < Mirror_points.Fx ) return;

 // �᫨ 㪠��⥫� ��� �ᯮ����� ��� ��ઠ��� - ���� �᪮��� �।����.
 if( Y_Point < Mirror_points.Hy )
  {
   // ���뫠�� ᮮ�饭�� � ��⮪.
   WinPostQueueMsg( Thread_message_queue, WM_DOWN, (MPARAM) Client_window, 0 );
   // ������.
   return;
  }

 // �᫨ �।���� �ᯮ������ � ����� ��ઠ��, � 㪠��⥫� ��� � �ࠢ�� - �।���� ���� ��७��� � ��㣮� ��ઠ��.
 INT Bounce = 0;
 if( X_Center > Mirror_points.Hx ) if( X_Center < Mirror_points.Ix )
  if( X_Point > Mirror_points.Jx ) Bounce = 1;

 // �᫨ �।���� �ᯮ������ � �ࠢ�� ��ઠ��, � 㪠��⥫� ��� � ����� - �।���� ���� ��७��� � ��㣮� ��ઠ��.
 if( X_Center > Mirror_points.Jx ) if( X_Center < Mirror_points.Kx )
  if( X_Point < Mirror_points.Ix ) Bounce = 1;

 // ��७�ᨬ �।���� � ��㣮� ��ઠ��.
 if( Bounce == 1 )
  {
   // ���뫠�� ᮮ�饭�� � ��⮪.
   WinPostQueueMsg( Thread_message_queue, WM_BOUNCE, (MPARAM) Client_window, 0 );
   // ������.
   return;
  }

 // �᫨ 㪠��⥫� ��� �ᯮ����� ᫥�� �� �।��� - ���� ��।������ ���.
 if( X_Point < X_Center )
  {
   // ���뫠�� ᮮ�饭�� � ��⮪.
   WinPostQueueMsg( Thread_message_queue, WM_LEFT, (MPARAM) Client_window, 0 );
   // ������.
   return;
  }

 // �᫨ 㪠��⥫� ��� �ᯮ����� �ࠢ� �� �।��� - ���� ��।������ ���.
 if( X_Point > X_Center )
  {
   // ���뫠�� ᮮ�饭�� � ��⮪.
   WinPostQueueMsg( Thread_message_queue, WM_RIGHT, (MPARAM) Client_window, 0 );
   // ������.
   return;
  }

 // ������.
 return;
}
