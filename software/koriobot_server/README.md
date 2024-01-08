# Koriobot Server

The koriobot server is a software backend for controlling and coordinating teams of koriobots. You can run the server and communicate over OSC from many third-party applications, like Blender, TouchDesigner, Rhino/Grasshopper, and Unity.

![](https://github.com/madelinegannon/koriobots/blob/main/assets/koriobot_server/koriobot_server.png)

### Limitations
The koriobot server is only compatible with Windows 10/11. 

Tested On:
- Windows 10/11
- openFrameworks v0.11.2
- ClearView v1.7.15
- Visual Studio 2022

## Setup
There are two ways to get started using the Koriobot Server: download and install a standalone installer (quickest), or build the project from source code (most customizable).

### Installer

1. Download and install the Teknic ClearView SDK ([.zip file](https://teknic.com/files/downloads/ClearView_Install.zip)).
> _Note:_ If you  want to run ClearPath-SC motors without installing ClearView, just install the Clearpath Port Driver ([.zip file](https://teknic.com/files/downloads/ClearPath%20Port%20Driver%20Install.exe)).
2. Download and install the installer:  `koriobots_server_installer.exe` ([.exe file](https://github.com/madelinegannon/koriobots/blob/main/software/koriobot_server/koriobot_server_installer.exe)).
> _Note:_ If the software doesn't run, you may need to manually download and install the latest Microsoft Visual C++ Redistributable ([vc_redist.x64.exe file](https://aka.ms/vs/17/release/vc_redist.x64.exe)). More info from Microsoft [here](https://learn.microsoft.com/en-US/cpp/windows/latest-supported-vc-redist?view=msvc-170#visual-studio-2015-2017-2019-and-2022).

### Build from Source

The koriobot server is built with [openFrameworks](https://openframeworks.cc/) — an open source C++ toolkit for creative coding — and the Teknic's ClearView SDK for controlling ClearPath-SC Series motors.

1. Download and install the Teknic ClearView SDK ([.zip file](https://teknic.com/files/downloads/ClearView_Install.zip))
2. Download and install [Microsoft Visual Studio 2022](https://visualstudio.microsoft.com/vs/community/)
     - Choose the Desktop Development with C/C++ workload
3. Download and install [openFrameworks v0.11.2](https://github.com/openframeworks/openFrameworks/releases/tag/0.11.2)
4. Install additional third-party addons: [ofxGizmo](https://github.com/NickHardeman/ofxGizmo)
5. Move the folder `/koriobot_server` into your `openFrameworks/apps/myApps` folder and build with the openFrameworks projectGenerator.
6. Build and Run the project in _Release x64_ mode in the IDE.


### Troubleshooting
- [How to install openFrameworks](https://openframeworks.cc/setup/vs/)
- [How to install an openFrameworks addon](https://openframeworks.cc/learning/01_basics/how_to_add_addon_to_project/)
- [How to use the openFrameworks projectGenerator](https://youtu.be/4k2ZcvC0YEA?si=gY0Y-3vlnC-ouL6y&t=252)

#### Teknic ClearPath Motors

- Teknic Downloads Website: https://teknic.com/downloads/
- Direct Download Teknic ClearView SDK [.zip file](https://teknic.com/files/downloads/ClearView_Install.zip)
- Examples are installed here: `C:\Program Files (x86)\Teknic\ClearView\sdk\examples`
- Source code & libraries are installed here: `C:\Program Files (x86)\Teknic\ClearView\sdk\sFoundation Source\sFoundation`


# Configuration File

Configure global settings for the koriobot system using an XML-based configuration file. These parameters are loaded everytime you run server.

- If you used the `Koriobot Server Installer`, your configuration file is located here: `C:\Program Files (x86)\ATONATON\Koriobot Server\data\configuration.xml`.
- If you built the project from source code, your configuration file is located here: `OF_PATH\apps\myApps\koriobot_server\bin\data\configuration.xml`.

The following code block documents what each configuration setting does:


```xml
<config>
    <!--1=1D, 2=2D, 3=3D-->
    <config_type>2</config_type>
    <!--Total number of koriobots-->
    <num_bots>2</num_bots>
    <!--OSC settings to send/receive messages-->
    <osc>
        <receive>
            <port>55555</port>
        </receive>
        <send>
            <host>127.0.0.1</host>
            <port>55556</port>
        </send>
    </osc>
    <!--System origin (in world coordinates)-->
    <origin>
        <X>-875.000000000</X>
        <Y>0.000000000</Y>
        <Z>0.000000000</Z>
        <QX>0.000000000</QX>
        <QY>0.000000000</QY>
        <QZ>0.000000000</QZ>
        <QW>1.000000000</QW>
    </origin>
    <!--Base reference position of each koriobot (in local coordinates)-->
    <bases>
        <pos>
            <X>0</X>
            <Y>0</Y>
            <Z>0</Z>
        </pos>
        <pos>
            <X>1750</X>
            <Y>0</Y>
            <Z>0</Z>
        </pos>
    </bases>
    <!--Motor Indices for 2D Configurations-->
    <motor_pairs>
        <pair>
            <A>0</A>
            <B>1</B>
        </pair>
    </motor_pairs>
    <!--Cable Drum Configuration-->
    <cable_drum>
        <!--1=LEFT-HANDED, 2=RIGHT-HANDED-->
        <direction>1</direction>
        <diameter_drum>99.000000000</diameter_drum>
        <diameter_cable>0.304800000</diameter_cable>
        <length>30</length>
        <turns>40</turns>
    </cable_drum>
    <!--End Effector Configuration-->
    <ee>
        <!--Offset from center of end-effector (in local coordinates)-->
        <offset>
            <X>100</X>
            <Y>0</Y>
            <Z>0</Z>
        </offset>
    </ee>
    <shutdown>
        <!--Position beyond bounds to lower koriobots before shutdown (in local coordinates)-->
        <pos>
            <X>0</X>
            <Y>3000</Y>
            <Z>0</Z>
        </pos>
    </shutdown>
    <!--Centroid, width, height of rectangular motion boundary (in World Coordinates)-->
    <bounds>
        <motion>
            <X>0</X>
            <Y>-1500.000000000</Y>
            <W>1750.0</W>
            <H>2600.0</H>
        </motion>
        <safety>
            <X>0</X>
            <Y>-1000.0</Y>
            <W>1000.0</W>
            <H>1000.0</H>
        </safety>
    </bounds>
    <!--Force Limits-->
    <limits>
        <velocity>35.0</velocity>
        <acceleration>200.0</acceleration>
        <torque>
            <MIN>-5</MIN>
            <MAX>35</MAX>
        </torque>
    </limits>
</config>
```

# Server API

| Message Address | Arg Format | Description |
| --- | ------ | --- |
| /stop | —— | Trigger soft stop all koriobots |
| /move_vel | bool | Toggle velocity move on/off for all koriobots |
| /reset | —— | Trigger reset to the center of the safety bounds |
| /norm | int, float, float | Index and normalized {x,y} target coordinate. _Note: use index=-1 to move all koriobots to the normalized target._ |
| /limits/velocity | float | Sets the velocity limit for all koriobots |
| /limit/acceleration | float | Sets the acceleration limit for all koriobots |
| /bounds/pos | float, float | Sets normalized {x,y} position of the safety boundary |
| /bounds/width | float | Sets normalized width of the safety boundary |
| /bounds/height | float | Sets normalized height of the safety boundary |


