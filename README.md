# TinySpacer

TinySpacer is a minimal, workspace grouper for Windows.

It does **not** create real Windows “virtual desktops”. It just hides and shows windows fast enough that it feels like it.

## Why

I have used GlazeWM, Komorebi, Workspacer, they are simply too bloated for my needs and I just didn't like how the tiling manager in them got in my way constantly.
I wanted most minimal (tinyspacer uses < 1 MB) way to group windows into a few workspaces on Windows without installing a huge window manager. 


## How to use

1. Download the `tinyspacer.exe` release or build from source.
2. Run it.
3. Look for the numbered TinySpacer icon in the system tray which shows group you are currently inside.
   (If you don't see it, press on "show hidden icons" and drag the TinySpacer icon to the taskbar.)

   ![tray icon](https://github.com/user-attachments/assets/1fbfd8a6-7d0f-4f3a-9b04-f3442f1f2708)

## Hotkeys

- `Alt + 1` ... `Alt + 5`  
  Switch to group 1–5.
  > **Ex:** You are in group 1 and want to move to group 3, just press `Alt + 3`.

- `Alt + Shift + 1` ... `Alt + Shift + 5`  
  Move the current window into group 1–5 and switch there.
  > **Ex:** You are in group 1 and want to move a window to group 3, press `Alt + Shift + 3`. This "drags" the window with you.

By default (when tinyspacer is launched) all currently opened windows will be attached to Group 1.


## Exit
To exit, right‑click the tray icon (on taskbar) and choose “Exit”. This will also release **ALL** the windows that were scattered accross different groups!

## To Build run:
```
./build.bat
```

## Tips
Personally, I have created a task scheduler that triggers **At log on** with **Run with highest privileges**, because the Hide/Show win32 callback in Tinyspacer wont touch windows that ran as administrator.
But if TinySpacer launches with highest privileg then it can handle them as well.

P.S.
TinySpacer is a single exe, a tray icon, and a couple of hotkeys. That’s it.
