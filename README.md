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

## OpenXcom Extended More Features
1\. 3x3 facilities preview for Ufopaedia.  
2\. Advanced Craft vs Hunter Killer dogfight behavior.  
3\. Multi-craft Hangars and craft sizes implementation.  
4\. Custom craft size classification and granulation.  
5\. Craft capacity info in Ufopaedia for hangars.  
6\. Implemented support for bigger craft sprites.  
7\. Debug trigger for base attacks and missile strikes.  
8\. Soldier/vehicle capacity as modifiable craft stats.  
9\. Facility sprites over shapes for bigger facilities.  
10\. Soldier screen inventory access of selected soldier via hotkey.  
11\. Option to show distance to target, when selecting crafts.  

### Advanced Craft vs Hunter Killer Dogfight
New global options available for `.rul` files (example below contains
default values).  
Formula: `Craft_Speed > HK_Speed * (1 + Acceleration_Coefficient/1000 *
(Acceleration_Penalty - Craft_Acceleration))`  
Defines, if dogfight mode is available versus hunter killer. `Standoff`
defines if you can keep enemy HK at 'safe' distance to minimize dogfight
window in order to wait for more crafts to join the fray. In addition,
for `Standoff` to be enabled `Craft_Speed > HK_Speed` also must be `true`.
It should be noted that `Craft_Speed > HK_Speed` can be `false` but due to
high `acceleration` it is possible to outmaneuver hunter killer. `Cautious`
defines if you get `Evasion Maneuvers` or `Cautious/Long Range` mode vs
hunter killer. `Combat` defines if you have `Standard` mode available vs
hunter killer. And `Maneuver` defines if in `Standard` mode will you be
holding enemy craft at distance of your weapons or not.
`accelerationBonusDivisor: 3` How much craft's acceleration affects
distance change speed in dogfight.  
`accelerationPenaltyStandoff: 10` Acceleration penalty for `Standoff`
formula.  
`accelerationPenaltyCautious: 10` Acceleration penalty for `Cautious`
formula.  
`accelerationPenaltyCombat: 10` Acceleration penalty for `Combat`
formula.  
`accelerationPenaltyManeuver: 10` Acceleration penalty for `Maneuver`
formula.  
`accelerationCoefficientStandoff: 20` Acceleration effect coefficient
for `Standoff` formula.  
`accelerationCoefficientCautious: 35` Acceleration effect coefficient
for `Cautious` formula.  
`accelerationCoefficientCombat: 50` Acceleration effect coefficient
for `Combat` formula.  
`accelerationCoefficientManeuver: 70` Acceleration effect coefficient
for `Maneuver` formula.  

### Multi-Craft Hangars, Craft Sizes and Craft Classifications
In `facilities:` code in `.rul` files it is possible to define positions
of crafts in hangar (when in base view mode) and if they are permanently
hidden.  
`facilities:`  
`  - type: NEW_FANCY_HANGAR`  
`    crafts: 2` Original code isn't going anywhere for sake of max
backwards compatibility.  
`    craftsHidden: false` Flag to render or not render housed crafts in
base view. Mostly for damaged/sealed hangars.  
`    craftOptions:` Horizontal offset - `X`, Vertical offset - `Y`, Craft
size limit `Z` (0 is no size limit and default).  
`      - [2, -4, 37]` This slot can fit size 37 craft or smaller.  
`      - [8, 2, -25]` This slot can fit size 24 craft or smaller and is
always hidden, regardless of the 'craftsHidden' setting.

In `crafts:` code in `.rul` files it is possible to define craft's size:  
`crafts:`  
`  - type: NEW_FANCY_CRAFT`  
`    craftSize: 12` Can't be placed into size 11 hangar slots. If 0, can be
placed into any hangar slot (0 is default).  
The `craftSize` variable has no bounds beside being an `int` (i.e. from
-2^31 to 2^31). Modders can define for themselves what craft size ranges
belongs to what craft classes.  

In `craftWeapons:` code in `.rul` files it is possible to set by how much 
craft's size is increased or decreased by installed weapon/system:  
`craftWeapons:`  
`  - type: NEW_FANCY_WEAPON`  
`    stats:`  
`      craftSize: 6` Increases craft's size by 6, if equipped. Can't be
equipped if no suitable hangar slots available.  

In global code in `.rul` files it is possible to define to which craft
class belong specific craft size ranges and if changes in craft size (via
weapons/system) allow craft to step out of defined classification bounderies:  
`craftClasses:`  
`  20: STR_CRAFT_CLASS_02` All crafts of size 20 or above will be assigned
`STR_CRAFT_CLASS_02` string.  
`  1: STR_CRAFT_CLASS_01` All crafts of size 1 to 19 will be assigned
`STR_CRAFT_CLASS_01` string.  
`  0: STR_CRAFT_CLASS_NA` All crafts of size 0 or below will be assigned
`STR_CRAFT_CLASS_NA` string.  
`craftsCanChangeClass: false` Allows to enforce craft size changes to be within
boundaries declared in the `craftClasses` (i.e. you won't be able install
equipment which size stat increases size of modified craft for example from
Fighter-class into Bomber-class). By default this enforcement is enabled, in
order to disable it, set value to `true`. If `craftClasses` value isn't
defined/declared, the option will be ignored.  
`pediaFacilityColOffset: 5` adjusts column (in Facility's Ufopaedia entries).
Useful, if you have hangars with many types of slots. Number indicates by how
much column will be moved to the left.  

For each string, in localization files you need to define two entries: 
standard and short.  
`en-US:`  
`  STR_CRAFT_CLASS_01: "Interceptor"`  
`  STR_CRAFT_CLASS_01_UC: "Int"`  
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
`  STR_NO_FREE_HANGARS_FOR_REFIT: "NO SUITABLE HANGAR SLOT AFTER
REFIT!{SMALLLINE}After refitting craft with selected weapon, system or
utility, its size will change. However, after such refit there is no
suitable hangar slot to house it."`  
`  STR_NO_CRAFT_CLASS_CHANGE: "CRAFT CAN'T CHANGE CLASS!{SMALLLINE}After
refitting craft with selected weapon, system or utility, its size will
change beyond the limits defined by the classification."`  

### Bigger Craft Spites (for Base View and Refit Screen)
In `crafts:` code in `.rul` files it is possible to define sprite offset for
crafts with bigger sprites in order to stil keep them centered at original
coordinates.  
`crafts:`  
`  - type: BIG_SPRITE_CRAFT`  
`    bigOffsetX: -11` Because sprite width got increased by 22, i.e.
from 32 to 54.  
`    bigOffsetY: -16` Because sprite height got increased by 32, i.e.
from 40 to 72.  
Everything is based of default craft's sprite size: i.e. 32x40 pixels.  
Any change to the sprite size should be even: i.e. you can't use 35x45
pixels sprite.  
Any change to sprite size should get reflected as negative half in `bigOffsetX`
and `bigOffsetY`.  
For example, if you increased craft's sprite width by `20 pixels`, you will
need to set `bigOffsetX: -10`  
For example, if you increased craft's sprite height by `14 pixels`, you will
need to set `bigOffsetY: -7`  
In theory, you can use this feature to reduce sprite size, but it isn't
tested.  

### Base Attack/Missile Strikes Debug Triggers
Only works when in `options.cfg` the option `debug: true` is set. In
**Geoscape**, if you hold SHIFT, while clicking any of your bases, it will
trigger base attack against random race/faction. If do so, whilst holding CTRL,
missile strike against selected base will be triggered. If your mod has no
`ufos:` with `missilePower` nothing will be triggered.  

### Soldier/Vehicle Capacity as Craft Stats
Since `soldiers` and `vehicles` values are modifiable through equipment,
additional `crafts:` parameter , `maxUnitsLimit` was added.  
`crafts:`  
`  - type: NEW_BIG_TRANSPORT`  
`    maxUnitsLimit: 24`  
It is highly advised for modders to set it to maximum available spawning
locations on the map to prevent any unexpected behavior (such as when amount
of soldiers exceed amount of available spawning locations on map). By default,
if `maxUnitsLimit` isn't set, it will have same value as `soldiers`, which
means, equipment that increases `soldiers` parameter won't have any effect.  

_Technical Localization Strings: (saved in **\Language\Technical** folder)_  
`en-US:`  
`  maxUnitsLimit:` `"Max cargo space"`  

_Standard Localization Strings: (saved in **\Language** folder)_  
`en-US:`  
`  STR_NO_CARGO_SPACE_FOR_REFIT:` `"CRAFT CAN'T CHANGE UNIT
CAPACITY!{SMALLLINE}After refitting craft with selected weapon, system or
utility, its unit capacity will change beyond currently available space. If
your craft has crew assigned to it, please remove them to free occupied
space."`  

### Bigger Facility Sprites over Shapes
In `facilities:` code in `.rul` files it is possible to enable same rendering
behavior for bigger facilities (`size: 2` and more) as for `size: 1` 
facilities: where first shape is drawn and only then facility sprite over it.  
`facilities:`  
`  - type: NEW_BIG_FACILITY`  
`    size: 2`  
`    spriteShape: 2041` Remember that bigger shapes require multiple images.  
`    spriteFacility: 549` Will be drawn over `spriteShape` if enabled.  
`    spriteEnabled: true` Will enabled drawing `spriteFacility`.  
The `spriteEnabled` option is irrelevant for `size: 1` facilities, since it is
their default behavior. Already implemented in main OXCE fork.  

### Selected Soldier Inventory Access via List
Once you're in Soldiers list (that is accessible from Base View), hovering
over soldier in list and clicking inventory hotkey (default `I`) now will
open invetory of that soldier and not of first one in list.  

### Distance to Interception Target from Crafts
In **Geoscape** in target's menu, if you're to hold ALT, while clicking the
`Set the Course` button, instead of base names column, you will see column 
with ranges (in KM) between crafts in list and current target.

## OpenXcom Extended More Plans
1\. Increase craft weapons/system limit to 6 in code.  
2\. Add motion-scanner stats, similar to medikit and script integration.  
3\. More depth to base defense: range (in units) for defense facilities.  
4\. Prefab kits for fast base deployment. More expensive than normal
facilities.  
5\. Geoscape base engagement with targets at range (similar to dogfight)?  

### Scanner Change Ideas
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

### Improved Base Defense
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

## Installation

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
- $XDG\_DATA\_DIRS/openxcom (for each directory in $XDG\_DATA\_DIRS if $XDG\_DATA\_DIRS is defined)
- /Users/Shared/OpenXcom
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
- $XDG\_DATA\_DIRS/openxcom (for each directory in $XDG\_DATA\_DIRS if $XDG\_DATA\_DIRS is defined)
- /usr/local/share/openxcom
- /usr/share/openxcom
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
