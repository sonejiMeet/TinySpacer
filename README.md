# TinySpacer

TinySpacer is a minimal, no‑bullshit workspace grouper for Windows.

## What it does

- Tracks normal top‑level windows (ignores the taskbar, desktop, tool windows).
- Keeps 5 window groups in memory.
- Hides windows from the inactive groups and shows only the current group.
- Shows a numbered tray icon (1–5) for the active group.

It does **not** create real Windows “virtual desktops”. It just hides and shows windows fast enough that it feels like it.

## Why

I have used GlazeWM, Komorebi, Workspacer, they are simply too bloated for my needs and I just didn't like how the tiling manager in them got in my way constantly.
I wanted most minimal, no‑bullshit way to group windows into a few workspaces on Windows without installing a huge window manager. 

Enjoy!

## Hotkeys

- `Alt + 1` … `Alt + 5`  
  Switch to workspace 1–5.

- `Alt + Shift + 1` … `Alt + Shift + 5`  
  Move the current foreground window into workspace 1–5 and switch there.

All hotkeys are global and work with any normal window.

## How to use

1. Download the `tinyspacer.exe` release or build from source.
2. Run it.
3. Look for the numbered TinySpacer icon in the system tray. (If you dont see it, press on "show hidden icons" and drag the TinySpacer icon to taskbar) <img width="46" height="127" alt="image" src="https://github.com/user-attachments/assets/1fbfd8a6-7d0f-4f3a-9b04-f3442f1f2708" />

4. Use `Alt + 1`–`5` to jump between groups, and `Alt + Shift + 1`–`5` to move the active window.

To exit, right‑click the tray icon and choose “Exit TinySpacer”. This will also release ALL the windows.

## To Build run:
```
./build.bat
```

TinySpacer is a single exe, a tray icon, and a couple of hotkeys. That’s it.
