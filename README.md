# OpenXcom + Extended + More

OpenXcom (OXC) is an open-source clone of the popular "UFO: Enemy Unknown" 
("X-COM: UFO Defense" in the USA release) and "X-COM: Terror From the Deep" 
videogames by Microprose, licensed under the GPL and written in C++ / SDL.

OpenXcom Extended (OXCE) is OXC up-to-date fork managed, improved and 
supported by Yankes and Meridian. OXCE has many additional features, modding 
capabilities and even scripts, to say the least.

OpenXcom Extended More (OXCEM) is OXCE fork managed by me. Here I implement 
all the features I desire. Sometimes they are simple, sometimes they are not.
Complete list of changes I've made so far (as well as all potential future 
plans) are listed below along with technical details on how to use them.

See more info at the [website](https://openxcom.org)
and the [wiki](https://www.ufopaedia.org/index.php/OpenXcom).

Uses modified code from SDL\_gfx (LGPL) with permission from author.

# OpenXcom Extended More Features
1\. Advanced craft vs hunter killer dogfight behavior.  
2\. Modifiable craft size stat and craft classifications.  
3\. Multi-craft hangar mechanics implementation.  
4\. Bigger craft sprites support for basescape/hangar.  
5\. Base attacks and missile strikes debug trigger.  
6\. Soldier screen direct inventory access shortcut.  
7\. Option to show distance to target, when selecting crafts.  
8\. Game Data Viewer option switch for Tech Tree Data Viewer.  

# Features Migrated to the Main Branch
1\. Configurable Ufopaedia facilities preview.  
2\. Multi-sprite rendering mode for bigger facilities.  
3\. Soldiers and vehicles as modifiable craft stats.  

**Note**: to use features that already migrated to the main branch,
please refer to the [official OXCE documentation](https://www.ufopaedia.org/index.php/Ruleset_Reference_Nightly_(OpenXcom)).  

## Advanced Craft vs Hunter Killer Dogfight
**Constants values for script files (with example below):**  
`constants:`  
`  accelerationPenalty: [10, 10, 10, 10]`  
`  accelerationCoefficient: [[10, 20], [15, 35], [20, 50], [25, 70]]`  
The `accelerationPenalty` is list of acceleration penalties for
various dogfight modes: `Standoff`, `Cautious`, `Combat` and `Maneuver`.  
The `accelerationCoefficient` is list of positive and negative acceleration
coefficients for `Standoff`, `Cautious`, `Combat` and `Maneuver` modes.
**Note**: if `Craft_Acceleration >= Acceleration_Penalty` positive coefficient
is applied.  
**Formula** `Craft_Speed > HK_Speed * (1 + Acceleration_Coefficient/1000 *
(Acceleration_Penalty - Craft_Acceleration))` defines, if dogfight mode is
available versus hunter killer. It should be noted although
`Craft_Speed > HK_Speed` can be `false`, but due to high **acceleration**
it is possible to outmaneuver hunter killer. **Standoff**
defines if you can keep enemy HK at 'safe' non-firing distance. **Cautious**
defines if you get **Evasion Maneuvers** or **Cautious/Long Range** mode vs
hunter killer. **Combat** defines if you have **Standard** mode available vs
hunter killer. And **Maneuver** defines if in **Standard** mode will you be
holding enemy craft at distance of your weapons or not. Only **Disengage**
and **Postpone** behavior didn't change. They still require original
`Craft_Speed > HK_Speed` to be `true`.

## Multi-Craft Hangars, Craft Sizes and Craft Classifications
**Facility values for script files (with example below):**  
`facilities:`  
`  - type: NEW_FANCY_HANGAR`  
`    crafts: 3` Original code for sake of max backwards compatibility.  
`    craftsHidden: false` Flag to render or not render housed crafts in
base view.  
`    craftOptions:` More explanations below in **Usage** section.  
`      - {x: 2, y: -4, min: 30, max: 49, hide: false}` Rendered in base view.  
`      - {x: 2, y: -4, min: 1, max: 9, hide: true}` Always hidden in base view.  
`      - {x: 8, y: 2, min: 30, max: 49, hide: false}` Rendered in base view.  
`      - {x: 8, y: 2, min: 1, max: 9, hide: true}` Always hidden in base view.  
`      - {x: 2, y: -4, min: 0, max: 0, hide: false}` Default hangar values.  
`    optionGroups: [2, 2, 1]` Allows disconnected craft size ranges.  
**Usage**: Now it is possible to define positions of crafts in hangar (when in
base view mode) and if they are permanently hidden. Each **craftOptions** entry
consists from 5 variables: *Horizontal Offset*, *Vertical Offset*, *Craft
Minimum Size*, *Craft Maximum Size*, *Hide Craft Flag*. Or: `x, y, min, max,
hide`, where `x, y, min, max` are `int` value (with range from -2\^31 to 2\^31)
and `hide` is `bool` that be `true` or `false`. The `x, y` define render offset
in base view from facility's center. The `min, max` define which craft can be
housed in slot. The `hide` defines if craft in that specific slot will be
rendered in base view. The `0, 0` for `min, max` is backwards compatibility
option to ignore craft sizes and use classic hangar defaults. All undefined
craft slots have `0, 0` set for their `min, max`. The **optionGroups** allow
to combine into 'single' slot for multiple craft size ranges (such as in
example above: 5 slots can be only used by 3 crafts). Slot priority is always
given to the bigger crafts first.

**Craft values for script files (with example below):**  
`crafts:`  
`  - type: NEW_FANCY_CRAFT`  
`    craftSize: 12`  
This craft with size `12` can't be placed into size 11 hangar slots. But if it
uses value of `0` (default value for all craft), it can be placed into any
hangar slot. The `craftSize` variable has no bounds beside being an `int` (i.e.
from -2\^31 to 2\^31). Modders can define for themselves what craft size ranges
belongs to what craft classes. Class granulation (below) allows modders to
classify what size ranges belong to what classes.

**Craft Weapon values for script files (with example below):**  
`craftWeapons:`  
`  - type: NEW_FANCY_WEAPON`  
`    stats:`  
`      craftSize: 6`  
In this example `craftSize` of `6` means that once this weapon is equipped,
craft's size will be increased by 6. If crafts aren't allowed to change class
(via option) or has no suitable hangar slot after such change, player will
get notification that it can't be equipped.

**Craft Class values for script files (with example below):**  
`craftClasses:` is a new global value for craft class settings.  
`  sizeClassMap:`  
`    250: STR_CLASS_MAX` Size range `250` and above.  Custom upper limit.  
`    190: STR_CLASS_AIR_LARGE` Size range `190 ~ 249` for large aircrafts.  
`    130: STR_CLASS_SUB_LARGE` Size range `130 ~ 189` for large submarines.  
`    70: STR_CLASS_CAR_LARGE` Size range `70 ~ 129` for large vehicles.  
`    50: STR_CLASS_AIR_SMALL` Size range `50 ~ 69` for small aircrafts.  
`    30: STR_CLASS_SUB_SMALL` Size range `30 ~ 49` for small submarines.  
`    10: STR_CLASS_CAR_SMALL` Size range `10 ~ 29` for small vehicles.  
`    1: STR_CLASS_TEAM` Size range `1 ~ 9` for human teams.  
`    0: STR_CLASS_NA` Size `0` is compatibility value. Always leave it as is.  
`    -1: STR_CLASS_NO` Size range `-1` and below. Custom bottom limit.  
This feature allows to assigns custom strings to selected craft size ranges.
Last entry in the list, i.e. `STR_CLASS_NO` will not be rendered or shown in
Ufopaedia (Analysis will show it anyway in numerical format). In conjunction
with **optionGroups** it is possible to create hangar that can house for
example only small sub or large aircraft. The `0` value is reserved for
backwards compatibility, since all aircrafts with undefined craft size have
their `craftSize` set to `0`, but you can assign any string to it.

`  allowClassChange: false` Allows to enforce craft size changes to be within
boundaries declared in the `sizeClassMap` (i.e. you won't be able install
equipment which size stat increases size of modified craft for example from
Small Submarine-class into Small Aircraft-class). By default this enforcement
is enabled, in order to disable it, set value to `true`. If `sizeClassMap`
value isn't defined/declared, the option will be ignored.

**Constants values for script files (with example below):**    
`constants:`  
`  baseCraftListShowClass: true` defines if class column is shown in vessel
list menu that is accessible from the base view. Default value is `false`.  
`  baseCraftListClassShort: false` defines if class column uses short class
abbreviation instead of full classification name. Default value is `false`.  
`  baseShortHangarLinks: true` defines if additional 'CRAFT>' text is
hidden in the basescape, while mouse is over hangar facility with craft.
Default value is `false`.  
`  pediaFacilityLockedStats: true` defines, if amount of shown stat rows in
in Facility's Ufopaedia entries is limited via this option. Default value
is `false`, because vanilla never had more than 5 parameters.  
`  pediaFacilityRowsCutoff: 5` limits how much stat rows will be shown in
Facility's Ufopaedia entries, if option `oxcePediaFacilityLockedStats`
is enabled.  
`  pediaFacilityColOffset: 10` adjusts column in Facility's Ufopaedia entries.
Useful, if you have hangars with many types of slots. Number indicates by how
much column will be moved to the left.  

**Strings Localization Guide (with references below):**  
For each string, in localization files you need to define two entries: 
standard and short.  
`en-US:`  
`  STR_CLASS_CAR_SMALL: "Small Vehicle"`  
`  STR_CLASS_CAR_SMALL_UC: "SV"`  
Standard version will be seen in Craft's Ufopaedia entry. Short will be seen
in Facility's Ufopaedia entry (hangars only).  

_Technical Localization Strings: (saved in **\Language\Technical** folder)_  
`en-US:`  
`  craftSize: "Craft size"`  
`  craftSlotSizes: "Hangar slot sizes"`  

_Standard Localization Strings: (saved in **\Language** folder)_  
`en-US:`  
`  STR_CRAFT_SIZE_UC: "SIZE>{ALT}{0}{ALT}"`  
`  STR_CRAFT_CLASS_UC: "CLASS>{ALT}{0}{ALT}"`  
`  STR_HANGAR_CRAFT_CAP: "Craft Capacity"`  
`  STR_NO_FREE_HANGARS: "HANGAR CAPACITY EXCEEDED!{SMALLLINE}The amount of
crafts we have in {0} exceeds hangar capacity. Transfer or sell the excess
crafts!"`  
`  STR_NO_FREE_HANGARS_AFTER_STRIKE: "HANGAR CAPACITY EXCEEDED!{SMALLLINE}After
missile strike, the amount of crafts we have in {0} exceeds hangar capacity.
Transfer or sell the excess crafts to avoid losing them!"`  
`  STR_NO_FREE_HANGARS_FOR_REFIT: "NO SUITABLE HANGAR SLOT AFTER
REFIT!{SMALLLINE}After refitting craft with selected weapon, system or
utility, its size will change. However, after such refit there is no
suitable hangar slot to house it."`  
`  STR_NO_CRAFT_CLASS_CHANGE: "CRAFT CAN'T CHANGE CLASS!{SMALLLINE}After
refitting craft with selected weapon, system or utility, its size will
change beyond the limits defined by the classification."`  
  `STR_CRAFT_CLASS: "CLASS"`  

## Bigger Craft Spites (for Base View and Refit Screen)
**Craft values for script files (with example below):**  
`crafts:`  
`  - type: BIG_SPRITE_CRAFT`  
`    spriteSize: [54, 72]`  
Now it is possible to define new sprite size for crafts with bigger sprites
in order to still keep them centered at original coordinates. The `spriteSize`
values should be exactly as as vertical and horizontal size of your image in
pixels. Any change to the sprite size should be even: i.e. you can't use 35x45
pixels sprite.

## Base Attack/Missile Strikes Debug Triggers
Only works when in `options.cfg` the option `debug: true` is set. In
**Geoscape**, if you hold SHIFT, while clicking any of your bases, it will
trigger base invasion with random race/faction. If you do so, whilst holding
CTRL, missile strike against selected base will be triggered.

**Note**: If your mod has no `ufos:` rules with `missilePower`, then no missile
strike will be triggered. In addition, all relevant missions, races and UFOs
should be properly linked together, so they can be triggered and spawned
normally by the game, otherwise nothing will spawn and you will get error
message in log what is missing.

## Soldier Screen Direct Inventory Access
Once you're in Soldiers list (that is accessible from Base View), hovering
over soldier in list and clicking inventory hotkey (default `I`) now will
open inventory of that soldier and not of first one in list.  

## Distance to Course Target from Crafts
Allows to enable range measurement in **Geoscape** target list, when you
select 'Set The Course' option. In additional column near base name you will
get calculated range to target (by default in KM).   

**Constants values for script files (with example below):**   
`constants:`  
`  geoShowTargetCourseRange: true` defines if option is enabled or not.
Default value is `false`.  
`  geoTargetCourseRangeMult: 1.8` allows to convert default unit (KM)
into anything you want. Default value is `1.0` (for KM).  
`  geoTargetRangeColOffset: -15` allows to adjust range column to specific
localization via offset.  

_Standard Localization Strings: (saved in **\Language** folder)_  
`en-US:`  
`  STR_BASE_DISTANCE:` `"HIDEOUT,{NEWLINE}DISTANCE (KM)"`  

**Reminder:** Please don't forget to modify `STR_BASE_DISTANCE` localization
string to ensure that it related to the chosen `geoTargetCourseRangeMult` for
correct distance measurement with new units.  

## Game Data Viewer Mode for Tech Tree Viewer
Helps to debug and analyze **Arc Scripts**, **Event Scripts** and **Mission
Scripts**. Mostly works when in `options.cfg` the option `debug: true` is set.
Once debug mode is enabled, go to Options -> Advanced -> Extended, find option
**Advanced Debugging Options**, enable it. Additional sub-section **Advanced
Debugging** will show. In it enable **Tech Tree Viewer - Data Mode**. Load
game, open Tech Tree Viewer via `Q` hotkey. Typing `ASCRIPT`, `ESCRIPT` or
`MSCRIPT` in search will list all Arc/Event/Mission scripts.  

Since it is quite spoiler-y, it can be completely disabled in mods via
`fixedUserOptions` through the `oxceTechTreeDataView: false` and
`oxceShowAdvancedDebugOptions: false` options.  

# OpenXcom Extended More Plans
1\. Increase craft weapons/system limit to 6 in code.  
2\. Add motion-scanner stats, similar to medikit and script integration.  
3\. More depth to base defense: range (in units) for defense facilities.  
4\. Prefab kits for fast base deployment. More expensive than normal
facilities.  
5\. Geoscape base engagement with targets at range (similar to dogfight)?  

# OpenXcom Extended More Far Fetched Plans
1\. Real-Time (with Pause) Geoscape Combat: inspired by X-COM: Apocalypse
cityscape real-time combat. But in this case it will occur on geoscape and
beside crafts vs UFOs, bases (both player's and hostile's) will join the
fray. In addition, capability for bases to shoot passing-by UFOs will add
to strategic complexity and variety.  
2\. Real-Time (with Pause) Battlescape Combat: inspired by X-COM: Apocalypse
real-time mode alternative to turn-based for battlescape. This one will be
pretty much copy-paste from X-COM: Apocalypse. The only issue is that massive
AI rework will be required to ensure that AI can handle real-time combat
just as good as player and without cheating, but by using all the tools that
player can use, such as scanners & etc.  

## Scanner Change Ideas
`scannerResolution: 2` - 1 is default one. 2 will allow you to see at
longer range, but with reduced precision.  
`scannerResolutionSwitch: True` - if `scannerResolution > 1` you can change
it during battle. Thus, you will be able to detect targets at longer range,
close up and switch resolution for precision.  
`scannerModes:` - script names of modes that will be used to calculate the
output (what you see and what you don't). Will be stored as
`std::vector<std::string>`.  
`  - SCANNER_MODE_MOTION_SCRIPT`  
`  - SCANNER_MODE_PSIONIC_SCRIPT`  
`  - SCANNER_MODE_ELECTROMAGNETIC_SCRIPT`  
`  - SCANNER_MODE_HEARTBEAT_SCRIPT`  
`  - SCANNER_MODE_DEMONIC_ENERGY_SCRIPT`  
`scannerAddDefault: true` - allows to enable default motion scanning mode in
addition to custom modes. Irrelevant, if `scannerModes` has no entries,
because it will be working in default mode anyway.  

## Improved Base Defense
New `hitRange` parameter. For example, if missile silo has `hitRange: 6` it
means it will fire 6 times, till UFO will come to base and attack it.  
If base has Grav Shield facility, it means defense facility with `hitRange: 6`
will get another chance for fire 6 salvos.  
If base has different defense facilities, for example, one with `hitRange: 2`
and one with `hitRange: 11`, first 9 times only long range facility will fire,
and last 2 times, both will fire.  
Engagement range of base facilities against UFO begins at range of facility
with highest `hitRange` parameter.  
Formatting: by default single `hitRange` unit equals to 10 km and will be shown
as such in Ufopaedia and Base Defense. Configured via options.  
The `hitTotal: true/false` parameter will define damage calculation
(for backwards compatibility).  
Default `hitTotal` is `true`, when it is set damage per salvo will be calculated
as `defense/hitRange`.  
If `hitTotal` is set to `false`, damage of each salvo will be equal to `defense`
rating.  
The `ammoPerSalvo: true/false` will define if `ammoItem` is consumed with each
salvo, or with each volley. Default is `false`.  
For example, `ammoPerSalvo: true` is valid for single missile, that should be
consumed with each salvo.  
While `ammoPerSalvo: false` is valid for flak cannon that uses box of ammo that
consumed each volley.  

# Installation

OpenXcom requires a vanilla copy of the X-COM resources -- from either or both
of the original games.  If you own the games on Steam, the Windows installer
will automatically detect it and copy the resources over for you.

If you want to copy things over manually, you can find the Steam game folders
at:

    UFO: "Steam\SteamApps\common\XCom UFO Defense\XCOM"
    TFTD: "Steam\SteamApps\common\X-COM Terror from the Deep\TFD"

Do not use modded versions (e.g. with XcomUtil) as they may cause bugs and
crashes.  Copy the UFO subfolders to the UFO subdirectory in OpenXcom's data
or user folder and/or the TFTD subfolders to the TFTD subdirectory in OpenXcom's
data or user folder (see below for folder locations).

## Mods

Mods are an important and exciting part of the game.  OpenXcom comes with a set
of standard mods based on traditional XcomUtil and UFOExtender functionality.
There is also a [mod portal website](https://openxcom.mod.io/) with a thriving
mod community with hundreds of innovative mods to choose from.

To install a mod, go to the mods subdirectory in your user directory (see below
for folder locations).  Extract the mod into a new subdirectory.  WinZip has an
"Extract to" option that creates a directory whose name is based on the archive
name.  It doesn't really matter what the directory name is as long as it is
unique.  Some mods are packed with extra directories at the top, so you may
need to move files around inside the new mod directory to get things straighted
out.  For example, if you extract a mod to mods/LulzMod and you see something
like:

    mods/LulzMod/data/TERRAIN/
    mods/LulzMod/data/Rulesets/

and so on, just move everything up a level so it looks like:

    mods/LulzMod/TERRAIN/
    mods/LulzMod/Rulesets/

and you're good to go!  Enable your new mod on the Options -> Mods page in-game.

## Directory Locations

OpenXcom has three directory locations that it searches for user and game files:

<table>
  <tr>
    <th>Folder Type</th>
    <th>Folder Contents</th>
  </tr>
  <tr>
    <td>user</td>
    <td>mods, savegames, screenshots</td>
  </tr>
  <tr>
    <td>config</td>
    <td>game configuration</td>
  </tr>
  <tr>
    <td>data</td>
    <td>UFO and TFTD data files, standard mods, common resources</td>
  </tr>
</table>

Each of these default to different paths on different operating systems (shown
below).  For the user and config directories, OpenXcom will search a list of
directories and use the first one that already exists.  If none exist, it will
create a directory and use that.  When searching for files in the data
directory, OpenXcom will search through all of the named directories, so some
files can be installed in one directory and others in another.  This gives
you some flexibility in case you can't copy UFO or TFTD resource files to some
system locations.  You can also specify your own path for each of these by
passing a commandline argument when running OpenXcom.  For example:

    openxcom -data "$HOME/bin/OpenXcom/usr/share/openxcom"

or, if you have a fully self-contained installation:

    openxcom -data "$HOME/games/openxcom/data" -user "$HOME/games/openxcom/user" -config "$HOME/games/openxcom/config"

### Windows

User and Config folder:
- C:\Documents and Settings\\\<user\>\My Documents\OpenXcom (Windows 2000/XP)
- C:\Users\\\<user\>\Documents\OpenXcom (Windows Vista/7)
- \<game directory\>\user
- .\user

Data folders:
- C:\Documents and Settings\\\<user\>\My Documents\OpenXcom\data (Windows 2000/XP)
- DATADIR build flag
- C:\Users\\\<user\>\Documents\OpenXcom\data (Windows Vista/7/8)
- \<game directory\>
- . (the current directory)

### Mac OS X

User and Config folder:
- $XDG\_DATA\_HOME/openxcom (if $XDG\_DATA\_HOME is defined)
- $HOME/Library/Application Support/OpenXcom
- $HOME/.openxcom
- ./user

Data folders:
- $XDG\_DATA\_HOME/openxcom (if $XDG\_DATA\_HOME is defined)
- $HOME/Library/Application Support/OpenXcom (if $XDG\_DATA\_HOME is not defined)
- DATADIR build flag
- $XDG\_DATA\_DIRS/openxcom (for each directory in $XDG\_DATA\_DIRS if $XDG\_DATA\_DIRS is defined)
- /Users/Shared/OpenXcom (if $XDG\_DATA\_DIRS is not defined or is empty)
- . (the current directory)

### Linux

User folder:
- $XDG\_DATA\_HOME/openxcom (if $XDG\_DATA\_HOME is defined)
- $HOME/.local/share/openxcom (if $XDG\_DATA\_HOME is not defined)
- $HOME/.openxcom
- ./user

Config folder:
- $XDG\_CONFIG\_HOME/openxcom (if $XDG\_CONFIG\_HOME is defined)
- $HOME/.config/openxcom (if $XDG\_CONFIG\_HOME is not defined)

Data folders:
- $XDG\_DATA\_HOME/openxcom (if $XDG\_DATA\_HOME is defined)
- $HOME/.local/share/openxcom (if $XDG\_DATA\_HOME is not defined)
- DATADIR build flag
- $XDG\_DATA\_DIRS/openxcom (for each directory in $XDG\_DATA\_DIRS if $XDG\_DATA\_DIRS is defined)
- /usr/local/share/openxcom (if $XDG\_DATA\_DIRS is not defined or is empty)
- /usr/share/openxcom (if $XDG\_DATA\_DIRS is not defined or is empty)
- the directory data files were installed to
- . (the current directory)

## Configuration

OpenXcom has a variety of game settings and extras that can be customized, both
in-game and out-game. These options are global and affect any old or new
savegame.

For more details please check the [wiki](https://ufopaedia.org/index.php/Options_(OpenXcom)).

## Development

OpenXcom requires the following developer libraries:

- [SDL](https://www.libsdl.org) (libsdl1.2)
- [SDL\_mixer](https://www.libsdl.org/projects/SDL_mixer/) (libsdl-mixer1.2)
- [SDL\_gfx](https://www.ferzkopp.net/wordpress/2016/01/02/sdl_gfx-sdl2_gfx/) (libsdl-gfx1.2), version 2.0.22 or later
- [SDL\_image](https://www.libsdl.org/projects/SDL_image/) (libsdl-image1.2)
- [yaml-cpp](https://github.com/jbeder/yaml-cpp), version 0.5.3 or later

The source code includes files for the following build tools:

- Microsoft Visual C++ 2010 or newer
- Xcode
- Make (see Makefile.simple)
- CMake

It's also been tested on a variety of other tools on Windows/Mac/Linux. More
detailed compiling instructions are available at the
[wiki](https://ufopaedia.org/index.php/Compiling_(OpenXcom)), along with
pre-compiled dependency packages.

## Compilation

Custom compilation instructions from under `Ubuntu 20.04 LTS x64` **VMware** virtual machine only (yes, its weird that I compile windows executable under linux, but it allows to compile everything into a single file). Current instructions for now are only for windows 64-bit executable. Later I will add instructions for linux, android and iOS as well.

**Note**: make sure that you have enabled 'OpenXcomExMore' VM shared folder and linked it to the actual path, where all OXCE related files reside, such as: `C:\Path\To\Cloned\Git\Folder\OpenXcomExMore`

If you're compiling the executable for the first time in the virtual machine, follow these instructions.
```
sudo apt update
sudo apt-get install autoconf automake autopoint bash bison bzip2 flex g++ g++-multilib gettext git gperf intltool libc6-dev-i386 libgdk-pixbuf2.0-dev libltdl-dev libgl-dev libssl-dev libtool-bin libxml-parser-perl zip lzip make openssl p7zip-full patch perl python3 python3-mako python3-pkg-resources python-is-python3 ruby sed unzip wget xz-utils -y
mkdir /opt
cd /opt
sudo git clone https://github.com/mxe/mxe.git
sudo chown -R `whoami`: mxe
cd ~/Desktop
mkdir -p ~/Desktop/OpenXcomExM
sudo vmhgfs-fuse .host:/OpenXcomExMore ~/Desktop/OpenXcomExM -o allow_other -o auto_unmount
mkdir -p ~/Desktop/OpenXcomExM/build
cd ~/Desktop/OpenXcomExM/build
export PATH=/opt/mxe/usr/bin:$PATH
/opt/mxe/usr/bin/x86_64-w64-mingw32.static-cmake -DCMAKE_BUILD_TYPE=Release -DDEV_BUILD=OFF -DBUILD_PACKAGE=OFF ..
make -j8
/opt/mxe/usr/bin/x86_64-w64-mingw32.static-strip -d ~/Desktop/OpenXcomExM/build/bin/openxcom.exe
mv ~/Desktop/OpenXcomExM/build/bin/openxcom.exe ~/Desktop/OpenXcomExM/build/bin/OpenXcomExM.exe
```

If you're re-compiling the executable after shutting down or restarting virtual machine, follow these instructions:
```
sudo vmhgfs-fuse .host:/OpenXcomExMore ~/Desktop/OpenXcomExM -o allow_other -o auto_unmount
cd ~/Desktop/OpenXcomExM/build
export PATH=/opt/mxe/usr/bin:$PATH
make -j8
/opt/mxe/usr/bin/x86_64-w64-mingw32.static-strip -d ~/Desktop/OpenXcomExM/build/bin/openxcom.exe
mv ~/Desktop/OpenXcomExM/build/bin/openxcom.exe ~/Desktop/OpenXcomExM/build/bin/OpenXcomExM.exe
```

If you're re-compiling the executable within the same virtual machine session, follow these instructions:
```
make -j8
/opt/mxe/usr/bin/x86_64-w64-mingw32.static-strip -d ~/Desktop/OpenXcomExM/build/bin/openxcom.exe
mv ~/Desktop/OpenXcomExM/build/bin/openxcom.exe ~/Desktop/OpenXcomExM/build/bin/OpenXcomExM.exe
```