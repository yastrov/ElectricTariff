# ElectricTariff Tech Information

## Why WinAPI ?

I want to take small application, with one or two timers, tray icon and menu. GUI Frameworks is
overskill for this task.

## TCHAR vs wchar_t vs char - That's problem.

One answer from WWW: Use wchar_t (or std::wstring) and Unicode for new projects.

## C++ class approach, or simple C-style for main and other place?

That is a question. I think, there is no one right answer. I had used simple style because I could not saw right way to make beautiful architecture. (And have no reason to create one class for all. It have reason, if I can extract )  
But other moments write in C++ style.

I wrote std::wstring in place, where it have reason (need for length information), and wchar_t * (need only pointer), where it more optimal.

For the and of topic: No class only for class.

## Resources

We can change many things via popup menu from "Solution Explorer". Create "resource.rc" like any other file (and don't forget about "resource.h").  
Double click on "resource.rc" (or "Resource View" item) and then we can see new window. Here we can click right mouse button 
on "resource.rc" and choice (click) "Add resource". In window near Explorer ("Properties" window)
we can change language and other.

Any defines we must declare in "resource.h" (and include it in our main.cpp).

We can add: version information, icons, menu, string constants.

Resource file is Unicode. But strings without null terminator (and additional '\0' or '\n' has no effect.)

### Menu 
See [MSDN link](https://msdn.microsoft.com/en-us/library/windows/desktop/ms647558(v=vs.85).aspx#_win32_Displaying_a_Shortcut_Menu).

hPopupMenu = LoadMenuW(hInst, MAKEINTRESOURCEW(IDR_POPUPMENU));

If we load menu from resources, we must show in TrackPopupMenu result of 
GetSubMenu(hPopupMenu, 0), and store in resource.rc like this:

    IDR_POPUPMENU MENU
    BEGIN
        POPUP "TEST, WE NEVER SEE THIS"
        BEGIN
            POPUP "Autostart"
            BEGIN
                MENUITEM "Remove from autostart", ID_AUTOSTART_REMOVE
                MENUITEM "Add to autostart", ID_AUTOSTART_ADD
            END
            MENUITEM SEPARATOR
            MENUITEM "Exit", ID_EXIT
        END
    END

And call before destroy programm:
    DestroyMenu(hPopupMenu);

## x64

We must use x64 friendly code. And use cast functions like: trToInt, LongToIntPtr, PtrToUlong, 
SizeTToInt, SizeTToDWord, PtrToInt, LongToIntPtr, PtrToUlong.
Some of this defined in <Intsafe.h>
See more in Chapter 15 in [Viva64, Ru](http://www.viva64.com/ru/l/full/) [Viva64, En](http://www.viva64.com/en/l/full/) and MSDN.

    int size = 0;
    SizeTToInt(wcslen(buf)*sizeof(wchar_t) + sizeof(wchar_t), &size);

## Compile

I chouse RuntimeLibrary to MultiThreaded:

For project Properties: Configuration Properties --> C/C++ --> Code Generation, change the "Runtime Library" field to "Multi-Threaded (/MT)"

I hope it help distribution without Visual C++ Redistributable package.

## Debug without IDE

For output information to debug console use OutputDebugStringW. We can see string in 
the DebugView tool (SysInternals) (without IDE or another debugger attached at that moment), or IDE output console.
