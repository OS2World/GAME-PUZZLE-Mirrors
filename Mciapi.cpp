// ************************************************************************************
// * Mciapi.cpp позволяет использовать в приложениях вызовы MMOS2.                    *
// ************************************************************************************

// ************************************************************************************
// * MMOS2 не может быть вызвана потоком, если его стек меньше 8 Кбайт.               *
// * Проще всего вызывать MMOS2 при обработке сообщений, приходящих окну.             *
// ************************************************************************************

// ************************************************************************************
// * Чтобы не занимать много памяти, звуки должны загружаться по требованию.          *
// * Звуки воспроизводятся MMOS2 отдельно от приложения, и после того, как звук будет *
// * воспроизведен, память надо освобождать.                                          *
// ************************************************************************************

#define INCL_DOSMODULEMGR


// Определения для указателей на вызовы MMOS2.
typedef ULONG ( APIENTRY MCISENDCOMMAND ) ( USHORT, USHORT, ULONG, PVOID, USHORT );

// Переменные для работы с MMOS2, указатели на вызовы.
struct MM_CALLS
 {
  // Указатель на Mdm.dll.
  HMODULE Mdm_module;
  // Указатель на вызов mciSendCommand(), который предоставляет Mdm.dll.
  MCISENDCOMMAND* mciSendCommand;
 };
MM_CALLS MM_Calls = { NULLHANDLE, NULLHANDLE };


// Команды MMOS2. Полное описание можно найти в Mcios2.h
#define MCI_OPEN                        1
#define MCI_CLOSE                       2
#define MCI_PLAY                        4
#define MCI_REWIND                     46

// Дополнительные параметры для любой команды.
#define MCI_NOTIFY             0x00000001L
#define MCI_WAIT               0x00000002L
#define MCI_OPEN_ELEMENT       0x00000100L
#define MCI_OPEN_SHAREABLE     0x00002000L
#define MCI_OPEN_READONLY      0x00008000L

// Значения, которые возвращает mciSendCommand().
#define MCIERR_SUCCESS                  0
#define MCIERR_CANNOT_LOAD_DRIVER    5010

// Сообщение окну о том, что звук воспроизведен.
#define MM_MCINOTIFY               0x0500

// Определения переменных
typedef USHORT HSOUND;

// Параметры команды открытия файла.
struct MCI_OPEN_PARMS
 {
  HWND    hwndCallback;    /* PM window handle for MCI notify message */
  USHORT  usDeviceID;      /* Device ID returned to user              */
  USHORT  usReserved0;     /* Reserved                                */
  PSZ     pszDeviceType;   /* Device name                             */
  PSZ     pszElementName;  /* Typically a file name or NULL           */
  PSZ     pszAlias;        /* Optional device alias                   */
 };

// Параметры команды воспроизведения звука.
struct MCI_PLAY_PARMS
 {
  HWND    hwndCallback;    /* PM window handle for MCI notify message */
  ULONG   ulFrom;          /* Play from this position                 */
  ULONG   ulTo;            /* Play to this position                   */
 };

// Параметры команды закрытия файла.
struct MCI_GENERIC_PARMS
 {
  HWND   hwndCallback;     /* PM window handle for MCI notify message */
 };


// Подключает к приложению вызовы MMOS2.
INT LoadMMOS2( VOID );

// Отключает MMOS2.
VOID UnloadMMOS2( VOID );

// Вызывает mciSendCommand().
ULONG mciSendCommand( USHORT usDeviceID, USHORT usMessage, ULONG ulParam1, PVOID pParam2, USHORT usUserParm );

// Загружает звук в память.
HSOUND LoadSound( PSZ File_name );

// Воспроизводит загруженный ранее звук.
ULONG PlaySound( HSOUND Sound_ID, HWND Window = NULLHANDLE );

// Освобождает память.
VOID DeleteSound( HSOUND Sound_ID );

// Воспроизводит звуковой файл.
ULONG PlayAudioFile( PSZ File_name );


// // // // // // // // // // // // // // // // // // // // // // // // // // // //
//                                                                               //
// ─── Пример использования Mciapi.cpp. ───                                      //
//                                                                               //
// VOID main( VOID )                                                             //
// {                                                                             //
//  // Подключаем MMOS2.                                                         //
//  LoadMMOS2();                                                                 //
//                                                                               //
//  // Играет звук.                                                              //
//  PlayAudioFile( "C:\\Mmos2\\Sounds\\Горн-1.wav" );                            //
//                                                                               //
//  // Отключаем MMOS2.                                                          //
//  UnloadMMOS2();                                                               //
//                                                                               //
//  // Возврат.                                                                  //
//  return;                                                                      //
// }                                                                             //
//                                                                               //
// // // // // // // // // // // // // // // // // // // // // // // // // // // //


// ─── Подключает к приложению вызовы MMOS2. ───

// Возвращает 1 или 0.
INT LoadMMOS2( VOID )
{
 // Подключаем Mdm.dll.
 APIRET Result = DosLoadModule( NULL, 0, "Mdm.dll", &MM_Calls.Mdm_module );

 // Если это сделать не удалось - возврат.
 if( Result != 0 ) return 0;

 // Подключаем вызов mciSendCommand(), который предоставляет Mdm.dll.
 DosQueryProcAddr( MM_Calls.Mdm_module, 0, "mciSendCommand", (PFN*) &MM_Calls.mciSendCommand );

 // Возврат.
 return 1;
}

// ─── Отключает MMOS2. ───

VOID UnloadMMOS2( VOID )
{
 // Освобождаем Mdm.dll.
 DosFreeModule( MM_Calls.Mdm_module ); MM_Calls.Mdm_module = NULLHANDLE;

 // Возврат.
 return;
}

// ─── Вызывает mciSendCommand(). ───

// Значения те же, что и в OS/2.
ULONG mciSendCommand( USHORT usDeviceID, USHORT usMessage, ULONG ulParam1, PVOID pParam2, USHORT usUserParm )
{
 // Вызываем mciSendCommand().
 if( MM_Calls.mciSendCommand != NULLHANDLE )
  return MM_Calls.mciSendCommand( usDeviceID, usMessage, ulParam1, pParam2, usUserParm );
 // Или возвращаем ошибку "Не был подключен драйвер".
 else return MCIERR_CANNOT_LOAD_DRIVER;
}

// ─── Загружает звук в память. ───

// File_name - имя файла ACM Wave.
// Возвращаемое значение определяет звук в памяти.
HSOUND LoadSound( PSZ File_name )
{
 // Открываем файл.
 MCI_OPEN_PARMS Open_parameters;
 Open_parameters.hwndCallback = NULLHANDLE;
 Open_parameters.usDeviceID = NULLHANDLE;
 Open_parameters.pszDeviceType = NULL;
 Open_parameters.pszElementName = File_name;
 mciSendCommand( NULLHANDLE, MCI_OPEN, MCI_WAIT | MCI_OPEN_ELEMENT | MCI_OPEN_SHAREABLE | MCI_OPEN_READONLY, (PVOID) &Open_parameters, 0 );

 // Возвращаем значение, определяющее звук в памяти.
 return Open_parameters.usDeviceID;
}

// ─── Воспроизводит загруженный ранее звук. ───

// Sound_ID - звук в памяти, Window - окно, которому сообщается о том, что звук воспроизведен.
// Если звук воспроизведен, возвращает 1, иначе 0.
ULONG PlaySound( HSOUND Sound_ID, HWND Window )
{
 // Если окно не указано, то надо ждать, пока звук будет воспроизведен.
 INT Parameter = 0; if( Window == NULLHANDLE ) Parameter = MCI_WAIT; else Parameter = MCI_NOTIFY;

 // Воспроизводим звук. После того, как звук будет воспроизведен, в окно Window будет
 // послано сообщение MM_MCINOTIFY. Обработчик этого сообщения должен освободить память.
 MCI_PLAY_PARMS Play_parameters;
 Play_parameters.hwndCallback = Window;
 Play_parameters.ulFrom = 0; Play_parameters.ulTo = 0;
 ULONG Result = mciSendCommand( Sound_ID, MCI_PLAY, Parameter, (PVOID) &Play_parameters, 0 );

 // Возврат.
 if( Result == MCIERR_SUCCESS ) return 1; else return 0;
}

// ─── Освобождает память. ───

// Sound_ID - звук в памяти.
VOID DeleteSound( HSOUND Sound_ID )
{
 // Закрываем файл.
 MCI_GENERIC_PARMS Close_parameters;
 Close_parameters.hwndCallback = NULLHANDLE;
 mciSendCommand( Sound_ID, MCI_CLOSE, 0, (PVOID) &Close_parameters, 0 );

 // Возврат.
 return;
}

// ─── Воспроизводит звуковой файл. ───

// Переменная File_name задает имя файла.
// Если звук воспроизведен, возвращает 1, иначе 0.
ULONG PlayAudioFile( PSZ File_name )
{
 // Загружаем звук в память.
 HSOUND Sound = LoadSound( File_name );
 // Воспроизводим звук.
 ULONG Result = PlaySound( Sound );
 // Освобождаем память.
 DeleteSound( Sound );

 // Возврат.
 return Result;
}
