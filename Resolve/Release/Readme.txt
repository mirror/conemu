Resolve plugin for Far Manager 2.x and 3.x

Плагин может быть полезен для разработчиков, умеет 2 функции:

1. Аналог 'Error lookup' - по коду ошибки ее имя, описание,
   Severity/Facility, и прочее...
╔══════════════════ CPP Resolve ══════════════════╗
║                                                 ║
║ Expr:   0xC0000022                             ↓║
║ Source: ntdll.dll                              ↓║
║                       [ Resolve ]  [ ErrLook ]  ║
╟─ ╔════════════════ Resolve ═════════════════╗ ──╢
║  ║ Name=STATUS_ACCESS_DENIED                ║   ║
║ T║ Err=0xC0000022, Dec=-1073741790, Word=34 ║   ║
║ V║ SEVERITY_ERROR, FACILITY_NULL            ║   ║
║  ║                                          ║   ║
╚══║ {Access Denied}                          ║═══╝
   ║ A process has requested access to an ob… ║                                                               
   ║                                          ║
   ╟──────────────────────────────────────────╢
   ║                  { OK }                  ║
   ╚══════════════════════════════════════════╝
   

2. 'Resolve' - показать значение макро из Windows API, например,
   чему равен WM_SIZE, CFSTR_FILECONTENTS и т.п.
╔══════════════════ CPP Resolve ══════════════════╗
║                                                 ║
║ Expr:   CFSTR_FILECONTENTS                     ↓║
║ Source:                                        ↓║
║                       [ Resolve ]  [ ErrLook ]  ║
╟─────────────────────────────────────────────────╢
║                                                 ║
║ Type:   char const [13]                         ║
║ Value:  FileContents                            ║
║         [ 1 Copy ]  [ 2 Copy # ]  [ 3 Copy /* ] ║
╚═════════════════════════════════════════════════╝

Умолчания плагина рассчитаны на Visual Studio 9.0 и SDK 7.0.
Пути, инклуды и либы можно настроить через настройку плагина.
Для функции 'Resolve' требуются 'cl.exe' и 'link.exe'.

При нажатии Enter функция (Resolve/ErrLook) выбирается автоматически.

Если при "Resolve" отображается ошибка [spoiler="вида"][code]Command executing failed:
cl.exe /c /nologo /W3 /EHsc /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /D "_CRT_SECURE_NO_WARNINGS" "DefR...[/code][/spoiler], то пути точно нужно настраивать ;) умолчания определились неправильно
