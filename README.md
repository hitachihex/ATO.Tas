# ATO.Tas

Usage:
Create a file called Ato.rec where the game executable is located (it does not create it for you)

Run Ato.Injector.exe after you launch the game.
Run Buggy after you've ran the injector. If it spits out garbage on the window, just right click the directional icon in the top left and click redo.
If the game is ran as administrator, then you will need to run the injector as administrator too, otherwise privileges don't matter.
Place ato.tas.conf (it is auto created if you do not, with default hotkeys) in the directory where the game executable is.

See here for a list of virtual key codes to use:
https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes

Default hotkeys:

F1 - Pause

Numpad9 - Unpause

F3 - Start/Stop playback (toggle), reads from Ato.rec where the game executable is located.

Numpad+ - Increase game speed <br />
Numpad- - Decrease game speed <br />
Numpad / - Set game speed to default (normal speed) <br />
] - Step a frame while paused. (this also reloads the input file so you can make changes to future inputs while framestepping.)<br/>

# Note: You may use multiple input files. Create a folder called Includes in the directory where Ato.exe is. Create files in here and name them anything with an extension of .rec
# But note that multi-level applies from the main input file only, so included input files may not read from another file.

--------------------------------------------------------

For playback to work, the tool Up/Down/Left/Right to be bound to WSAD. <br />
Jump must be bound to Space. <br />
Attack must be bound to X. <br />
Dodge must be bound to Z. <br />
Magic must be found to F. <br />
Menu must be bound to Enter. <br />
Map must be bound to Tab. <br />

#NOTE: 
Commands read from input file are in the format of:
   frames, Actions

(They are case insensitive)
Accepted commands are: <br />
    Runto - to be placed under a line you want to fast forward playback to, then the game will pause (note, no frames for this command) <br />
    Walkto - to be placed under a line you want to normal speed playback to, then the game will pause (note, no frames for this command) <br />
    Left <br />
    LeftP - single frame left press. <br />
    Right <br />
    RightP - single frame right press. <br />
    Up <br />
    UpP - single frame up press. <br />
    Down <br />
    DownP - single frame down press.. <br />
    Jump <br />
    UnJump - presses and releases jump on the same frame. <br />
    Attack <br />
    Dodge <br />
    Map <br />
    Menu <br />
    Proceed <br />
    Magic <br />
    Restart - restart the room you are in. <br />
    DRNGFIX - fixes a crash at the beginning of the game due to draw rng problems I haven't fixed yet. <br />
    FixDash - sets grace timer and speedboostalarm to their values as if you are in active run state. (5.0, 100.0) <br />
    Goto - accepts an integer as a roomID to go to. e.g (1, Goto, 7) <br />
    DeleteSave - accepts an integer as a save number to delete. e.g  (1, DeleteSave, 1 would delete the first save file.) <br />
    CopySave - accepts 2 integers , first is save number to copy from, second is the save number to. e.g (1, CopySave, 1, 2 would copy save 1 to 2.) <br />
    Seed - sets the rng seed. <br />
    Pos - accepts 2 values to set character position. <br />
    

    Slow  - set this on inputs you don't want runto to fast forward during. Can be useful to combat desynchs via finding offending frames that work poorly in runto. <br />
    Read - accepts a sub command for the input file to read from. e.g Read, File.rec. It assumes they are in the Includes folder. This command can only be used in the main Ato.rec file<br />
