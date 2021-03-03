# Deja Vu

Deja Vu keeps track of the players you've played against so you can answer the question ["Have we not met before, Monsieur?"](https://www.youtube.com/watch?v=Zv4ZiINQxA0)

Settings for controlling the behavior and visual style of the display are in the **F2 - > Plugins - > Deja Vu** menu.

## Overview
- In-game display showing players in current match and how many times you've met them, your record, and custom notes.
  - Can be always-on or only shown when the scoreboard is up.
- Window for displaying and filtering through all of your data.
- Add notes to players while in game with the quick note keybind.

## Features
- You can adjust the scale, position, and size of the display along with which data columns are shown.
  - All record column not shown here

  ![columns scale position](https://media.giphy.com/media/QaMYIdgKLL67jm77qX/giphy.gif)

- You can specify the colors and transparency for different attributes of the display.

  ![colors](https://media.giphy.com/media/tVqcoaSXU1Ujn9cs56/giphy.gif)

- You can toggle whether you want the drawable to always be shown or only while the scoreboard is up.
  - Need gif.
- A window for displaying and filtering all of the data that you have accumulated can be keybound.

  ![bindings](https://media.giphy.com/media/rE6235sXD44OQ6ApDp/giphy.gif)

- While in a match, you can use the quick note keybind to bring up a window containing players you have seen that match in order to write a note about them (please use responsibly, like during a replay).

  ![quicknote](https://media.giphy.com/media/L4eCxmbJMhKIg9K8Vb/giphy.gif)


## Planned Features
- More control over the behavior of the table columns in the display (max width, alignment, etc).

You can contact me with suggestions or issues on discord at **adamk33n3r#1933** or on the GitHub repository linked above.

## Known Issues
- Private matches with spectators seem to cause an infinite freeze. Private matches disabled until a fix is found.

## Changelog

### v2.0.0
- Added ability to toggle drawable columns on/off individually.
- Set a max width to the name column when the notes column is enabled.
- Redesigned the layout of the plugin settings window.
  - Swapped colors to use a new color picker widget (should auto upgrade).
  - Generates the `DejaVu.set` file on the fly.
- Added a new GUI that displays all of your data in a filterable table.
- Added new column for showing record across all playlists.
- Added player notes feature.
  - Can edit any players note in the main GUI and also in a quick-launch dialog while in a match.
  - The quick-launch dialog shows all players that you've seen during that match (even if they've left).
  - Now able to show players notes in the drawable during a match.
- Both new windows are able to have keybinds set in the plugin settings.

### v1.5.0
- Switched to using my new table framework to allow for better sizing/alignment/construction of the drawable.

### v1.4.0
- Added the ability to only show the drawable ui when the scoreboard is up. Turn it on the plugin settings.

### v1.3.9
- Fixed some reported crashes.

### v1.3.8
- Added support for epic players.

### v1.3.7
- Added some more safety checks to prevent crashes.

### v1.3.6
- Disabled in private matches. Was causing issues with spectators that would freeze the game.

### v1.3.5
- Fix from v1.3.4 was almost perfect but there was a small chance the issue was still happening. This version fixes that small chance.

### v1.3.4
- Attempted fix at the met count = 2 bug for newly met players.

### v1.3.3
- Update to x64.

### v1.3.2
- Only shows the * next to names when you're in record mode and the record is 0:0.
- Better text alignment in display.

### v1.3.1
- Fixed 1v1 displaying Waiting... for your team.
- Shows "You" on the teams area you're on.

### v1.3.0
- Added toggle in settings to bring back showing the met count instead of the record.

### v1.2.1
- Fixed issue where it was incrementing met count more than once.
- Turned off logging by default.

### v1.2.0
- Changed met count to track record (wins/losses).
- Now shows a `*` after their name if you've met them before.
- Fixed the casual leaving bug for real this time (crosses fingers).

### v1.1.2
- Display width setting.
- Automatic name shortener when too wide.

### v1.1.1
- X and Y display position settings.
- Moved data file storage into the `bakkesmod/data/dejavu` folder.

### v1.1.0
- Attempt at fixing the unranked recount issue.
- Added options for tracking teammates, enemies, or partied players.

### v1.0.0
- Initial release!