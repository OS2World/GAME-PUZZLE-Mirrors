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

// Общие переменные.
struct SYSTEM_METRICS
 {
  // Страна, в которой работает приложение.
  INT Code_page;
  // Размер экрана.
  INT X_Screen;
  INT Y_Screen;
 };
SYSTEM_METRICS System_metrics = { 0, 0, 0 };

// Приложение.
HAB Application = NULLHANDLE;

// Очередь сообщений потока.
HMQ Thread_message_queue = NULLHANDLE;

// Ответы от потока.
struct THREAD_RESPONDS
 {
  // Поток создан. 0 - нет, 1 - да, -1 - произошла ошибка.
  INT Thread_is_created;
  // Сообщение о перерисовке окна принято.
  INT Draw_message_is_received;
  // Сообщение от счетчика времени принято.
  INT Timer_message_is_received;
 };
THREAD_RESPONDS Thread_responds = { 0, 1, 1 };

// Изображение, которое используется в окне.
struct BACKGROUND
 {
  // Изображение.
  HBITMAP Bitmap;
  // Ширина.
  INT Bitmap_width;
  // Высота.
  INT Bitmap_height;
  // Изображение, подготовленное в памяти.
  HBITMAP Tile;
 };
BACKGROUND Background = { NULLHANDLE, 0, 0, NULLHANDLE };

// Настройки для игры.
struct SETTINGS
 {
  // Состояние игры. 0 - заставка, 1 - начало игры, 2 - игра, 3 - завершение игры.
  INT Game_mode;
  // Есть ли сетка. 0 - нет, 1 - есть.
  INT Grid_is_visible;
  // Остановлена ли игра. 0 - нет, 1 - есть.
  INT Game_is_paused;
  // Можно ли бросать сложные предметы. 0 - нет, 1 - да.
  INT Game_is_difficult;
  // Включен ли звук. 0 - нет, 1 - да.
  INT Sound_is_enabled;
 };
SETTINGS Settings = { 0, 1, 0, 1, 1 };

// Строки текста, которые изображаются в окне.
struct STRINGS_IN_WINDOW
 {
  // Строки меню.
  CHAR Menu_strings[ 3 ][ 20 ];
  // Название.
  CHAR Title[ 25 ];
  // Описание игры.
  CHAR Description[ 6 ][ 50 ];
  // Разработчик.
  CHAR Developer[ 50 ];
 };
STRINGS_IN_WINDOW Strings_in_window = { NULL, NULL, NULL, NULL };

// Размеры зеркал зависят от размеров окна.
struct MIRROR_SIZE
 {
  // Ширина зеркала.
  INT Width;
  // Высота зеркала.
  INT Height;
 };
MIRROR_SIZE Mirror_size = { 0, 0 };

// Зеркала разделяются на квадраты.
struct MIRROR_SQUARES
 {
  // Что расположено в этом квадрате? 0 - ничего, 1-8 - ящик, 9-48 - предмет.
  INT Object;
  // Высота, на которой должен быть нарисован предмет. Может быть меньше нуля.
  INT Y_Offset;
  // Действие - полет, движение, или падение.
  INT Action;
  // Счетчики - работают, прежде чем предмет превратится в ящик.
  INT Action_count; INT Change_point;
 };
MIRROR_SQUARES* Left_mirror = NULL; MIRROR_SQUARES* Right_mirror = NULL;

// Точки в окне, по которым должны быть нарисованы зеркала.
// A B C D E F
//  ║   ║   ║
// G║H I║J K║L
//  ╚═══╩═══╝
struct MIRROR_POINTS
 {
  INT Ax; INT Ay; INT Bx; INT By; INT Cx; INT Cy;
  INT Dx; INT Dy; INT Ex; INT Ey; INT Fx; INT Fy;
  INT Gx; INT Gy; INT Hx; INT Hy; INT Ix; INT Iy;
  INT Jx; INT Jy; INT Kx; INT Ky; INT Lx; INT Ly;
 };
MIRROR_POINTS Mirror_points = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

// Пространство отображения в памяти.
HPS Memory_space = NULLHANDLE;

// Изображения предметов.
struct SPRITE
 {
  // Изображение.
  HBITMAP Image;
  // Основа для него.
  HBITMAP Base;
 };
SPRITE Sprite[ MAX_SPRITE + 1 ];

// Скорости передвижения предметов.
struct RATE
 {
  // Полет сверху вниз.
  INT Flying_step;
  // Стремительное движение.
  INT Darting_step;
  // Падение.
  INT Throwing_step;
 };
RATE Rate = { MAX_STEP - 1, MAX_STEP * 3, MAX_STEP };

// Звук.
struct SOUND
 {
  // Имена звуковых файлов, которые использованы в игре.
  CHAR Sound_file[ MAX_SOUND + 1 ][ 255 ];
  // Звуки, загруженные в память. Они соответствуют именам файлов.
  HSOUND Sound_in_memory[ MAX_SOUND + 1 ];
  // Вызванное приложение: архив, который содержит звук.
  PID Sound_application;
  // Указывает, что в приложении воспроизводится звук.
  INT Sound_is_playing;
 };
SOUND Sound = { NULL, NULL, NULLHANDLE, 0 };

// Возвращает случайное число от 0 до Max.
INT Rnd( INT Max );

// Переносит звуковые файлы во временный каталог, задает имена файлов
VOID PrepareSoundFiles( VOID );

// Удаляет звуковые файлы во временном каталоге.
VOID DeleteSoundFiles( VOID );

// Подготавливает в памяти изображение, которое используется в окне.
VOID CreateBackgroundTile( VOID );

// Рисует изображение предмета.
VOID DrawSprite( HPS Presentation_space, INT X, INT Y, SPRITE Sprite );

// Скрывает предмет, восстанавливая часть картинки в окне.
VOID HideSprite( HPS Presentation_space, INT X, INT Y );

// Принимает сообщения, которые приходят в окно приложения. Сообщения передаются в поток.
MRESULT EXPENTRY WinProc( HWND Window, ULONG Message, MPARAM First_parameter, MPARAM Second_parameter );

// Поток для рисования.
VOID ThreadProc( VOID );

// Обработчик сообщений, которые были переданы в поток рисования.
VOID MessageProcessing( ULONG Message, MPARAM First_parameter, MPARAM Second_parameter );

// Добавляет строки в меню окна.
VOID AddMenu( HWND Frame_window );

// Ставит отметки в строки меню или снимает их.
VOID CheckMenuItems( HWND Frame_window );

// Делает зеркала пустыми.
VOID CleanMirrors( VOID );

// Располагает на поверхности зеркал несколько предметов.
VOID ThrowObjects( VOID );

// Рисует все в окне.
VOID DrawImageInWindow( HWND Client_window );

// Вычисляет размеры зеркал.
VOID CalculateMirrorSize( RECTL Window_rectangle );

// Вычисляет точки, по которым надо нарисовать зеркала.
VOID CalculateMirrorPoints( RECTL Window_rectangle );

// Обрабатывает сообщение от счетчика времени, передвигает предметы.
VOID AnimateObjects( HWND Client_window );

// Удаляет ряды.
INT DeleteRows( HWND Client_window );

// Проверяет, есть ли в зеркалах "повисшие" ящики и бросает их.
VOID DetectHangedBoxes( VOID );

// Располагает на поверхности зеркал набор предметов.
INT ThrowObjectSet( HWND Client_window );

// Удаляет все предметы с поверхности зеркал.
VOID DeleteAllObjects( VOID );

// Передвигает набор предметов.
VOID MoveObjects( HWND Client_window, INT Where );

// Обрабатывает сообщения от мыши.
VOID TranslateMouseMessages( INT X_Point, INT Y_Point );

// ─── Приложение ───

INT main( VOID )
{
 // Определяем приложение в системе.
 Application = WinInitialize( 0 );
 // Если это сделать не удалось - выход.
 if( Application == NULLHANDLE )
  {
   // Звук - ошибка.
   WinAlarm( HWND_DESKTOP, WA_ERROR );
   // Выход.
   return 0;
  }

 // Создаем поток для обработки сообщений
 TID Thread_ID = NULLHANDLE;
 APIRET Thread_is_created = DosCreateThread( &Thread_ID, (PFNTHREAD) ThreadProc, 0, 0, 8192 );

 // Если поток создан - ждем, пока в нем будет создана очередь сообщений.
 if( Thread_is_created == NO_ERROR ) while( Thread_responds.Thread_is_created == 0 ) { DosSleep( 1 ); }

 // Если поток создать не удалось - выход.
 if( Thread_is_created != NO_ERROR || Thread_responds.Thread_is_created == -1 )
  {
   // Звук - ошибка.
   WinAlarm( HWND_DESKTOP, WA_ERROR );
   // Выход.
   WinTerminate( Application ); return 0;
  }

 // Создаем очередь сообщений.
 HMQ Message_queue = WinCreateMsgQueue( Application, 0 );
 // Если очередь создать не удалось - выход.
 if( Message_queue == NULLHANDLE )
  {
   // Звук - ошибка.
   WinAlarm( HWND_DESKTOP, WA_ERROR );
   // Выход.
   DosKillThread( Thread_ID ); WinTerminate( Application ); return 0;
  }

 // Устанавливаем приоритет приложения.
 DosSetPriority( PRTYS_PROCESS, PRTYC_IDLETIME, PRTYD_MAXIMUM / 2, 0 );

 // Запоминаем страну, в которой работает приложение.
 System_metrics.Code_page = WinQueryCp( Message_queue );

 // Запоминаем имя, под которым определено окно приложения.
 CHAR AppWindow_name[] = "Mirrors";

 // Определяем окно в системе.
 UINT Window_is_registered = WinRegisterClass( Application, AppWindow_name, (PFNWP) WinProc, 0, 0 );
 // Если это сделать не удалось - выход.
 if( !Window_is_registered )
  {
   // Звук - ошибка.
   WinAlarm( HWND_DESKTOP, WA_ERROR );
   // Выход.
   DosKillThread( Thread_ID );
   WinDestroyMsgQueue( Message_queue ); WinTerminate( Application ); return 0;
  }

 // Как будет выглядеть рамка окна.
 ULONG Frame_style = CS_FRAME | WS_ANIMATE;
 // Как будет выглядеть рабочая область окна.
 ULONG Client_style = 0;
 // Будут ли в окне заголовок и кнопки.
 ULONG Window_style = FCF_TITLEBAR | FCF_SYSMENU | FCF_MINBUTTON | FCF_MAXBUTTON | FCF_SIZEBORDER | FCF_TASKLIST;

 // Строка заголовка - зависит от страны.
 CHAR AppWindow_title[ 10 ] = "";
 INT Strings_offset = 0; if( System_metrics.Code_page != RUSSIAN ) Strings_offset = 100;
 WinLoadString( Application, NULLHANDLE, Strings_offset + 1, sizeof( AppWindow_title ), AppWindow_title );

 // Создаем окно.
 HWND Client_window = NULLHANDLE;
 HWND Frame_window = WinCreateStdWindow( HWND_DESKTOP, Frame_style, &Window_style, AppWindow_name, AppWindow_title, Client_style, 0, 0, &Client_window );
 // Если окно создать не удалось - выход.
 if( Frame_window == NULLHANDLE )
  {
   // Звук - ошибка.
   WinAlarm( HWND_DESKTOP, WA_ERROR );
   // Выход.
   DosKillThread( Thread_ID );
   WinDestroyMsgQueue( Message_queue ); WinTerminate( Application ); return 0;
  }

 // Узнаем размер экрана.
 System_metrics.X_Screen = WinQuerySysValue( HWND_DESKTOP, SV_CXSCREEN );
 System_metrics.Y_Screen = WinQuerySysValue( HWND_DESKTOP, SV_CYSCREEN );

 // Вычисляем размеры зеркал.
 RECTL Rectangle = { 0, 0, System_metrics.X_Screen, System_metrics.Y_Screen };
 CalculateMirrorSize( Rectangle );

 // Выделяем память для зеркал.
 Left_mirror = new MIRROR_SQUARES[ Mirror_size.Width * Mirror_size.Height ];
 Right_mirror = new MIRROR_SQUARES[ Mirror_size.Width * Mirror_size.Height ];

 // Если память выделить не удалось - выход.
 if( Left_mirror == NULL || Right_mirror == NULL )
  {
   // Звук - ошибка.
   WinAlarm( HWND_DESKTOP, WA_ERROR );
   // Выход.
   WinDestroyWindow( Frame_window ); DosKillThread( Thread_ID );
   WinDestroyMsgQueue( Message_queue ); WinTerminate( Application ); return 0;
  }

 // Создаем пространство отображения в памяти.
 DEVOPENSTRUC Display = { 0, "Display", NULL, 0, 0, 0, 0, 0, 0 }; SIZEL Size = { 0, 0 };
 HDC Device = DevOpenDC( Application, OD_MEMORY, "*", 5, (PDEVOPENDATA) &Display, NULLHANDLE );
 Memory_space = GpiCreatePS( Application, Device, &Size, PU_PELS | GPIA_ASSOC );
 DevCloseDC( Device );

 // Если пространство отображения создать не удалось - выход.
 if( Memory_space == GPI_ERROR )
  {
   // Звук - ошибка.
   WinAlarm( HWND_DESKTOP, WA_ERROR );
   // Выход.
   delete Left_mirror; delete Right_mirror;
   WinDestroyWindow( Frame_window ); DosKillThread( Thread_ID );
   WinDestroyMsgQueue( Message_queue ); WinTerminate( Application ); return 0;
  }

 // Запускаем датчик случайных чисел.
 INT Time = WinGetCurrentTime( Application ); srand( Time );

 // Делаем зеркала пустыми и располагаем на поверхности несколько предметов.
 CleanMirrors(); ThrowObjects();

 // Загружаем картинку, чтобы установить ее в левом верхнем углу окна.
 HPOINTER Window_icon = WinLoadPointer( HWND_DESKTOP, NULL, RC_ICON );

 // Если она есть - запоминаем это, иначе используем обычную картинку.
 BYTE Icon_is_loaded = 0;
 if( Window_icon != NULLHANDLE ) Icon_is_loaded = 1;
 else Window_icon = WinQuerySysPointer( HWND_DESKTOP, SPTR_DISPLAY_PTRS, 0 );

 // Устанавливаем картинку в левом верхнем углу окна.
 WinSendMsg( Frame_window, WM_SETICON, (MPARAM) Window_icon, 0 );

 // Загружаем строки для меню.
 for( INT Count = 2; Count <= 4; Count ++ )
  WinLoadString( Application, NULLHANDLE, Strings_offset + Count, sizeof( Strings_in_window.Menu_strings[ Count - 2 ] ), Strings_in_window.Menu_strings[ Count - 2 ] );

 // Добавляем строки в меню, которое вызывается при нажатии на картинку.
 AddMenu( Frame_window );

 // Загружаем изображения.
 HPS Presentation_space = WinGetPS( Client_window );
 // Загружаем изображения предметов. Предмет 0 - пустой квадрат.
 Sprite[ 0 ].Image = NULLHANDLE;
 for( Count = 1; Count <= MAX_SPRITE; Count ++ )
  {
   // Загружаем изображение.
   Sprite[ Count ].Image = GpiLoadBitmap( Presentation_space, NULL, RC_SPRITES + Count - 1, 0, 0 );
   // Загружаем основу.
   Sprite[ Count ].Base =  GpiLoadBitmap( Presentation_space, NULL, RC_BASES + Count - 1, 0, 0 );
  }
 // Загружаем картинку, которая будет использована в окне.
 Background.Bitmap = GpiLoadBitmap( Presentation_space, NULL, RC_BITMAPS + Rnd( 1 ), 0, 0 );
 WinReleasePS( Presentation_space );

 // Запоминаем размеры картинки.
 BITMAPINFOHEADER Bitmap_info; GpiQueryBitmapParameters( Background.Bitmap, &Bitmap_info );
 Background.Bitmap_width = Bitmap_info.cx; Background.Bitmap_height = Bitmap_info.cy;

 // Подготавливаем картинку в памяти.
 CreateBackgroundTile();

 // Загружаем строки - название игры.
 WinLoadString( Application, NULLHANDLE, Strings_offset + 5, sizeof( Strings_in_window.Title ), Strings_in_window.Title );
 // Описание и правила.
 for( Count = 6; Count <= 11; Count ++ )
  WinLoadString( Application, NULLHANDLE, Strings_offset + Count, sizeof( Strings_in_window.Description[ Count - 6 ] ), Strings_in_window.Description[ Count - 6 ] );
 // Разработчик.
 WinLoadString( Application, NULLHANDLE, Strings_offset + 12, sizeof( Strings_in_window.Developer ), Strings_in_window.Developer );

 // Подключаем MMOS2;
 INT MMOS2_is_loaded = LoadMMOS2();

 // Переносим звуковые файлы во временный каталог и задаем имена файлов.
 if( MMOS2_is_loaded ) PrepareSoundFiles();

 // Задаем расположение окна на экране.
 INT X_position = 1; INT Y_position = 1;
 INT X_size = System_metrics.X_Screen - 2;
 INT Y_size = System_metrics.Y_Screen - 2;
 WinSetWindowPos( Frame_window, HWND_TOP, X_position, Y_position, X_size, Y_size, SWP_SIZE | SWP_MOVE | SWP_ZORDER | SWP_NOADJUST );

 // Окно будет увеличено во весь экран.
 WinSendMsg( Frame_window, WM_SYSCOMMAND, (MPARAM) SC_MAXIMIZE, 0 );

 // Вызываем окно приложения и делаем его выбранным.
 WinShowWindow( Frame_window, 1 ); WinSetActiveWindow( HWND_DESKTOP, Frame_window );

 // Будет загружаться звук - обновляем окно приложения.
 WinUpdateWindow( Client_window );

 // Звук, возможно, еще не подготовлен. Ждем 1/4 секунды.
 DosSleep( 250 );

 // Звук - заставка.
 WinPostMsg( Client_window, WM_PLAYSOUND, (MPARAM) SOUND_BEGIN, 0 );

 // Получение и передача сообщений окну приложения - пока оно не будет закрыто.
 QMSG Message = { 0, 0, 0, 0, 0, 0, 0 };
 while( WinGetMsg( Application, &Message, 0, 0, 0 ) ) WinDispatchMsg( Application, &Message );

 // Останавливаем счетчик времени.
 WinStopTimer( Application, Client_window, 1 );

 // Закрываем окно.
 WinDestroyWindow( Frame_window );

 // Звук - завершение.
 if( Settings.Sound_is_enabled ) PlayAudioFile( Sound.Sound_file[ SOUND_END ] );

 // Удаляем звуковые файлы во временном каталоге.
 if( MMOS2_is_loaded ) DeleteSoundFiles();

 // Отключаем MMOS2.
 if( MMOS2_is_loaded ) UnloadMMOS2();

 // Сбрасываем изображение, которое использовалось в окне.
 if( Background.Bitmap != NULLHANDLE ) GpiDeleteBitmap( Background.Bitmap );
 if( Background.Tile != NULLHANDLE ) GpiDeleteBitmap( Background.Tile );

 // Сбрасываем картинку, которая была установлена в левом верхнем углу окна.
 if( Icon_is_loaded ) WinDestroyPointer( Window_icon );

 // Сбрасываем пространство отображения в памяти.
 GpiDestroyPS( Memory_space );

 // Освобождаем память для зеркал.
 delete Left_mirror; delete Right_mirror;

 // Завершаем работу потока.
 DosKillThread( Thread_ID );

 // Сбрасываем очередь сообщений.
 WinDestroyMsgQueue( Message_queue );

 // Выход.
 WinTerminate( Application ); return 0;
}

// ─── Возвращает случайное число от 0 до Max ───

INT Rnd( INT Max )
{
 // Если число больше допустимого значения - возврат.
 if( Max > RAND_MAX - 1 ) return -1;

 // Получаем случайное число от 0 до 1.
 INT Integer_value = rand();

 // Число должно быть от 0 до Max.
 INT Divider = RAND_MAX / ( Max + 1 );

 // Получаем число.
 INT Result = Integer_value / Divider;
 if( Result > Max ) Result = Max;

 // Возвращаем число.
 return Result;
}

// ─── Переносит звуковые файлы во временный каталог, задает имена файлов ───

// Файлы сжаты в архив, который может развернуться самостоятельно.
// Имена должны быть установлены в переменной Sound.Sound_file[].
VOID PrepareSoundFiles( VOID )
{
 // Имена звуковых файлов и звуки в памяти неопределены.
 for( INT Count = 0; Count <= MAX_SOUND; Count ++ )
  {
   Sound.Sound_file[ Count ][ 0 ] = NULL;
   Sound.Sound_in_memory[ Count ] = NULLHANDLE;
  }

 // Узнаем свой рабочий каталог.
 ULONG Current_drive = 0; ULONG Drive_map = 0;
 DosQueryCurrentDisk( &Current_drive, &Drive_map );
 CHAR Current_directory[ 255 ] = ""; ULONG Length = 250;
 Current_directory[ 0 ] = (CHAR) Current_drive + 64;
 Current_directory[ 1 ] = ':'; Current_directory[ 2 ] = '\\';
 DosQueryCurrentDir( 0, &Current_directory[ 3 ], &Length );

 // Узнаем временный каталог. Он должен быть указан в переменной среды.
 PSZ Temp_directory = "";
 INT Result = DosScanEnv( "TEMP", (PCSZ*) &Temp_directory );
 if( Result != 0 ) DosScanEnv( "TMP", (PCSZ*) &Temp_directory );
 if( Result != 0 ) DosScanEnv( "TEMPDIR", (PCSZ*) &Temp_directory );
 if( Result != 0 ) DosScanEnv( "TMPDIR", (PCSZ*) &Temp_directory );

 // Если временный каталог не указан - возврат.
 if( Result != 0 ) return;

 // Если временный каталог завершается буквой "\" - ставим вместо нее конец строки.
 // Если это корень диска - остается буква диска и двоеточие, например, "C:".
 INT End_of_string = strlen( Temp_directory ) - 1;
 if( Temp_directory[ End_of_string ] == '\\' ) Temp_directory[ End_of_string ] = NULL;

 // Переходим во временный каталог.
 ULONG Temp_drive = (ULONG) Temp_directory[ 0 ] - 64;
 DosSetDefaultDisk( Temp_drive ); DosSetCurrentDir( Temp_directory );

 // Задаем названия звуковых файлов.
 for( Count = 0; Count <= MAX_SOUND; Count ++ ) strcpy( Sound.Sound_file[ Count ], Temp_directory );
 strcat( Sound.Sound_file[ SOUND_BEGIN ], "\\MG_Begin.wav" );
 strcat( Sound.Sound_file[ SOUND_GAME ], "\\MG_Game.wav" );
 strcat( Sound.Sound_file[ SOUND_PAUSE ], "\\MG_Pause.wav" );
 strcat( Sound.Sound_file[ SOUND_MOUSE ], "\\MG_Mouse.wav" );
 strcat( Sound.Sound_file[ SOUND_DELETE ], "\\MG_Del.wav" );
 strcat( Sound.Sound_file[ SOUND_OVER ], "\\MG_Over.wav" );
 strcat( Sound.Sound_file[ SOUND_END ], "\\MG_End.wav" );

 // Эти файлы уже могут быть во временном каталоге. Удаляем их.
 DeleteSoundFiles();

 // Задаем имя архива со звуком. Он расположен в каталоге приложения.
 CHAR Sound_archive[ 255 ] = ""; strcpy( Sound_archive, Current_directory );
 // Это может быть корень или каталог диска.
 if( strlen( Sound_archive ) > 3 ) strcat( Sound_archive, "\\" );
 strcat( Sound_archive, "Mirrors.snd" );

 // Вызываем архив со звуком как приложение. Он разворачивается во временный каталог.
 CHAR Error_string[ 1 ]; RESULTCODES Return_codes = { 0, 0 };
 DosExecPgm( Error_string, sizeof( Error_string ), EXEC_BACKGROUND, NULL, NULL, &Return_codes, Sound_archive );

 // Запоминаем вызванное приложение.
 Sound.Sound_application = Return_codes.codeTerminate;

 // Переходим в свой рабочий каталог.
 DosSetDefaultDisk( Current_drive ); DosSetCurrentDir( Current_directory );

 // Возврат.
 return;
}

// ─── Удаляет звуковые файлы во временном каталоге ───

// Имена файлов установлены в переменной Sound.Sound_file[].
VOID DeleteSoundFiles( VOID )
{
 // Если звуковых файлов нет - возврат.
 if( Sound.Sound_file[ 0 ][ 0 ] == NULL ) return;

 // Останавливаем вызванное приложение, это архив со звуком.
 DosKillProcess( DKP_PROCESS, Sound.Sound_application );

 // Удаляем звуковые файлы во временном каталоге.
 for( INT Count = 0; Count <= MAX_SOUND; Count ++ )
  {
   // Если звук загружен в память - освобождаем ее.
   if( Sound.Sound_in_memory[ Count ] != NULLHANDLE ) DeleteSound( Sound.Sound_in_memory[ Count ] );

   // Сбрасываем для файла свойство "Только для чтения".
   FILESTATUS3 File_status = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
   DosSetPathInfo( Sound.Sound_file[ Count ], FIL_STANDARD, &File_status, sizeof( File_status ), DSPI_WRTTHRU );

   // Удаляем файл.
   DosForceDelete( Sound.Sound_file[ Count ] );
  }

 // Возврат.
 return;
}

// ─── Подготавливает в памяти изображение, которое используется в окне ───

// Изображение повторяется четыре раза - так, чтобы его можно было нарисовать, начиная
// с любой точки. Это надо для того, чтобы восстанавливать его при движении предметов.
VOID CreateBackgroundTile( VOID )
{
 // Задаем размеры изображения. Это должен быть размер картинки в окне + расстояние,
 // на которое предмет может выходить за него, то есть размер картинки для предмета.
 INT Width = Background.Bitmap_width + SPRITE_WIDTH;
 INT Height = Background.Bitmap_height + SPRITE_HEIGHT;

 // Узнаем вид устройства.
 LONG Format[ 2 ]; GpiQueryDeviceBitmapFormats( Memory_space, 2, Format );

 // Данные для того, чтобы создать изображение.
 BITMAPINFOHEADER2 Header;
 Header.cbFix = (ULONG) sizeof( BITMAPINFOHEADER2 ); Header.cx = Width; Header.cy = Height;
 Header.cPlanes = (SHORT) Format[ 0 ]; Header.cBitCount = (SHORT) Format[ 1 ]; Header.ulCompression = BCA_UNCOMP;
 Header.cbImage = ( ( ( Width * ( 1 << Header.cPlanes ) * ( 1 << Header.cBitCount ) ) + 31 ) / 32 ) * Height;
 Header.cxResolution = 70; Header.cyResolution = 70; Header.cclrUsed = 2; Header.cclrImportant = 0;
 Header.usUnits = BRU_METRIC; Header.usReserved = 0; Header.usRecording = BRA_BOTTOMUP; Header.usRendering = BRH_NOTHALFTONED;
 Header.cSize1 = 0; Header.cSize2 = 0; Header.ulColorEncoding = BCE_RGB; Header.ulIdentifier = 0;

 // Заголовок изображения - остается в памяти.
 PBITMAPINFO2 Information = NULLHANDLE;
 DosAllocMem( (PPVOID) &Information, sizeof( BITMAPINFO2 ) + ( sizeof( RGB2 ) * ( 1 << Header.cPlanes ) * ( 1 << Header.cBitCount ) ), PAG_COMMIT | PAG_READ | PAG_WRITE );
 Information->cbFix = Header.cbFix; Information->cx = Header.cx; Information->cy = Header.cy;
 Information->cPlanes = Header.cPlanes; Information->cBitCount = Header.cBitCount; Information->ulCompression = BCA_UNCOMP;
 Information->cbImage = ( ( Width + 31 ) / 32 ) * Height; Information->cxResolution = 70; Information->cyResolution = 70;
 Information->cclrUsed = 2; Information->cclrImportant = 0; Information->usUnits = BRU_METRIC;
 Information->usReserved = 0; Information->usRecording = BRA_BOTTOMUP; Information->usRendering = BRH_NOTHALFTONED;
 Information->cSize1 = 0; Information->cSize2 = 0; Information->ulColorEncoding = BCE_RGB; Information->ulIdentifier = 0;

 // Создаем изображение.
 Background.Tile = GpiCreateBitmap( Memory_space, &Header, FALSE, NULL, Information );

 // Заполняем его так же, как должно быть заполнено окно.
 if( Background.Bitmap != NULLHANDLE )
  {
   GpiSetBitmap( Memory_space, Background.Tile );
   for( INT X_Count = 0; X_Count <= Width; X_Count += Background.Bitmap_width )
    for( INT Y_Count = 0; Y_Count <= Height; Y_Count += Background.Bitmap_height )
     {
      // Рисуем картинку.
      POINTL Point = { X_Count, Y_Count };
      WinDrawBitmap( Memory_space, Background.Bitmap, NULL, &Point, 0, 0, DBM_NORMAL );
     }
  }

 // Возврат.
 return;
}

// ─── Рисует изображение предмета ───

// Presentation_space - пространство отображения, X и Y - точка, Sprite - предмет.
VOID DrawSprite( HPS Presentation_space, INT X, INT Y, SPRITE Sprite )
{
 // Если это пустой квадрат - возврат.
 if( Sprite.Image == NULLHANDLE ) return;

 // Запоминаем цвета в пространстве отображения.
 COLOR Old_color = GpiQueryColor( Presentation_space );
 COLOR Old_background_color = GpiQueryBackColor( Presentation_space );

 // Задаем новые цвета.
 GpiSetColor( Presentation_space, CLR_WHITE );
 GpiSetBackColor( Presentation_space, CLR_BLACK );

 // Задаем расположение картинки.
 POINTL Points[ 4 ] = { X, Y, X + SPRITE_WIDTH, Y + SPRITE_HEIGHT, 0, 0, SPRITE_WIDTH, SPRITE_HEIGHT };

 // Рисуем основу.
 GpiSetBitmap( Memory_space, Sprite.Base );
 GpiBitBlt( Presentation_space, Memory_space, 4, Points, ROP_SRCAND, BBO_IGNORE );

 // Рисуем картинку.
 GpiSetBitmap( Memory_space, Sprite.Image );
 GpiBitBlt( Presentation_space, Memory_space, 4, Points, ROP_SRCPAINT, BBO_IGNORE );

 // Восстанавливаем цвета в пространстве отображения.
 GpiSetColor( Presentation_space, Old_color );
 GpiSetBackColor( Presentation_space, Old_background_color );

 // Возврат.
 return;
}

// ─── Скрывает предмет, восстанавливая часть картинки в окне ───

// Presentation_space - пространство отображения, X и Y - точка.
VOID HideSprite( HPS Presentation_space, INT X, INT Y )
{
 // Узнаем, сколько изображений картинки в окне надо нарисовать до точки ( X, Y ).
 INT X_Bitmaps = X / Background.Bitmap_width;
 INT Y_Bitmaps = Y / Background.Bitmap_height;          //       │
 // Узнаем точку, в которой они заканчиваются.          //     **│***
 INT X_Node = X_Bitmaps * Background.Bitmap_width;      //     **│***
 INT Y_Node = Y_Bitmaps * Background.Bitmap_height;     // ────*─┼─**───
 // Вычисляем расстояние от этой точки до ( X, Y ).     //     **│***
 INT X_Distance = X - X_Node;                           //       │
 INT Y_Distance = Y - Y_Node;                           // // // // // // // //

 // Рисуем часть картинки, которая была подготовлена в памяти.
 POINTL Point = { X, Y };
 RECTL Part_of_bitmap = { X_Distance, Y_Distance, X_Distance + SPRITE_WIDTH, Y_Distance + SPRITE_HEIGHT };
 WinDrawBitmap( Presentation_space, Background.Tile, &Part_of_bitmap, &Point, 0, 0, DBM_NORMAL );

 // Возврат.
 return;
}

// ─── Принимает сообщения, которые приходят в окно приложения ───

// OS/2 вызывает WinProc всякий раз, когда для окна есть сообщение.
// Client_window определяет окно, Message - сообщение, *_parameter -
// данные, которые передаются вместе с сообщением.
MRESULT EXPENTRY WinProc( HWND Client_window, ULONG Message, MPARAM First_parameter, MPARAM Second_parameter )
{
 // Сообщение о том, что окно должно быть перерисовано.
 if( Message == WM_PAINT )
  {
   // Работаем в пространстве отображения окна.
   RECTL Rectangle = { 0, 0, 0, 0 };
   HPS Presentation_space = WinBeginPaint( Client_window, NULLHANDLE, &Rectangle );
   // Закрашиваем окно черным цветом.
   WinFillRect( Presentation_space, &Rectangle, CLR_BLACK );
   // Завершаем работу в пространстве отображения окна.
   WinEndPaint( Presentation_space );

   // Если предыдущее сообщение было принято:
   if( Thread_responds.Draw_message_is_received )
    {
     // Сбрасываем переменную.
     Thread_responds.Draw_message_is_received = 0;

     // Передаем сообщение в поток. Переменная First_parameter определяет окно приложения.
     WinPostQueueMsg( Thread_message_queue, WM_DRAW, (MPARAM) Client_window, 0 );
    }

   // Возврат.
   return 0;
  }

 // Сообщение от счетчика времени.
 if( Message == WM_TIMER )
  {
   // Если предыдущее сообщение было принято:
   if( Thread_responds.Timer_message_is_received )
    {
     // Сбрасываем переменную.
     Thread_responds.Timer_message_is_received = 0;

     // Передаем сообщение в поток. First_parameter - окно приложения, Second_parameter - счетчик времени.
     WinPostQueueMsg( Thread_message_queue, WM_TIMER, (MPARAM) Client_window, First_parameter );
    }

   // Возврат.
   return 0;
  }

 // Нажатие клавиши.
 if( Message == WM_CHAR )
  {
   // Смотрим, какая клавиша нажата.
   SHORT Key = CHARMSG( &Message ) -> vkey;

   // Если нажата 'F1' - вызов руководства.
   if( Key == VK_F1 ) WinPostQueueMsg( Thread_message_queue, WM_GUIDE, 0, 0 );

   // Остальные клавиши.
   if( !( SHORT1FROMMP( First_parameter ) & KC_KEYUP ) )
    {
     // Если показана заставка - переходим в игру.
     if( Settings.Game_mode == 0 )
      if( Key != VK_F1 ) if( Key != VK_ESC ) if( Key != VK_INSERT ) if( Key != VK_PAUSE )
       {
        // Переходим в игру.
        Settings.Game_mode = 1;

        // Звук - игра.
        WinPostMsg( Client_window, WM_PLAYSOUND, (MPARAM) SOUND_GAME, 0 );

        // Запускаем счетчик времени.
        WinStartTimer( Application, Client_window, 1, TIMER_RATE );

        // Окно должно быть перерисовано. Передаем сообщение в поток.
        Thread_responds.Draw_message_is_received = 0;
        WinPostQueueMsg( Thread_message_queue, WM_DRAW, (MPARAM) Client_window, 0 );
       }

     // Стрелки, пробел и Tab - передвижение предметов.
     if( Settings.Game_mode == 2 ) if( Key == VK_LEFT )
      WinPostQueueMsg( Thread_message_queue, WM_LEFT, (MPARAM) Client_window, 0 );

     if( Settings.Game_mode == 2 ) if( Key == VK_RIGHT )
      WinPostQueueMsg( Thread_message_queue, WM_RIGHT, (MPARAM) Client_window, 0 );

     if( Settings.Game_mode == 2 ) if( Key == VK_DOWN || Key == VK_SPACE )
      WinPostQueueMsg( Thread_message_queue, WM_DOWN, (MPARAM) Client_window, 0 );

     if( Settings.Game_mode == 2 ) if( Key == VK_TAB )
      WinPostQueueMsg( Thread_message_queue, WM_BOUNCE, (MPARAM) Client_window, 0 );

     // Pause - остановка или продолжение игры.
     if( Settings.Game_mode == 1 || Settings.Game_mode == 2 ) if( Key == VK_PAUSE )
      {
       // Остановка или продолжение игры.
       if( Settings.Game_is_paused ) Settings.Game_is_paused = 0;
       else Settings.Game_is_paused = 1;

       // Звук - остановка игры.
       WinPostMsg( Client_window, WM_PLAYSOUND, (MPARAM) SOUND_PAUSE, 0 );
      }

     // Если нажата Insert - показываем или скрываем сетку.
     if( Key == VK_INSERT )
      {
       // Показываем или скрываем сетку.
       if( Settings.Grid_is_visible ) Settings.Grid_is_visible = 0;
       else Settings.Grid_is_visible = 1;

       // Звук - как при нажатии кнопки мыши.
       WinPostMsg( Client_window, WM_PLAYSOUND, (MPARAM) SOUND_MOUSE, 0 );

       // Окно должно быть перерисовано. Передаем сообщение в поток.
       Thread_responds.Draw_message_is_received = 0;
       WinPostQueueMsg( Thread_message_queue, WM_DRAW, (MPARAM) Client_window, 0 );
      }

     // Если нажата 'Esc' - выход.
     if( Key == VK_ESC )
      {
       // Узнаем рамку окна приложения.
       HWND Frame_window = WinQueryWindow( Client_window, QW_PARENT );
       // Скрываем окно - при вызванном списке окон рабочий стол не перерисовывается.
       WinSetWindowPos( Frame_window, HWND_BOTTOM, 0, 0, 0, 0, SWP_SIZE | SWP_MOVE | SWP_ZORDER | SWP_HIDE | SWP_NOADJUST );
       // Закрываем окно.
       WinPostMsg( Client_window, WM_CLOSE, 0, 0 );
      }
    }

   // Возврат.
   return 0;
  }

 // Нажатие кнопки мыши:
 if( Message == WM_BUTTON1DOWN || Message == WM_BUTTON1DBLCLK ||
     Message == WM_BUTTON2DOWN || Message == WM_BUTTON2DBLCLK ||
     Message == WM_BUTTON3DOWN || Message == WM_BUTTON3DBLCLK )
  {
   // Если показана заставка - переходим в игру.
   if( Settings.Game_mode == 0 ) if( Message == WM_BUTTON1DOWN || Message == WM_BUTTON1DBLCLK )
    {
     // Переходим в игру.
     Settings.Game_mode = 1;

     // Звук - игра.
     WinPostMsg( Client_window, WM_PLAYSOUND, (MPARAM) SOUND_GAME, 0 );

     // Запускаем счетчик времени.
     WinStartTimer( Application, Client_window, 1, TIMER_RATE );

     // Окно должно быть перерисовано. Передаем сообщение в поток.
     Thread_responds.Draw_message_is_received = 0;
     WinPostQueueMsg( Thread_message_queue, WM_DRAW, (MPARAM) Client_window, 0 );
    }

   // Если во время игры нажата левая кнопка мыши - передвигаем предметы.
   if( Settings.Game_mode == 2 ) if( Message == WM_BUTTON1DOWN || Message == WM_BUTTON1DBLCLK )
    {
     // Посылаем сообщение в поток, *parameter - точка, в которой расположен указатель мыши.
     POINTS Point; Point.x = MOUSEMSG( &Message ) -> x; Point.y = MOUSEMSG( &Message ) -> y;
     WinPostQueueMsg( Thread_message_queue, WM_MOUSE, (MPARAM) Point.x, (MPARAM) Point.y );
    }

   // Если нажата правая кнопка мыши - показываем или скрываем сетку.
   if( Message == WM_BUTTON2DOWN || Message == WM_BUTTON2DBLCLK ||
       Message == WM_BUTTON3DOWN || Message == WM_BUTTON3DBLCLK )
    {
     // Показываем или скрываем сетку.
     if( Settings.Grid_is_visible ) Settings.Grid_is_visible = 0;
     else Settings.Grid_is_visible = 1;

     // Звук - нажатие кнопки мыши.
     WinPostMsg( Client_window, WM_PLAYSOUND, (MPARAM) SOUND_MOUSE, 0 );

     // Окно должно быть перерисовано. Передаем сообщение в поток.
     Thread_responds.Draw_message_is_received = 0;
     WinPostQueueMsg( Thread_message_queue, WM_DRAW, (MPARAM) Client_window, 0 );
    }

   // Возможно, окно должно стать выбранным. Передаем сообщение OS/2 на обработку.
   WinDefWindowProc( Client_window, Message, First_parameter, Second_parameter );

   // Возврат.
   return 0;
  }

 // Выбор строки меню.
 if( Message == WM_COMMAND )
  {
   // Смотрим, какая строка выбрана.
   SHORT Item = LOUSHORT( First_parameter );

   // Установка звука.
   if( Item == SOUND_MENU_ITEM )
    {
     // Меняем настройки.
     if( Settings.Sound_is_enabled )
      {
       // Звук - как при нажатии кнопки мыши.
       WinSendMsg( Client_window, WM_PLAYSOUND, (MPARAM) SOUND_MOUSE, 0 );

       // Отключаем звук.
       Settings.Sound_is_enabled = 0;
      }
     else
      {
       // Включаем звук.
       Settings.Sound_is_enabled = 1;
      }

     // Узнаем рамку окна приложения.
     HWND Frame_window = WinQueryWindow( Client_window, QW_PARENT );
     // Отмечаем строки меню или снимаем отметки.
     CheckMenuItems( Frame_window );
    }

   // Установка сложности.
   if( Item == DIFF_MENU_ITEM )
    {
     // Меняем настройки.
     if( Settings.Game_is_difficult ) Settings.Game_is_difficult = 0;
     else Settings.Game_is_difficult = 1;

     // Узнаем рамку окна приложения.
     HWND Frame_window = WinQueryWindow( Client_window, QW_PARENT );
     // Отмечаем строки меню или снимаем отметки.
     CheckMenuItems( Frame_window );
    }

   // Вызов руководства.
   if( Item == HELP_MENU_ITEM ) WinPostQueueMsg( Thread_message_queue, WM_GUIDE, 0, 0 );

   // Звук - как при нажатии кнопки мыши.
   WinPostMsg( Client_window, WM_PLAYSOUND, (MPARAM) SOUND_MOUSE, 0 );

   // Возврат.
   return 0;
  }

 // Воспроизведение звука. First_parameter определяет звук, который надо воспроизвести.
 if( Message == WM_PLAYSOUND )
  {
   // Если звук не воспроизводится в это время и он включен - воспроизводим его.
   if( !Sound.Sound_is_playing ) if( Settings.Sound_is_enabled )
    {
     // Загружаем звук в память и запоминаем его.
     INT Playing_sound = (INT) First_parameter;
     Sound.Sound_in_memory[ Playing_sound ] = LoadSound( Sound.Sound_file[ Playing_sound ] );

     // Воспроизводим звук.
     INT Success = PlaySound( Sound.Sound_in_memory[ Playing_sound ], Client_window );

     // Если звук воспроизводится - запоминаем это.
     if( Success ) Sound.Sound_is_playing = 1;
    }

   // Возврат.
   return 0;
  }

 // Сообщение о том, что звук воспроизведен. Second_parameter определяет звук.
 if( Message == MM_MCINOTIFY )
  {
   // Освобождаем память.
   HSOUND Played_sound = SHORT1FROMMP( Second_parameter );
   DeleteSound( Played_sound );

   // Запоминаем, что звук больше не занимает память.
   for( INT Count = 0; Count <= MAX_SOUND; Count ++ )
    if( Sound.Sound_in_memory[ Count ] == Played_sound )
     Sound.Sound_in_memory[ Count ] = NULLHANDLE;

   // Запоминаем, что звук больше не воспроизводится.
   Sound.Sound_is_playing = 0;

   // Возврат.
   return 0;
  }

 // Отрисовка окна - возвращаем "Да".
 if( Message == WM_ERASEBACKGROUND ) return (MRESULT) 1;

 // Другое сообщение - не обрабатывается.
 return WinDefWindowProc( Client_window, Message, First_parameter, Second_parameter );
}

// ─── Поток для рисования ───

// Поток работает как отдельное приложение и имеет отдельную очередь сообщений.
// В поток передаются сообщения, которые WinProc() получает для окна приложения.
// Поток завершается при завершении работы приложения.
VOID ThreadProc( VOID )
{
 // Определяем поток в системе.
 HAB Thread = WinInitialize( 0 );
 // Если это сделать не удалось - выход.
 if( Thread == NULLHANDLE )
  {
   // При создании потока произошла ошибка.
   Thread_responds.Thread_is_created = -1;
   // Выход.
   return;
  }

 // Создаем очередь сообщений для потока.
 Thread_message_queue = WinCreateMsgQueue( Thread, 0 );
 // Если очередь создать не удалось - выход.
 if( Thread_message_queue == NULLHANDLE )
  {
   // Завершаем работу потока.
   WinTerminate( Thread );

   // При создании потока произошла ошибка.
   Thread_responds.Thread_is_created = -1;
   // Выход.
   return;
  }

 // Поток создан успешно.
 Thread_responds.Thread_is_created = 1;

 // Получение и обработка сообщений, приходящих в поток.
 QMSG Message = { 0, 0, 0, 0, 0, 0, 0 };
 while( 1 )
  {
   // Выбираем очередное сообщение.
   WinGetMsg( Thread, &Message, 0, 0, 0 );

   // Обрабатываем сообщение.
   MessageProcessing( Message.msg, (MPARAM) Message.mp1, (MPARAM) Message.mp2 );
  }
}

// ─── Обработчик сообщений, которые были переданы в поток ───

// Message определет сообщение, *parameter - дополнительные сведения.
VOID MessageProcessing( ULONG Message, MPARAM First_parameter, MPARAM Second_parameter )
{
 // Если приходит сообщение о перерисовке окна:
 if( Message == WM_DRAW )
  {
   // Сообщение было принято.
   Thread_responds.Draw_message_is_received = 1;

   // Рисуем все в окне.
   HWND Client_window = (HWND) First_parameter;
   DrawImageInWindow( Client_window );

   // Возврат.
   return;
  }

 // Если приходит сообщение от счетчика времени:
 if( Message == WM_TIMER )
  {
   // Сообщение было принято.
   Thread_responds.Timer_message_is_received = 1;

   // Если игра не остановлена - передвигаем предметы.
   if( !Settings.Game_is_paused )
    {
     HWND Client_window = (HWND) First_parameter;
     AnimateObjects( Client_window );
    }

   // Возврат.
   return;
  }

 // Если нажата стрелка:
 if( Message >= WM_LEFT ) if( Message <= WM_BOUNCE )
  {
   // Если игра не остановлена - передвигаем предметы.
   if( !Settings.Game_is_paused )
    {
     // Передвигаем предметы.
     HWND Client_window = (HWND) First_parameter;
     MoveObjects( Client_window, Message );
    }

   // Возврат.
   return;
  }

 // Если нажата кнопка мыши:
 if( Message == WM_MOUSE )
  {
   // Обрабатываем сообщение.
   if( !Settings.Game_is_paused )
    {
     INT X_Point = (INT) First_parameter; INT Y_Point = (INT) Second_parameter;
     TranslateMouseMessages( X_Point, Y_Point );
    }

   // Возврат.
   return;
  }

 // Вызов руководства:
 if( Message == WM_GUIDE )
  {
   // Вызываем руководство. Глава руководства зависит от страны.
   CHAR Parameters[ 255 ]; Parameters[ 0 ] = 0;
   if( System_metrics.Code_page == RUSSIAN )
    strcpy( &Parameters[ 1 ], "/C View.exe Mirrors.inf Зеркала" );
   else
    strcpy( &Parameters[ 1 ], "/C View.exe Mirrors.inf Mirrors" );

   CHAR Error_string[ 1 ]; RESULTCODES Return_codes;
   DosExecPgm( Error_string, sizeof( Error_string ), EXEC_ASYNC, Parameters, NULL, &Return_codes, "Cmd.exe" );

   // Возврат.
   return;
  }
}

// ─── Добавляет строки в меню окна ───

// Frame_window определяет рамку окна.
VOID AddMenu( HWND Frame_window )
{
 // Узнаем окно картинки в левом верхнем углу окна.
 HWND Picture_window = WinWindowFromID( Frame_window, FID_SYSMENU );

 // Если картинки нет - возврат.
 if( Picture_window == NULLHANDLE ) return;

 // Узнаем окно меню.
 SHORT Menu_ID = SHORT1FROMMP( WinSendMsg( Picture_window, MM_ITEMIDFROMPOSITION, MPFROMSHORT( 0 ), 0 ) );
 MENUITEM Menu_item = { 0, 0, 0, 0, 0, 0 }; WinSendMsg( Picture_window, MM_QUERYITEM, MPFROMSHORT( Menu_ID ), MPFROMP( &Menu_item ) );
 HWND Menu_window = Menu_item.hwndSubMenu;

 // Задаем разделительную линию.
 Menu_item.iPosition = MIT_END;
 Menu_item.afStyle = MIS_SEPARATOR;
 Menu_item.afAttribute = 0;
 Menu_item.id = -1;
 Menu_item.hwndSubMenu = NULLHANDLE;
 Menu_item.hItem = 0;

 // Добавляем разделительную линию в меню.
 WinSendMsg( Menu_window, MM_INSERTITEM, MPFROMP( &Menu_item ), 0 );

 // Добавляем строку - звук.
 Menu_item.afStyle = MIS_TEXT;
 Menu_item.id = SOUND_MENU_ITEM;
 WinSendMsg( Menu_window, MM_INSERTITEM, MPFROMP( &Menu_item ), Strings_in_window.Menu_strings[ 0 ] );

 // Добавляем строку - бросать сложные предметы.
 Menu_item.id = DIFF_MENU_ITEM;
 WinSendMsg( Menu_window, MM_INSERTITEM, MPFROMP( &Menu_item ), Strings_in_window.Menu_strings[ 1 ] );

 // Добавляем строку - справка.
 Menu_item.id = HELP_MENU_ITEM;
 WinSendMsg( Menu_window, MM_INSERTITEM, MPFROMP( &Menu_item ), Strings_in_window.Menu_strings[ 2 ] );

 // Ставим отметки в строки меню.
 CheckMenuItems( Frame_window );

 // Возврат.
 return;
}

// ─── Ставит отметки в строки меню или снимает их ───

// Frame_window определяет рамку окна.
VOID CheckMenuItems( HWND Frame_window )
{
 // Узнаем окно картинки в левом верхнем углу окна.
 HWND Picture_window = WinWindowFromID( Frame_window, FID_SYSMENU );

 // Если картинки нет - возврат.
 if( Picture_window == NULLHANDLE ) return;

 // Узнаем окно меню.
 SHORT Menu_ID = SHORT1FROMMP( WinSendMsg( Picture_window, MM_ITEMIDFROMPOSITION, MPFROMSHORT( 0 ), 0 ) );
 MENUITEM Menu_item = { 0, 0, 0, 0, 0, 0 }; WinSendMsg( Picture_window, MM_QUERYITEM, MPFROMSHORT( Menu_ID ), MPFROMP( &Menu_item ) );
 HWND Menu_window = Menu_item.hwndSubMenu;

 // Ставим отметку в строку "Звук".
 INT Attribute = 0; if( Settings.Sound_is_enabled ) Attribute = MIA_CHECKED;
 WinSendMsg( Menu_window, MM_SETITEMATTR, (MPARAM) SOUND_MENU_ITEM, MPFROM2SHORT( MIA_CHECKED, Attribute ) );

 // Ставим отметку в строку "Сложные предметы".
 Attribute = 0; if( Settings.Game_is_difficult ) Attribute = MIA_CHECKED;
 WinSendMsg( Menu_window, MM_SETITEMATTR, (MPARAM) DIFF_MENU_ITEM, MPFROM2SHORT( MIA_CHECKED, Attribute ) );

 // Возврат.
 return;
}

// ─── Делает зеркала пустыми ───

VOID CleanMirrors( VOID )
{
 // Делает зеркала пустыми.
 for( INT Count = 0; Count < Mirror_size.Width * Mirror_size.Height; Count ++ )
  {
   // Квадраты ничего не содержат.
   Left_mirror[ Count ].Object = 0; Right_mirror[ Count ].Object = 0;
  }

 // Возврат.
 return;
}

// Располагает на поверхности зеркал несколько предметов.

VOID ThrowObjects( VOID )
{
 // Действие выполняется два раза - для левого и правого зеркала.
 for( INT Mirrors_count = 1; Mirrors_count <= 2; Mirrors_count ++ )
  {
   // Выбираем зеркало.
   MIRROR_SQUARES* Mirror = NULL;
   if( Mirrors_count == 1 ) Mirror = Left_mirror; else Mirror = Right_mirror;

   // Заполняем одну десятую часть поверхности.
   for( INT Count = 0; Count < Mirror_size.Width * Mirror_size.Height / 10; Count ++ )
    {
     // Располагаем предметы. Первые восемь пропускаем, это пустота и ящики.
     INT Position = Rnd( Mirror_size.Width * Mirror_size.Height / 1.05 );
     Mirror[ Position ].Object = Rnd( MAX_SPRITE - MAX_BOX - 1 ) + MAX_BOX + 1;
     Mirror[ Position ].Y_Offset = 0;
     Mirror[ Position ].Action = THROWING;
    }
  }

 // Возврат.
 return;
}

// ─── Рисует все в окне ───

// Client_window определяет окно приложения.
VOID DrawImageInWindow( HWND Client_window )
{
 // Узнаем размеры окна приложения.
 RECTL Rectangle = { 0, 0, 0, 0 }; WinQueryWindowRect( Client_window, &Rectangle );

 // Работаем в пространстве отображения окна.
 HPS Presentation_space = WinGetPS( Client_window );

 // Если есть картинка - используем ее для заполнения окна.
 if( Background.Bitmap != NULLHANDLE )
  {
   // Заполняем окно.
   for( INT X_Count = 0; X_Count <= Rectangle.xRight + Background.Bitmap_width; X_Count += Background.Bitmap_width )
    for( INT Y_Count = 0; Y_Count <= Rectangle.yTop + Background.Bitmap_height; Y_Count += Background.Bitmap_height )
     {
      // Рисуем картинку.
      POINTL Point = { X_Count, Y_Count };
      WinDrawBitmap( Presentation_space, Background.Bitmap, NULL, &Point, 0, 0, DBM_NORMAL );
     }
  }
 // А если картинки нет - закрашиваем окно черным цветом.
 else WinFillRect( Presentation_space, &Rectangle, CLR_BLACK );

 // Вычисляем точки, по которым надо нарисовать зеркала.
 CalculateMirrorPoints( Rectangle );

 // Проводим линии.
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

 // Рисуем кубики и сетку. Действие выполняется два раза - для левого и правого зеркала.
 for( INT Mirrors_count = 1; Mirrors_count <= 2; Mirrors_count ++ )
  {
   // Выбираем зеркало и точки.
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

   // Рисуем кубики и сетку.
   INT Position = 0;
   for( INT Y_Count = Y_First_point + GRID_WIDTH; Y_Count <= Y_Last_point - SPRITE_HEIGHT; Y_Count += SPRITE_HEIGHT + GRID_WIDTH )
    for( INT X_Count = X_First_point + GRID_WIDTH; X_Count <= X_Last_point - GRID_WIDTH - SPRITE_WIDTH; X_Count += SPRITE_WIDTH + GRID_WIDTH )
     {
      // Рисуем предмет.
      DrawSprite( Presentation_space, X_Count, Y_Count + Mirror[ Position ].Y_Offset, Sprite[ Mirror[ Position ].Object ] );

      // Рисуем сетку.
      if( Settings.Grid_is_visible ) if( X_Count - 1 != Mirror_points.Hx ) if( Y_Count - 1 != Mirror_points.Hy )
       { POINTL Point = { X_Count - 1, Y_Count - 1 }; GpiSetPel( Presentation_space, &Point ); }

      // Переходим к следующему предмету.
      Position ++;
     }
  }

 // Если надо показать заставку:
 if( Settings.Game_mode == 0 )
  {
   // Узнаем рамку окна приложения.
   HWND Frame_window = WinQueryWindow( Client_window, QW_PARENT );
   // Узнаем заголовок окна приложения.
   HWND Titlebar_window = WinWindowFromID( Frame_window, FID_TITLEBAR );

   // Работаем в пространстве отображения заголовка окна.
   HPS Titlebar_space = WinGetPS( Titlebar_window );

   // Узнаем, какой шрифт сейчас используется в заголовке окна.
   FONTMETRICS Titlebar_font_metrics;
   GpiQueryFontMetrics( Titlebar_space, sizeof( FONTMETRICS ), &Titlebar_font_metrics );

   // Завершаем работу в пространстве отображения заголовка.
   WinReleasePS( Titlebar_space );

   // Шрифт для рисования будет таким же, как и в заголовке окна.
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

   // Создаем шрифт.
   GpiCreateLogFont( Presentation_space, NULL, 1, &Font_attributes );
   // Выбираем шрифт в пространстве отображения окна.
   GpiSetCharSet( Presentation_space, 1 );

   // Узнаем высоту строки для шрифта, который используется в окне.
   RECTL Letter_rectangle = { 0, 0, 1, 1 };
   WinDrawText( Presentation_space, -1, "Mirrors game. (C) Sergey Posokhov, Moscow. E-mail: abc@posokhov.msk.ru.", &Letter_rectangle, 0, 0, DT_QUERYEXTENT );
   INT Letter_height = Letter_rectangle.yTop - Letter_rectangle.yBottom;

   // Задаем расположение строк текста.
   Rectangle.xLeft += 25; Rectangle.xRight -= 25;
   Rectangle.yTop -= Rectangle.yTop / 5; Rectangle.yBottom = Rectangle.yTop - Letter_height;

   // Название игры.
   WinDrawText( Presentation_space, -1, Strings_in_window.Title, &Rectangle, 0, 0, DT_TEXTATTRS | DT_CENTER | DT_VCENTER );

   // Пропускаем несколько строк.
   Rectangle.yTop -= Letter_height * 3; Rectangle.yBottom -= Letter_height * 3;

   // Описание и правила игры.
   GpiSetColor( Presentation_space, CLR_WHITE );
   for( INT Count = 0; Count <= 5; Count ++ )
    {
     WinDrawText( Presentation_space, -1, Strings_in_window.Description[ Count ], &Rectangle, 0, 0, DT_TEXTATTRS | DT_CENTER | DT_VCENTER | DT_WORDBREAK );
     Rectangle.yTop -= Letter_height * 1.25; Rectangle.yBottom -= Letter_height * 1.25;
    }

   // Пропускаем несколько строк.
   Rectangle.yTop -= Letter_height * 2; Rectangle.yBottom -= Letter_height * 2;

   // Разработчик.
   GpiSetColor( Presentation_space, CLR_CYAN );
   WinDrawText( Presentation_space, -1, Strings_in_window.Developer, &Rectangle, 0, 0, DT_TEXTATTRS | DT_CENTER | DT_VCENTER | DT_WORDBREAK );
  }

 // Завершаем работу в пространстве отображения окна.
 WinReleasePS( Presentation_space );

 // Возврат.
 return;
}

// ─── Вычисляет размеры зеркал ───

// Window_rectangle определяет размеры окна.
VOID CalculateMirrorSize( RECTL Window_rectangle )
{
 // Устанавливаем размеры зеркал.
 Mirror_size.Width = ( Window_rectangle.xRight - BORDER_WIDTH * 3 - Window_rectangle.xRight / 2 ) / ( SPRITE_WIDTH + GRID_WIDTH ) / 2;
 Mirror_size.Height = ( Window_rectangle.yTop - BORDER_WIDTH - Window_rectangle.yTop / 4 ) / ( SPRITE_HEIGHT + GRID_WIDTH );

 // Размер зеркала должен быть >= 3x2.
 if( Mirror_size.Width <= 3 ) Mirror_size.Width = 3;
 if( Mirror_size.Height <= 2 ) Mirror_size.Height = 2;

 // Возврат.
 return;
}

// ─── Вычисляет точки, по которым надо нарисовать зеркала ───

// Window_rectangle определяет размеры окна.
VOID CalculateMirrorPoints( RECTL Window_rectangle )
{
 // Вычисляем размеры зеркала. К ширине прибавляется значение GRID_WIDTH, потому что
 // к ряду справа надо добавить точки, соответствующие ширине сетки.
 INT Mirror_width = Mirror_size.Width * ( SPRITE_WIDTH + GRID_WIDTH ) + GRID_WIDTH;
 INT Mirror_height = Mirror_size.Height * ( SPRITE_HEIGHT + GRID_WIDTH );

 // Устанавливаем точки.
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

 // Возврат.
 return;
}

// ─── Обрабатывает сообщение от счетчика времени, передвигает предметы ───

// Client_window определяет окно приложения, Timer - счетчик времени.
VOID AnimateObjects( HWND Client_window )
{
 // Указывает, что летящие предметы должны остановиться.
 INT Flying_is_done = 0;
 // Указывает, что ящики надо привести в движение.
 INT Objects_must_be_shaked = 0;
 // Указывает, что столбик ящиков надо остановить.
 INT Column_must_be_stopped = 0; INT Stopping_position = 0;

 // Работаем в пространстве отображения окна.
 HPS Presentation_space = WinGetPS( Client_window );

 // Действие выполняется два раза - для левого и правого зеркала.
 for( INT Mirrors_count = 1; Mirrors_count <= 2; Mirrors_count ++ )
  {
   // Выбираем зеркало и точки.
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

   // Передвигаем предметы в выбранном зеркале.
   INT Position = 0;
   for( INT Y_Count = Y_First_point + GRID_WIDTH; Y_Count <= Y_Last_point - SPRITE_HEIGHT; Y_Count += SPRITE_HEIGHT + GRID_WIDTH )
    for( INT X_Count = X_First_point + GRID_WIDTH; X_Count <= X_Last_point - GRID_WIDTH - SPRITE_WIDTH; X_Count += SPRITE_WIDTH + GRID_WIDTH )
     {
      // Если движение не было завершено - передвигаем предметы.
      if( !Flying_is_done )
       {
        // Если для предмета задано действие "движение" - передвигаем предмет.
        if( Mirror[ Position ].Object != 0 )
         if( Mirror[ Position ].Action == FLYING || Mirror[ Position ].Action == DARTING || Mirror[ Position ].Action == THROWING )
          {
           // Запоминаем точку, в которой расположен предмет.
           INT Previous_Y = Y_Count + Mirror[ Position ].Y_Offset;

           // Вычисляем новое расположение предмета.
           INT Next_position = Position - Mirror_size.Width;

           // Если внизу расположен ящик, который должен исчезнуть - не передвигаем предмет.
           if( Next_position >= 0 )
            if( Mirror[ Next_position ].Object != 0 )
             if( Mirror[ Next_position ].Action == DISAPPEARING ) continue;

           // Если внизу есть что-то, что движется медленнее предмета - он замедляет движение.
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

           // Задаем новое расположение предмета.
           INT Step = 0;
           if( Mirror[ Position ].Action == FLYING ) Step = Rate.Flying_step;
           if( Mirror[ Position ].Action == DARTING ) Step = Rate.Darting_step;
           if( Mirror[ Position ].Action == THROWING ) Step = Rate.Throwing_step;
           Mirror[ Position ].Y_Offset -= Step;

           // Если предмет достиг дна стакана - он останавливается.
           if( Next_position < 0 ) if( Mirror[ Position ].Y_Offset <= 0 )
            {
             // Если идет игра - движение должно быть завершено.
             if( Settings.Game_mode == 2 )
              if( Mirror[ Position ].Action == FLYING || Mirror[ Position ].Action == DARTING ) Flying_is_done = 1;

             // Если это движется ящик, заполняя ряды - останавливаем его сразу.
             if( Mirror[ Position ].Object <= MAX_BOX )
              {
               // Останавливаем ящик.
               Mirror[ Position ].Y_Offset = 0;
               Mirror[ Position ].Action = 0;
               // Другие предметы тоже должны быть остановлены.
               Column_must_be_stopped = 1; Stopping_position = Position;
              }
             // Иначе - останавливаем предмет, и он постепенно превращается в ящик.
             else
              {
               // Останавливаем предмет.
               Mirror[ Position ].Y_Offset = 0;
               Mirror[ Position ].Action = PACKING;
               Mirror[ Position ].Action_count = 0;
               Mirror[ Position ].Change_point = Rnd( STOP_POINT - STOP_POINT / 2 ) + STOP_POINT / 2;
              }
            }

           // Если предмет пролетел квадрат, и в следующем квадрате что-то есть:
           if( Next_position >= 0 )
            if( Mirror[ Position ].Y_Offset <= 0 ) if( Mirror[ Next_position ].Object != 0 )
             {
              // Если внизу есть что-то неподвижное - предмет останавливается.
              if( Mirror[ Next_position ].Action == 0 || Mirror[ Next_position ].Action == PACKING )
               {
                // Если идет игра - движение должно быть завершено.
                if( Settings.Game_mode == 2 )
                 if( Mirror[ Position ].Action == FLYING || Mirror[ Position ].Action == DARTING ) Flying_is_done = 1;

                // Если это движется ящик, заполняя ряды - останавливаем его сразу.
                if( Mirror[ Position ].Object <= MAX_BOX )
                 {
                  // Останавливаем ящик.
                  Mirror[ Position ].Y_Offset = 0;
                  Mirror[ Position ].Action = 0;
                  // Другие предметы в этом столбике тоже должны быть остановлены.
                  Column_must_be_stopped = 1; Stopping_position = Position;
                 }
                // Иначе - останавливаем предмет, и он постепенно превращается в ящик.
                else
                 {
                  // Останавливаем предмет.
                  Mirror[ Position ].Y_Offset = 0;
                  Mirror[ Position ].Action = PACKING;
                  Mirror[ Position ].Action_count = 0;
                  Mirror[ Position ].Change_point = Rnd( STOP_POINT - STOP_POINT / 2 ) + STOP_POINT / 2;
                 }
               }
             }

           // Скрываем предмет в точке, в которой он был расположен.
           HideSprite( Presentation_space, X_Count, Previous_Y );
           // Рисуем предмет в новой точке.
           DrawSprite( Presentation_space, X_Count, Y_Count + Mirror[ Position ].Y_Offset, Sprite[ Mirror[ Position ].Object ] );

           // Если предмет пролетел половину квадрата - переносим его в другой квадрат.
           if( Next_position >= 0 )
            if( Mirror[ Position ].Y_Offset <= ( -1 ) * SPRITE_HEIGHT / 2 )
             {
              // Переносим туда предмет.
              Mirror[ Next_position ].Object = Mirror[ Position ].Object;
              Mirror[ Next_position ].Y_Offset = Mirror[ Position ].Y_Offset + SPRITE_HEIGHT + 1;
              Mirror[ Next_position ].Action = Mirror[ Position ].Action;

              // Выбранный квадрат становится пустым.
              Mirror[ Position ].Object = 0; Mirror[ Position ].Action = 0;
             }
          }
       }

      // Если предмет был остановлен - он превращается в ящик.
      if( Mirror[ Position ].Object != 0 ) if( Mirror[ Position ].Action == PACKING )
       {
        // Должно пройти некоторое время, для этого есть счетчик.
        Mirror[ Position ].Action_count ++;
        // Сначала предмет превращается в ящик.
        if( Mirror[ Position ].Action_count == Mirror[ Position ].Change_point )
         {
          // Предмет превращается в ящик.
          Mirror[ Position ].Object = Rnd( MAX_BOX - 1 ) + 1;
          // Скрываем предмет.
          HideSprite( Presentation_space, X_Count, Y_Count );
          // Рисуем предмет заново.
          DrawSprite( Presentation_space, X_Count, Y_Count, Sprite[ Mirror[ Position ].Object ] );
         }
        // После этого ящик становится неподвижным.
        if( Mirror[ Position ].Action_count == STOP_POINT )
         Mirror[ Position ].Action = 0;
       }

      // Если предмет должен исчезнуть:
      if( Mirror[ Position ].Object != 0 ) if( Mirror[ Position ].Action == DISAPPEARING )
       {
        // Должно пройти некоторое время, для этого есть счетчик.
        Mirror[ Position ].Action_count ++;
        // После этого предмет исчезает.
        if( Mirror[ Position ].Action_count == Mirror[ Position ].Change_point )
         {
          // Предмет исчезает.
          Mirror[ Position ].Object = 0;
          Mirror[ Position ].Action = 0;
          // Скрываем предмет.
          HideSprite( Presentation_space, X_Count, Y_Count );
          // Если идет игра - ящики надо привести в движение.
          if( Settings.Game_mode == 2 ) Objects_must_be_shaked = 1;
         }
       }

      // Если надо остановить столбик ящиков - делаем это.
      if( Column_must_be_stopped )
       {
        // Останавливаем столбик ящиков.
        for( INT Count = Stopping_position; Count < Mirror_size.Width * Mirror_size.Height; Count += Mirror_size.Width )
         if( Mirror[ Count ].Object != 0 ) if( Mirror[ Count ].Object <= MAX_BOX )
          if( Mirror[ Count ].Action == THROWING )
           {
            // Скрываем предмет в точке, в которой он был расположен.
            INT V_Count = Count / Mirror_size.Width;
            INT H_Count = Count - V_Count * Mirror_size.Width;
            INT X = X_First_point + GRID_WIDTH + ( SPRITE_WIDTH + GRID_WIDTH ) * H_Count;
            INT Y = Y_First_point + GRID_WIDTH + ( SPRITE_HEIGHT + GRID_WIDTH ) * V_Count + Mirror[ Count ].Y_Offset;
            HideSprite( Presentation_space, X, Y );

            // Останавливаем предмет.
            Mirror[ Count ].Action = 0; Mirror[ Count ].Y_Offset = 0;

            // Рисуем предмет заново.
            Y = Y_First_point + GRID_WIDTH + ( SPRITE_HEIGHT + GRID_WIDTH ) * V_Count;
            DrawSprite( Presentation_space, X, Y, Sprite[ Mirror[ Count ].Object ] );
           }

        // Столбик ящиков остановлен.
        Column_must_be_stopped = 0;
       }

      // Переходим к следующему предмету.
      Position ++;
     }

    // Если движение было завершено - останавливаем все летящие предметы.
    if( Flying_is_done )
     {
      // Останавливаем летящие предметы в выбранном зеркале.
      INT Position = 0;
      for( INT Y_Count = Y_First_point + GRID_WIDTH; Y_Count <= Y_Last_point - SPRITE_HEIGHT; Y_Count += SPRITE_HEIGHT + GRID_WIDTH )
       for( INT X_Count = X_First_point + GRID_WIDTH; X_Count <= X_Last_point - GRID_WIDTH - SPRITE_WIDTH; X_Count += SPRITE_WIDTH + GRID_WIDTH )
        {
         // Если для предмета задано действие "полет" - останавливаем его.
         if( Mirror[ Position ].Object != 0 )
          if( Mirror[ Position ].Action == FLYING || Mirror[ Position ].Action == DARTING )
           {
            // Скрываем предмет в точке, в которой он был расположен.
            HideSprite( Presentation_space, X_Count, Y_Count + Mirror[ Position ].Y_Offset );

            // Останавливаем предмет.
            Mirror[ Position ].Y_Offset = 0;
            Mirror[ Position ].Action = PACKING;
            Mirror[ Position ].Action_count = 0;
            Mirror[ Position ].Change_point = Rnd( STOP_POINT - STOP_POINT / 2 ) + STOP_POINT / 2;

            // Рисуем предмет заново.
            DrawSprite( Presentation_space, X_Count, Y_Count, Sprite[ Mirror[ Position ].Object ] );
           }

         // Переходим к следующему предмету.
         Position ++;
        }
     }
  }

 // Завершаем работу в пространстве отображения окна.
 WinReleasePS( Presentation_space );

 // Если ящики надо привести в движение - делаем это.
 if( Objects_must_be_shaked )
  for( INT Count = 0; Count < Mirror_size.Width * Mirror_size.Height; Count ++ )
   {
    if( Left_mirror[ Count ].Object != 0 ) if( Left_mirror[ Count ].Action == 0 )
     Left_mirror[ Count ].Action = THROWING;
    if( Right_mirror[ Count ].Object != 0 ) if( Right_mirror[ Count ].Action == 0 )
     Right_mirror[ Count ].Action = THROWING;
   }

 // Если есть "повисшие" ящики - они должны продолжать падать.
 DetectHangedBoxes();

 // Проверяем, есть ли еще летящие или изменяющиеся предметы.
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

 // Если игра только начинается и падающих предметов нет - удаляем ряды, и если они не
 // были удалены, приводим все ящики в движение еще раз и переходим в игру.
 if( Settings.Game_mode == 1 )
  if( Objects_is_throwed ) if( Objects_is_packed ) if( Objects_is_disappeared )
   {
    // Удаляем ряды.
    INT Rows_was_deleted = DeleteRows( Client_window );

    // Если ряды не были удалены - переходим в игру.
    if( !Rows_was_deleted ) Settings.Game_mode = 2;
   }

 // Если идет игра - действуем по необходимости.
 if( Settings.Game_mode == 2 )
  {
   // Если все действия завершены - удаляем ряды и бросаем новые предметы.
   if( Objects_is_stopped ) if( Objects_is_throwed ) if( Objects_is_packed ) if( Objects_is_disappeared )
    {
     // Удаляем ряды.
     DeleteRows( Client_window );
     // Бросаем новые предметы и узнаем, можно ли продолжать игру.
     INT Game_can_be_continued = ThrowObjectSet( Client_window );

     // Если игра должна быть завершена - завершаем ее.
     if( !Game_can_be_continued )
      {
       // Все предметы должны исчезнуть.
       DeleteAllObjects();
       // Игра завершается.
       Settings.Game_mode = 3;

       // Звук - игра завершается.
       WinPostMsg( Client_window, WM_PLAYSOUND, (MPARAM) SOUND_OVER, 0 );

       // Возврат.
       return;
      }
    }

   // Если есть только летящие предметы - возможно, ряды надо снова удалить.
   if( Objects_is_throwed ) if( Objects_is_packed ) if( Objects_is_disappeared )
    DeleteRows( Client_window );
  }

 // Если игра завершается и все предметы исчезли - переходим в заставку.
 if( Settings.Game_mode == 3 ) if( Objects_is_disappeared )
  {
   // Переходим в заставку.
   Settings.Game_mode = 0;

   // Останавливаем счетчик времени.
   WinStopTimer( Application, Client_window, 1 );

   // Делаем зеркала пустыми и располагаем на поверхности несколько предметов.
   CleanMirrors(); ThrowObjects();

   // Окно должно быть перерисовано. Передаем сообщение в поток.
   Thread_responds.Draw_message_is_received = 0;
   WinPostQueueMsg( Thread_message_queue, WM_DRAW, (MPARAM) Client_window, 0 );
  }

 // Возврат.
 return;
}

// ─── Удаляет ряды ───

// Client_window определяет окно приложения.
// Возвращает 1 или 0 в зависимости от того, были удалены ряды, или нет.
INT DeleteRows( HWND Client_window )
{
 // Указывает, что ряды были удалены.
 INT Rows_was_deleted = 0;

 // Просматриваем зеркала.
 for( INT V_Count = 0; V_Count < Mirror_size.Height; V_Count ++ )
  {
   // Ряды должны быть не пустыми и зеркальными.
   INT Rows_is_empty = 1; INT Rows_is_alike = 0;

   // Проверяем ряды на заполненность.
   for( INT H_Count = 0; H_Count < Mirror_size.Width; H_Count ++ )
    if( Left_mirror[ V_Count * Mirror_size.Width + H_Count ].Object != 0 )
     if( Left_mirror[ V_Count * Mirror_size.Width + H_Count ].Object <= MAX_BOX )
      Rows_is_empty = 0;

   // Проверяем ряды на зеркальность.
   if( !Rows_is_empty )
    {
     // Просматриваем зеркала от края к середине.
     Rows_is_alike = 1;
     for( H_Count = 0; H_Count < Mirror_size.Width; H_Count ++ )
      {
       // В зеркале слева что-то есть:
       INT Left_object = 0;
        if( Left_mirror[ V_Count * Mirror_size.Width + H_Count ].Object != 0 )
         if( Left_mirror[ V_Count * Mirror_size.Width + H_Count ].Object <= MAX_BOX )
          if( Left_mirror[ V_Count * Mirror_size.Width + H_Count ].Action == 0 )
           Left_object = 1;
       // В зеркале справа что-то есть:
       INT Right_object = 0;
        if( Right_mirror[ ( V_Count * Mirror_size.Width + Mirror_size.Width - 1 ) - H_Count ].Object != 0 )
         if( Right_mirror[ ( V_Count * Mirror_size.Width + Mirror_size.Width - 1 ) - H_Count ].Object <= MAX_BOX )
          if( Right_mirror[ ( V_Count * Mirror_size.Width + Mirror_size.Width - 1 ) - H_Count ].Action == 0 )
           Right_object = 1;
       // Если совпадения нет - запоминаем это.
       if( Left_object != Right_object ) Rows_is_alike = 0;
      }
    }

   // Если ряды зеркальны:
   if( Rows_is_alike )
    for( H_Count = 0; H_Count < Mirror_size.Width; H_Count ++ )
     {
      // Ящики исчезают.
      INT Position = V_Count * Mirror_size.Width + H_Count;
      Left_mirror[ Position ].Action = DISAPPEARING;
      Left_mirror[ Position ].Action_count = 0;
      Left_mirror[ Position ].Change_point = Rnd( STOP_POINT ) + STOP_POINT / 2;

      Right_mirror[ Position ].Action = DISAPPEARING;
      Right_mirror[ Position ].Action_count = 0;
      Right_mirror[ Position ].Change_point = Rnd( STOP_POINT ) + STOP_POINT / 2;

      // Ряды были удалены.
      Rows_was_deleted = 1;
     }
  }

 // Звук - ряды были удалены.
 if( Rows_was_deleted ) WinPostMsg( Client_window, WM_PLAYSOUND, (MPARAM) SOUND_DELETE, 0 );

 // Возврат.
 return Rows_was_deleted;
}

// ─── Проверяет, есть ли в зеркалах "повисшие" ящики и бросает их ───

VOID DetectHangedBoxes( VOID )
{
 // Проверяем, есть ли в зеркале "повисшие" ящики.
 for( INT Mirrors_count = 1; Mirrors_count <= 2; Mirrors_count ++ )
  {
   // Выбираем зеркало.
   MIRROR_SQUARES* Mirror = NULL;
   if( Mirrors_count == 1 ) Mirror = Left_mirror; else Mirror = Right_mirror;

   for( INT V_Count = 0; V_Count < Mirror_size.Height; V_Count ++ )
    for( INT H_Count = 0; H_Count < Mirror_size.Width; H_Count ++ )
     {
      // Проверяем предмет.
      INT Position = V_Count * Mirror_size.Width + H_Count;

      // Смотрим, есть ли что-нибудь внизу.
      if( Position >= Mirror_size.Width )
       if( Mirror[ Position ].Object != 0 )
        if( Mirror[ Position ].Action == 0 || Mirror[ Position ].Action == PACKING )
         {
          // Делаем три проверки.
          INT Results[ 3 ] = { 0, 0, 0 };

          // Смотрим, есть ли что-нибудь внизу.
          if( Mirror[ Position - Mirror_size.Width ].Object == 0 ) Results[ 0 ] = 1;

          // Смотрим, есть ли что-нибудь внизу слева. Предмет может быть расположен около границы зеркала.
          if( H_Count == 0 ) Results[ 1 ] = 1;
          else if( Mirror[ Position - Mirror_size.Width - 1 ].Object == 0 ) Results[ 1 ] = 1;

          // Смотрим, есть ли что-нибудь внизу справа. Предмет может быть расположен около границы зеркала.
          if( H_Count == Mirror_size.Width - 1 ) Results[ 2 ] = 1;
          else if( Mirror[ Position - Mirror_size.Width + 1 ].Object == 0 ) Results[ 2 ] = 1;

          // Если все они дали ответ "под предметом ничего нет", то он падает.
          if( Results[ 0 ] + Results[ 1 ] + Results[ 2 ] == 3 )
           if( Mirror[ Position ].Action == PACKING ) Mirror[ Position ].Action = FLYING;
            else Mirror[ Position ].Action = THROWING;
         }
     }
  }

 // Возврат.
 return;
}

// ─── Располагает на поверхности зеркал набор предметов ───

// Client_window определяет окно приложения. Возвращает 1 или 0.
INT ThrowObjectSet( HWND Client_window )
{
 // Выбираем зеркало.
 MIRROR_SQUARES* Mirror = NULL;
 if( Rnd( 1 ) == 1 ) Mirror = Left_mirror; else Mirror = Right_mirror;

 // Выбираем новый предмет. Первые восемь пропускаем - это пустота и ящики.
 INT Position = 0; INT Object = Rnd( MAX_SPRITE - MAX_BOX - 1 ) + MAX_BOX + 1;
 // Можно бросить разные наборы предметов.
 INT Object_set = 0;
 if( Settings.Game_is_difficult ) Object_set = Rnd( 15 ); else Object_set = Rnd( 10 );
 switch( Object_set )
  {
   case 0:
    // Выбираем точку.
    Position = Mirror_size.Height * Mirror_size.Width - Rnd( Mirror_size.Width - 1 ) - 1;
    // Если в этой точке что-то есть - возврат.
    if( Mirror[ Position ].Object != 0 ) return 0;
    // Располагаем на поверхности зеркала набор предметов.
    Mirror[ Position ].Object = Object;                              //
    Mirror[ Position ].Y_Offset = 0;                                 //   X
    Mirror[ Position ].Action = FLYING;                              //
   break;

   case 1:
    // Выбираем точку.
    Position = Mirror_size.Height * Mirror_size.Width - Rnd( Mirror_size.Width - 2 ) - 2;
    // Если в точках, которые надо заполнить, что-то есть - возврат.
    if( Mirror[ Position ].Object != 0 ) return 0;
    if( Mirror[ Position + 1 ].Object != 0 ) return 0;
    // Располагаем на поверхности зеркала набор предметов.
    Mirror[ Position ].Object = Object;
    Mirror[ Position ].Y_Offset = 0;                                 //
    Mirror[ Position ].Action = FLYING;                              //   X X
    Mirror[ Position + 1 ].Object = Object;                          //
    Mirror[ Position + 1 ].Y_Offset = 0;
    Mirror[ Position + 1 ].Action = FLYING;
   break;

   case 2:
    // Выбираем точку.
    Position = Mirror_size.Height * Mirror_size.Width - Rnd( Mirror_size.Width - 1 ) - 1;
    // Если в точках, которые надо заполнить, что-то есть - возврат.
    if( Mirror[ Position ].Object != 0 ) return 0;
    if( Mirror[ Position - Mirror_size.Width ].Object != 0 ) return 0;
    // Располагаем на поверхности зеркала набор предметов.
    Mirror[ Position ].Object = Object;                              //
    Mirror[ Position ].Y_Offset = 0;                                 //   X
    Mirror[ Position ].Action = FLYING;                              //   X
    Mirror[ Position - Mirror_size.Width ].Object = Object;          //
    Mirror[ Position - Mirror_size.Width ].Y_Offset = 0;
    Mirror[ Position - Mirror_size.Width ].Action = FLYING;
   break;

   case 3:
    // Выбираем точку.
    Position = Mirror_size.Height * Mirror_size.Width - Rnd( Mirror_size.Width - 2 ) - 2;
    // Если в точках, которые надо заполнить, что-то есть - возврат.
    if( Mirror[ Position ].Object != 0 ) return 0;
    if( Mirror[ Position - Mirror_size.Width + 1 ].Object != 0 ) return 0;
    // Располагаем на поверхности зеркала набор предметов.
    Mirror[ Position ].Object = Object;                              //
    Mirror[ Position ].Y_Offset = 0;                                 //   X
    Mirror[ Position ].Action = FLYING;                              //     X
    Mirror[ Position - Mirror_size.Width + 1 ].Object = Object;      //
    Mirror[ Position - Mirror_size.Width + 1 ].Y_Offset = 0;
    Mirror[ Position - Mirror_size.Width + 1 ].Action = FLYING;
   break;

   case 4:
    // Выбираем точку.
    Position = Mirror_size.Height * Mirror_size.Width - Rnd( Mirror_size.Width - 2 ) - 2;
    // Если в точках, которые надо заполнить, что-то есть - возврат.
    if( Mirror[ Position + 1 ].Object != 0 ) return 0;
    if( Mirror[ Position - Mirror_size.Width ].Object != 0 ) return 0;
    // Располагаем на поверхности зеркала набор предметов.
    Mirror[ Position + 1 ].Object = Object;                          //
    Mirror[ Position + 1 ].Y_Offset = 0;                             //     X
    Mirror[ Position + 1 ].Action = FLYING;                          //   X
    Mirror[ Position - Mirror_size.Width ].Object = Object;          //
    Mirror[ Position - Mirror_size.Width ].Y_Offset = 0;
    Mirror[ Position - Mirror_size.Width ].Action = FLYING;
   break;

   case 5:
    // Выбираем точку.
    Position = Mirror_size.Height * Mirror_size.Width - Rnd( Mirror_size.Width - 2 ) - 2;
    // Если в точках, которые надо заполнить, что-то есть - возврат.
    if( Mirror[ Position ].Object != 0 ) return 0;
    if( Mirror[ Position - Mirror_size.Width ].Object != 0 ) return 0;
    if( Mirror[ Position - Mirror_size.Width + 1 ].Object != 0 ) return 0;
    // Располагаем на поверхности зеркала набор предметов.
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
    // Выбираем точку.
    Position = Mirror_size.Height * Mirror_size.Width - Rnd( Mirror_size.Width - 2 ) - 2;
    // Если в точках, которые надо заполнить, что-то есть - возврат.
    if( Mirror[ Position ].Object != 0 ) return 0;
    if( Mirror[ Position + 1 ].Object != 0 ) return 0;
    if( Mirror[ Position - Mirror_size.Width + 1 ].Object != 0 ) return 0;
    // Располагаем на поверхности зеркала набор предметов.
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
    // Выбираем точку.
    Position = Mirror_size.Height * Mirror_size.Width - Rnd( Mirror_size.Width - 2 ) - 2;
    // Если в точках, которые надо заполнить, что-то есть - возврат.
    if( Mirror[ Position + 1 ].Object != 0 ) return 0;
    if( Mirror[ Position - Mirror_size.Width ].Object != 0 ) return 0;
    if( Mirror[ Position - Mirror_size.Width + 1 ].Object != 0 ) return 0;
    // Располагаем на поверхности зеркала набор предметов.
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
    // Выбираем точку.
    Position = Mirror_size.Height * Mirror_size.Width - Rnd( Mirror_size.Width - 2 ) - 2;
    // Если в точках, которые надо заполнить, что-то есть - возврат.
    if( Mirror[ Position ].Object != 0 ) return 0;
    if( Mirror[ Position + 1 ].Object != 0 ) return 0;
    if( Mirror[ Position - Mirror_size.Width ].Object != 0 ) return 0;
    // Располагаем на поверхности зеркала набор предметов.
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
    // Выбираем точку.
    Position = Mirror_size.Height * Mirror_size.Width - Rnd( Mirror_size.Width - 1 ) - 1;
    // Если в точках, которые надо заполнить, что-то есть - возврат.
    if( Mirror[ Position ].Object != 0 ) return 0;
    if( Mirror[ Position - Mirror_size.Width ].Object != 0 ) return 0;
    if( Mirror[ Position - Mirror_size.Width * 2 ].Object != 0 ) return 0;
    // Располагаем на поверхности зеркала набор предметов.
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
    // Выбираем точку.
    Position = Mirror_size.Height * Mirror_size.Width - Rnd( Mirror_size.Width - 3 ) - 3;
    // Если в точках, которые надо заполнить, что-то есть - возврат.
    if( Mirror[ Position ].Object != 0 ) return 0;
    if( Mirror[ Position + 1 ].Object != 0 ) return 0;
    if( Mirror[ Position + 2 ].Object != 0 ) return 0;
    // Располагаем на поверхности зеркала набор предметов.
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
    // Выбираем точку.
    Position = Mirror_size.Height * Mirror_size.Width - Rnd( Mirror_size.Width - 3 ) - 3;
    // Если в точках, которые надо заполнить, что-то есть - возврат.
    if( Mirror[ Position ].Object != 0 ) return 0;
    if( Mirror[ Position - Mirror_size.Width + 1 ].Object != 0 ) return 0;
    if( Mirror[ Position - Mirror_size.Width * 2 ].Object != 0 ) return 0;
    if( Mirror[ Position - Mirror_size.Width * 2 + 2 ].Object != 0 ) return 0;
    // Располагаем на поверхности зеркала набор предметов.
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
    // Выбираем точку.
    Position = Mirror_size.Height * Mirror_size.Width - Rnd( Mirror_size.Width - 3 ) - 3;
    // Если в точках, которые надо заполнить, что-то есть - возврат.
    if( Mirror[ Position ].Object != 0 ) return 0;
    if( Mirror[ Position + 2 ].Object != 0 ) return 0;
    if( Mirror[ Position - Mirror_size.Width + 1 ].Object != 0 ) return 0;
    if( Mirror[ Position - Mirror_size.Width * 2 + 2 ].Object != 0 ) return 0;
    // Располагаем на поверхности зеркала набор предметов.
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
    // Выбираем точку.
    Position = Mirror_size.Height * Mirror_size.Width - Rnd( Mirror_size.Width - 3 ) - 3;
    // Если в точках, которые надо заполнить, что-то есть - возврат.
    if( Mirror[ Position + 2 ].Object != 0 ) return 0;
    if( Mirror[ Position - Mirror_size.Width + 1 ].Object != 0 ) return 0;
    if( Mirror[ Position - Mirror_size.Width * 2 ].Object != 0 ) return 0;
    if( Mirror[ Position - Mirror_size.Width * 2 + 2 ].Object != 0 ) return 0;
    // Располагаем на поверхности зеркала набор предметов.
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
    // Выбираем точку.
    Position = Mirror_size.Height * Mirror_size.Width - Rnd( Mirror_size.Width - 3 ) - 3;
    // Если в точках, которые надо заполнить, что-то есть - возврат.
    if( Mirror[ Position ].Object != 0 ) return 0;
    if( Mirror[ Position + 2 ].Object != 0 ) return 0;
    if( Mirror[ Position - Mirror_size.Width + 1 ].Object != 0 ) return 0;
    if( Mirror[ Position - Mirror_size.Width * 2 ].Object != 0 ) return 0;
    // Располагаем на поверхности зеркала набор предметов.
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
    // Выбираем точку.
    Position = Mirror_size.Height * Mirror_size.Width - Rnd( Mirror_size.Width - 3 ) - 3;
    // Если в точках, которые надо заполнить, что-то есть - возврат.
    if( Mirror[ Position ].Object != 0 ) return 0;
    if( Mirror[ Position + 2 ].Object != 0 ) return 0;
    if( Mirror[ Position - Mirror_size.Width + 1 ].Object != 0 ) return 0;
    if( Mirror[ Position - Mirror_size.Width * 2 ].Object != 0 ) return 0;
    if( Mirror[ Position - Mirror_size.Width * 2 + 2 ].Object != 0 ) return 0;
    // Располагаем на поверхности зеркала набор предметов.
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

 // Перерисовываем все предметы.
 HPS Presentation_space = WinGetPS( Client_window );

 // Действие выполняется два раза - для левого и правого зеркала.
 for( INT Mirrors_count = 1; Mirrors_count <= 2; Mirrors_count ++ )
  {
   // Выбираем зеркало и точки.
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

   // Перерисовываем предметы в выбранном зеркале.
   INT Position = 0;
   for( INT Y_Count = Y_First_point + GRID_WIDTH; Y_Count <= Y_Last_point - SPRITE_HEIGHT; Y_Count += SPRITE_HEIGHT + GRID_WIDTH )
    for( INT X_Count = X_First_point + GRID_WIDTH; X_Count <= X_Last_point - GRID_WIDTH - SPRITE_WIDTH; X_Count += SPRITE_WIDTH + GRID_WIDTH )
      {
       // Перерисовываем предмет.
       if( Mirror[ Position ].Object != 0 )
        DrawSprite( Presentation_space, X_Count, Y_Count + Mirror[ Position ].Y_Offset, Sprite[ Mirror[ Position ].Object ] );

       // Переходим к другому предмету.
       Position ++;
      }
  }

 // Завершаем работу в пространстве отображения окна.
 WinReleasePS( Presentation_space );

 // Возврат.
 return 1;
}

// ─── Удаляет все предметы с поверхности зеркал ───

VOID DeleteAllObjects( VOID )
{
 // Действие выполняется два раза - для левого и правого зеркала.
 for( INT Mirrors_count = 1; Mirrors_count <= 2; Mirrors_count ++ )
  {
   // Выбираем зеркало.
   MIRROR_SQUARES* Mirror = NULL;
   if( Mirrors_count == 1 ) Mirror = Left_mirror; else Mirror = Right_mirror;

   // Все предметы должны исчезнуть.
   for( INT Count = 0; Count < Mirror_size.Width * Mirror_size.Height; Count ++ )
    {
     Mirror[ Count ].Action = DISAPPEARING;
     Mirror[ Count ].Action_count = 0;
     Mirror[ Count ].Change_point = Rnd( STOP_POINT ) + STOP_POINT / 2;
    }
  }

 // Возврат.
 return;
}

// ─── Передвигает набор предметов ───

// Client_window - окно приложения, Where - направление движения.
VOID MoveObjects( HWND Client_window, INT Where )
{
 // Если предметы надо передвинуть влево - делаем это.
 if( Where == WM_LEFT )
  {
   // Указывает, что передвижение возможно.
   INT Movement_is_possible = 1;

   // Действие выполняется два раза - для левого и правого зеркала.
   for( INT Mirrors_count = 1; Mirrors_count <= 2; Mirrors_count ++ )
    {
     // Выбираем зеркало.
     MIRROR_SQUARES* Mirror = NULL;
     if( Mirrors_count == 1 ) Mirror = Left_mirror; else Mirror = Right_mirror;

     // Просматриваем зеркала и проверяем, возможно ли передвижение.
     for( INT V_Count = 0; V_Count < Mirror_size.Height; V_Count ++ )
      for( INT H_Count = 0; H_Count < Mirror_size.Width; H_Count ++ )
       {
        // Если предметы достигли левого края зеркала - передвижение невозможно.
        if( H_Count == 0 )
         {
          INT Position = V_Count * Mirror_size.Width;
          if( Mirror[ Position ].Object != 0 )
           if( Mirror[ Position ].Action == FLYING || Mirror[ Position ].Action == DARTING || Mirror[ Position ].Action == PACKING )
            Movement_is_possible = 0;
         }
        // Если слева есть неподвижный предмет - передвижение невозможно.
        else
         {
          // Если в квадрате есть летящий предмет:
          INT Position = V_Count * Mirror_size.Width + H_Count;
          if( Mirror[ Position ].Object != 0 )
           if( Mirror[ Position ].Action == FLYING || Mirror[ Position ].Action == DARTING || Mirror[ Position ].Action == PACKING )
            {
             // Смотрим, что расположено слева.
             INT Left_position = Position - 1;
             if( Left_position >= 0 )
              if( Mirror[ Left_position ].Object != 0 )
               if( Mirror[ Left_position ].Action != FLYING )
                if( Mirror[ Left_position ].Action != DARTING )
                 if( Mirror[ Left_position ].Action != PACKING  )
                  Movement_is_possible = 0;

             // Если предмет не пролетел квадрат - смотрим на то, что слева вверху.
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

             // А если он пролетел квадрат - смотрим на то, что слева внизу.
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

   // Передвигаем предметы, если это возможно.
   if( Movement_is_possible )
    {
     // Работаем в пространстве отображения окна.
     HPS Presentation_space = WinGetPS( Client_window );

     // Действие выполняется два раза - для левого и правого зеркала.
     for( INT Mirrors_count = 1; Mirrors_count <= 2; Mirrors_count ++ )
      {
       // Выбираем зеркало и точки.
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

       // Передвигаем предметы.
       INT Position = 0;
       for( INT Y_Count = Y_First_point + GRID_WIDTH; Y_Count <= Y_Last_point - SPRITE_HEIGHT; Y_Count += SPRITE_HEIGHT + GRID_WIDTH )
        for( INT X_Count = X_First_point + GRID_WIDTH; X_Count <= X_Last_point - GRID_WIDTH - SPRITE_WIDTH; X_Count += SPRITE_WIDTH + GRID_WIDTH )
         {
          // Если предмет можно передвинуть - делаем это.
          if( Mirror[ Position ].Object != 0 )
           if( Mirror[ Position ].Action == FLYING || Mirror[ Position ].Action == DARTING || Mirror[ Position ].Action == PACKING )
            {
             // Запоминаем точку, в которой расположен предмет.
             INT Previous_X = X_Count;

             // Задаем новое расположение предмета.
             INT New_X = X_Count - GRID_WIDTH - SPRITE_WIDTH;

             // Скрываем предмет в точке, в которой он был расположен.
             HideSprite( Presentation_space, Previous_X, Y_Count + Mirror[ Position ].Y_Offset );
             // Рисуем предмет в новой точке.
             DrawSprite( Presentation_space, New_X, Y_Count + Mirror[ Position ].Y_Offset, Sprite[ Mirror[ Position ].Object ] );

             // Переносим предмет в другой квадрат.
             INT Next_position = Position - 1;
             Mirror[ Next_position ].Object = Mirror[ Position ].Object;
             Mirror[ Next_position ].Y_Offset = Mirror[ Position ].Y_Offset;
             Mirror[ Next_position ].Action = Mirror[ Position ].Action;
             Mirror[ Next_position ].Action_count = Mirror[ Position ].Action_count;
             Mirror[ Next_position ].Change_point = Mirror[ Position ].Change_point;

             // Выбранный квадрат становится пустым.
             Mirror[ Position ].Object = 0; Mirror[ Position ].Action = 0;
            }

          // Переходим к следующему предмету.
          Position ++;
         }
      }

     // Завершаем работу в пространстве отображения окна.
     WinReleasePS( Presentation_space );
    }
  }

 // Если предметы надо передвинуть вправо - делаем это.
 if( Where == WM_RIGHT )
  {
   // Указывает, что передвижение возможно.
   INT Movement_is_possible = 1;

   // Действие выполняется два раза - для левого и правого зеркала.
   for( INT Mirrors_count = 1; Mirrors_count <= 2; Mirrors_count ++ )
    {
     // Выбираем зеркало.
     MIRROR_SQUARES* Mirror = NULL;
     if( Mirrors_count == 1 ) Mirror = Left_mirror; else Mirror = Right_mirror;

     // Просматриваем зеркала и проверяем, возможно ли передвижение.
     for( INT V_Count = 0; V_Count < Mirror_size.Height; V_Count ++ )
      for( INT H_Count = 0; H_Count < Mirror_size.Width; H_Count ++ )
       {
        // Если предметы достигли правого края зеркала - передвижение невозможно.
        if( H_Count == Mirror_size.Width - 1 )
         {
          INT Position = V_Count * Mirror_size.Width + Mirror_size.Width - 1;
          if( Mirror[ Position ].Object != 0 )
           if( Mirror[ Position ].Action == FLYING || Mirror[ Position ].Action == DARTING || Mirror[ Position ].Action == PACKING )
            Movement_is_possible = 0;
         }
        // Если справа есть неподвижный предмет - передвижение невозможно.
        else
         {
          // Если в квадрате есть летящий предмет:
          INT Position = V_Count * Mirror_size.Width + H_Count;
          if( Mirror[ Position ].Object != 0 )
           if( Mirror[ Position ].Action == FLYING || Mirror[ Position ].Action == DARTING || Mirror[ Position ].Action == PACKING )
            {
             // Смотрим, что расположено справа.
             INT Right_position = Position + 1;
             if( Right_position < Mirror_size.Width * Mirror_size.Height )
              if( Mirror[ Right_position ].Object != 0 )
               if( Mirror[ Right_position ].Action != FLYING )
                if( Mirror[ Right_position ].Action != DARTING )
                 if( Mirror[ Right_position ].Action != PACKING )
                  Movement_is_possible = 0;

             // Если предмет не пролетел квадрат - смотрим на то, что справа вверху.
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

             // А если он пролетел квадрат - смотрим на то, что справа внизу.
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

   // Передвигаем предметы, если это возможно.
   if( Movement_is_possible )
    {
     // Работаем в пространстве отображения окна.
     HPS Presentation_space = WinGetPS( Client_window );

     // Действие выполняется два раза - для левого и правого зеркала.
     for( INT Mirrors_count = 1; Mirrors_count <= 2; Mirrors_count ++ )
      {
       // Выбираем зеркало и точки.
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

       // Передвигаем предметы.
       INT Position = Mirror_size.Width * Mirror_size.Height - 1;
       for( INT Y_Count = Y_Last_point - SPRITE_HEIGHT - GRID_WIDTH; Y_Count >= Y_First_point; Y_Count -= SPRITE_HEIGHT + GRID_WIDTH )
        for( INT X_Count = X_Last_point - GRID_WIDTH - SPRITE_WIDTH; X_Count >= X_First_point + GRID_WIDTH; X_Count -= SPRITE_WIDTH + GRID_WIDTH )
         {
          // Если предмет можно передвинуть - делаем это.
          if( Mirror[ Position ].Object != 0 )
           if( Mirror[ Position ].Action == FLYING || Mirror[ Position ].Action == DARTING || Mirror[ Position ].Action == PACKING )
            {
             // Запоминаем точку, в которой расположен предмет.
             INT Previous_X = X_Count;

             // Задаем новое расположение предмета.
             INT New_X = X_Count + GRID_WIDTH + SPRITE_WIDTH;

             // Скрываем предмет в точке, в которой он был расположен.
             HideSprite( Presentation_space, Previous_X, Y_Count + Mirror[ Position ].Y_Offset );
             // Рисуем предмет в новой точке.
             DrawSprite( Presentation_space, New_X, Y_Count + Mirror[ Position ].Y_Offset, Sprite[ Mirror[ Position ].Object ] );

             // Переносим предмет в другой квадрат.
             INT Next_position = Position + 1;
             Mirror[ Next_position ].Object = Mirror[ Position ].Object;
             Mirror[ Next_position ].Y_Offset = Mirror[ Position ].Y_Offset;
             Mirror[ Next_position ].Action = Mirror[ Position ].Action;
             Mirror[ Next_position ].Action_count = Mirror[ Position ].Action_count;
             Mirror[ Next_position ].Change_point = Mirror[ Position ].Change_point;

             // Выбранный квадрат становится пустым.
             Mirror[ Position ].Object = 0; Mirror[ Position ].Action = 0;
            }

          // Переходим к следующему предмету.
          Position --;
         }
      }

     // Завершаем работу в пространстве отображения окна.
     WinReleasePS( Presentation_space );
    }
  }

 // Если предметы надо ускорить - делаем это.
 if( Where == WM_DOWN )
  for( INT Count = 0; Count < Mirror_size.Width * Mirror_size.Height; Count ++ )
   {
    if( Left_mirror[ Count ].Object != 0 ) if( Left_mirror[ Count ].Action == FLYING )
     Left_mirror[ Count ].Action = DARTING;
    if( Right_mirror[ Count ].Object != 0 ) if( Right_mirror[ Count ].Action == FLYING )
     Right_mirror[ Count ].Action = DARTING;
   }

 // Если предметы надо перенести в другое зеркало - делаем это.
 if( Where == WM_BOUNCE )
  {
   // Указывает, что передвижение возможно.
   INT Bounce_is_possible = 1;

   // Действие выполняется два раза - для левого и правого зеркала.
   for( INT Mirrors_count = 1; Mirrors_count <= 2; Mirrors_count ++ )
    {
     // Выбираем зеркало.
     MIRROR_SQUARES* Mirror = NULL; MIRROR_SQUARES* Bounce_mirror = NULL;
     if( Mirrors_count == 1 )
      { Mirror = Left_mirror; Bounce_mirror = Right_mirror; }
     else
      { Mirror = Right_mirror; Bounce_mirror = Left_mirror; }

     // Просматриваем зеркала и проверяем, возможно ли передвижение.
     for( INT V_Count = 0; V_Count < Mirror_size.Height; V_Count ++ )
      for( INT H_Count = 0; H_Count < Mirror_size.Width; H_Count ++ )
       {
        // Если в другом зеркале есть неподвижный предмет - передвижение невозможно.
        INT Position = V_Count * Mirror_size.Width + H_Count;
        if( Mirror[ Position ].Object != 0 )
         if( Mirror[ Position ].Action == FLYING || Mirror[ Position ].Action == DARTING || Mirror[ Position ].Action == PACKING )
          {
           // Смотрим, что расположено в другом зеркале.
           INT Mirrored_position = ( V_Count * Mirror_size.Width + Mirror_size.Width - 1 ) - H_Count;
           INT Bounce_position = Mirrored_position;
           if( Bounce_mirror[ Bounce_position ].Object != 0 )
            if( Bounce_mirror[ Bounce_position ].Action != FLYING )
             if( Bounce_mirror[ Bounce_position ].Action != DARTING )
              if( Bounce_mirror[ Bounce_position ].Action != PACKING  )
               Bounce_is_possible = 0;

           // Если предмет не пролетел квадрат - смотрим на то, что в другом зеркале вверху.
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

           // А если он пролетел квадрат - смотрим на то, что в другом зеркале внизу.
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

   // Переносим предметы, если это возможно.
   if( Bounce_is_possible )
    {
     // Работаем в пространстве отображения окна.
     HPS Presentation_space = WinGetPS( Client_window );

     // Действие выполняется два раза - для левого и правого зеркала.
     INT Bounce_is_performed = 0;
     for( INT Mirrors_count = 1; Mirrors_count <= 2; Mirrors_count ++ )
      {
       // Выбираем зеркала и точки.
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

       // Переносим предметы.
       if( !Bounce_is_performed )
        for( INT V_Count = 0; V_Count < Mirror_size.Height; V_Count ++ )
         for( INT H_Count = 0; H_Count < Mirror_size.Width; H_Count ++ )
          {
           // Если предмет можно перенести - делаем это.
           INT Position = V_Count * Mirror_size.Width + H_Count;
           if( Mirror[ Position ].Object != 0 )
            if( Mirror[ Position ].Action == FLYING || Mirror[ Position ].Action == DARTING )
             {
              // Скрываем предмет в точке, в которой он был расположен.
              INT X = X_First_point + GRID_WIDTH + ( SPRITE_WIDTH + GRID_WIDTH ) * H_Count;
              INT Y = Y_First_point + GRID_WIDTH + ( SPRITE_HEIGHT + GRID_WIDTH ) * V_Count + Mirror[ Position ].Y_Offset;
              HideSprite( Presentation_space, X, Y );

              // Переносим предмет в другое зеркало.
              INT Bounce_position = ( V_Count * Mirror_size.Width + Mirror_size.Width - 1 ) - H_Count;
              Bounce_mirror[ Bounce_position ].Object = Mirror[ Position ].Object;
              Bounce_mirror[ Bounce_position ].Y_Offset = Mirror[ Position ].Y_Offset;
              Bounce_mirror[ Bounce_position ].Action = Mirror[ Position ].Action;

              // Выбранный квадрат становится пустым.
              Mirror[ Position ].Object = 0; Mirror[ Position ].Action = 0;

              // Второй раз этого делать не надо.
              Bounce_is_performed = 1;
             }
          }
      }

     // Завершаем работу в пространстве отображения окна.
     WinReleasePS( Presentation_space );
    }
  }

 // Возврат.
 return;
}

// ─── Обрабатывает сообщения от мыши ───

VOID TranslateMouseMessages( INT X_Point, INT Y_Point )
{
 // Точки окна, которые определяют расположение набора предметов.
 INT X_Left_brink = 0; INT X_Right_brink = 0; INT X_Center = 0;
 // Квадраты зеркала, которые определяют расположение набора предметов.
 INT Left_H_Count = Mirror_size.Width; INT Right_H_Count = 0;

 // Действие выполняется два раза - для левого и правого зеркала.
 for( INT Mirrors_count = 1; Mirrors_count <= 2; Mirrors_count ++ )
  {
   // Выбираем зеркало и точки.
   MIRROR_SQUARES* Mirror = NULL;
   INT X_First_point = 0;
   if( Mirrors_count == 1 )
    { Mirror = Left_mirror; X_First_point = Mirror_points.Hx; }
   else
    { Mirror = Right_mirror; X_First_point = Mirror_points.Jx; }

   // Узнаем квадрат зеркала, в котором расположен набор предметов.
   for( INT V_Count = 0; V_Count < Mirror_size.Height; V_Count ++ )
    for( INT H_Count = 0; H_Count < Mirror_size.Width; H_Count ++ )
     {
      // Если в зеркале есть летящий предмет - запоминаем его расположение.
      INT Position = V_Count * Mirror_size.Width + H_Count;
      if( Mirror[ Position ].Object != 0 )
       if( Mirror[ Position ].Action == FLYING || Mirror[ Position ].Action == DARTING )
        {
         // Запоминаем расположение предметов в зеркале.
         if( H_Count < Left_H_Count ) Left_H_Count = H_Count;
         if( H_Count > Right_H_Count ) Right_H_Count = H_Count;
         // Вычисляем точки окна, которые определяют расположение набора предметов.
         X_Left_brink = X_First_point + GRID_WIDTH + ( SPRITE_WIDTH + GRID_WIDTH ) * Left_H_Count;
         X_Right_brink = X_First_point + GRID_WIDTH + ( SPRITE_WIDTH + GRID_WIDTH ) * Right_H_Count + SPRITE_WIDTH;
         X_Center = ( X_Left_brink + X_Right_brink ) / 2;
        }
     }
  }

 // Узнаем окно приложения - оно должно быть выбранным.
 HWND Client_window = WinQueryFocus( HWND_DESKTOP );

 // Если указатель мыши расположен над зеркалом - ничего не делаем, возврат.
 if( Y_Point > Mirror_points.Ay - SPRITE_HEIGHT - GRID_WIDTH )
  if( X_Point > Mirror_points.Ax ) if( X_Point < Mirror_points.Fx ) return;

 // Если указатель мыши расположен под зеркалом - надо ускорить предметы.
 if( Y_Point < Mirror_points.Hy )
  {
   // Посылаем сообщение в поток.
   WinPostQueueMsg( Thread_message_queue, WM_DOWN, (MPARAM) Client_window, 0 );
   // Возврат.
   return;
  }

 // Если предметы расположены в левом зеркале, а указатель мыши в правом - предметы надо перенести в другое зеркало.
 INT Bounce = 0;
 if( X_Center > Mirror_points.Hx ) if( X_Center < Mirror_points.Ix )
  if( X_Point > Mirror_points.Jx ) Bounce = 1;

 // Если предметы расположены в правом зеркале, а указатель мыши в левом - предметы надо перенести в другое зеркало.
 if( X_Center > Mirror_points.Jx ) if( X_Center < Mirror_points.Kx )
  if( X_Point < Mirror_points.Ix ) Bounce = 1;

 // Переносим предметы в другое зеркало.
 if( Bounce == 1 )
  {
   // Посылаем сообщение в поток.
   WinPostQueueMsg( Thread_message_queue, WM_BOUNCE, (MPARAM) Client_window, 0 );
   // Возврат.
   return;
  }

 // Если указатель мыши расположен слева от предмета - надо передвинуть его.
 if( X_Point < X_Center )
  {
   // Посылаем сообщение в поток.
   WinPostQueueMsg( Thread_message_queue, WM_LEFT, (MPARAM) Client_window, 0 );
   // Возврат.
   return;
  }

 // Если указатель мыши расположен справа от предмета - надо передвинуть его.
 if( X_Point > X_Center )
  {
   // Посылаем сообщение в поток.
   WinPostQueueMsg( Thread_message_queue, WM_RIGHT, (MPARAM) Client_window, 0 );
   // Возврат.
   return;
  }

 // Возврат.
 return;
}
