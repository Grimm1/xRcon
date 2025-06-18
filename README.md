# xRcon

xRcon is a Windows-based Remote Console (RCON) application designed for managing game servers for titles like *Medal of Honor* and *Call of Duty* series. It provides a user-friendly graphical interface to add, edit, and manage server configurations, send RCON commands, update server settings, and monitor player activity. The application supports multiple game protocols and offers validation for server details to ensure reliable connectivity.

---

## Table of Contents

- [Features](#features)
- [Supported Games](#supported-games)
- [Installation](#installation)
- [Usage](#usage)
- [Configuration](#configuration)
- [Contributing](#contributing)
- [License](#license)
- [Acknowledgments](#acknowledgments)

---

## Features

- **Server Management**: Add, edit, and delete game server configurations stored in an INI file (`servers.ini`).
- **RCON Command Interface**: Send custom RCON commands to manage servers with confirmation prompts for sensitive actions (e.g., kick, ban, rename).
- **Player Management**: View real-time player information (e.g., slot, name, IP, score, ping) with game-specific actions like kick, ban, rename, or unbind.
  - **Note**: For *Medal of Honor: Allied Assault* and *Medal of Honor: AA Spearhead*, rename and unbind commands require a server-side mod. The necessary scripts are included in the `moh_scripts` folder.
- **Server Settings**: Modify server settings such as hostname, map, and gametype with dropdown selectors and validation.
- **Game-Specific Support**: Tailored support for different games with custom commands and UI adjustments.
- **Automatic Refresh**: Periodic updates for player lists and server settings every 60 seconds.
- **Input Validation**: Ensures valid IP/hostname, port, and list formats for gametypes and maps.
- **Debug Logging**: Logs errors and actions to `debug.log` for troubleshooting.

---

## Supported Games

xRcon supports the following games with their respective protocol IDs:

- **Medal of Honor: Allied Assault** (Protocol ID: 1)
- **Medal of Honor: AA Spearhead** (Protocol ID: 1)
- **Medal of Honor: AA Breakthrough** (Protocol ID: 1)
- **Call of Duty** (Protocol ID: 2)
- **Call of Duty: United Offensive** (Protocol ID: 2)
- **Call of Duty 2** (Protocol ID: 2)
- **Call of Duty 4: Modern Warfare** (Protocol ID: 2)
- **Call of Duty: World at War** (Protocol ID: 2)

---

## Installation

1. **Prerequisites**:
    - Windows operating system (tested on Windows XP and later).
    - Administrative privileges for installation and server management.
    - Network access to communicate with game servers.

2. **Steps**:
    - Clone or download the repository from [GitHub](#) (replace with actual repository URL if available).
    - Ensure the following dependencies are available:
        - Microsoft Visual C++ Runtime (for Windows API and Common Controls).
        - Winsock library (`Ws2_32.lib`).
        - [GameServerQuery-DLL](https://github.com/Grimm1/GameServerQuery-DLL) (required DLL for server querying).
    - Build the project using a C++ compiler (e.g., Visual Studio) with the provided source code.
        - Link against `comctl32.lib`, `Ws2_32.lib`, and `GameServerQuery.lib`.
    - Copy the compiled executable (`xRcon.exe`) and required files (`aa.ico`, `default_maps.ini`, `default_gametypes.ini`, and the `moh_scripts` folder) to your desired directory.
    - For *Medal of Honor: Allied Assault* and *Medal of Honor: AA Spearhead*:
        - Install the server-side mod scripts from the `moh_scripts` folder onto the target game server to enable rename and unbind functionalities. Follow the instructions provided in the `moh_scripts` folder for setup.

3. **Configuration Files**:
    - `servers.ini`: Stores server configurations (created automatically on first save).
    - `default_maps.ini`: Contains default map lists for each game.
    - `default_gametypes.ini`: Contains default gametype lists for each game.
    - `moh_scripts/`: Contains server-side mod scripts for *Medal of Honor* rename and unbind commands.

---

## Usage

1. **Launch the Application**:
    - Run `xRcon.exe` to open the main window.

2. **Navigate the Interface**:
    - **Servers Page**: Add or edit server configurations using the form. Click "Edit" or "Delete" in the server table to modify existing servers.
    - **RCON Page**: Select a server from the dropdown, view player information, send RCON commands, and update server settings (hostname, map, gametype).
    - Use the sidebar to switch between the Servers and RCON pages.

3. **Managing Servers**:
    - Add a new server by filling out the form (server name, IP/hostname, port, game, RCON password, gametypes, maps) and clicking "Save Server".
    - Edit or delete servers via the server table's action columns.
    - Ensure IP/hostname, port, gametypes, and maps are in valid formats (e.g., `id:humanreadable` for lists).

4. **RCON Commands**:
    - Enter commands in the RCON input box and click "Send Command".
    - Sensitive commands (e.g., kick, ban, rename, unbind) prompt for confirmation.
    - For *Medal of Honor: Allied Assault* and *Medal of Honor: AA Spearhead*, ensure the server has the mod scripts from `moh_scripts` installed to use rename and unbind commands.
    - Use the player table to perform actions like kick or ban by clicking the respective buttons.

5. **Server Settings**:
    - Update the server hostname, map, or gametype using the provided controls.
    - Confirm changes with a dialog to prevent accidental modifications.

6. **Troubleshooting**:
    - Check `debug.log` for error messages if issues arise.
    - Ensure the server is online and the RCON password is correct.
    - For *Medal of Honor* rename/unbind issues, verify that the scripts from `moh_scripts` are correctly installed on the server.

---

## Configuration

- **servers.ini**: Stores server details in the format:
```ini
[ServerName]
ip=192.168.0.1
port=12345
game=Medal of Honor: Allied Assault
protocol=1
rconPassword=password123
gametypes=dm:Deathmatch,tdm:Team Deathmatch
maps=mp_brecourt:Brecourt,mp_dawnville:Dawnville
```
- **default_maps.ini**: Contains default maps for each game, e.g.:```ini
[Medal of Honor: Allied Assault]
maps=mp_brecourt:Brecourt,mp_dawnville:Dawnville
```
- **default_gametypes.ini**: Contains default gametypes for each game, e.g.:```ini
[Medal of Honor: Allied Assault]
gametypes=dm:Deathmatch,tdm:Team Deathmatch
```
- **moh_scripts/**: Contains server-side mod scripts for *Medal of Honor: Allied Assault* and *Spearhead* to enable rename and unbind commands.  
  Refer to the documentation inside the `moh_scripts` folder for installation instructions.

---

## Contributing

We welcome contributions to improve xRcon! To contribute:

1. Fork the repository.
2. Create a feature branch (`git checkout -b feature/YourFeature`).
3. Commit your changes (`git commit -m "Add YourFeature"`).
4. Push to the branch (`git push origin feature/YourFeature`).
5. Open a pull request with a detailed description of your changes.

Please ensure your code follows the existing style and includes appropriate comments. Test your changes thoroughly, especially for game-specific features like the Medal of Honor mod scripts in the `moh_scripts` folder. When adding new features, consider compatibility with supported games and ensure any server-side scripts are documented.

---

## License

This project is licensed under the MIT License. See the LICENSE file for details.

---

## Acknowledgments

- Built with the Windows API for a native Windows experience.
- Supports legacy games like Medal of Honor and Call of Duty series.
- Special thanks to the game server community for feedback and testing.
- Gratitude to contributors who provided the server-side mod scripts for Medal of Honor in the `moh_scripts` folder.

---

