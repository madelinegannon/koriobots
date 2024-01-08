# Koriobot Server

The koriobot server is a software backend for controlling and coordinating teams of koriobots. You can run the server and communicate over OSC from many third-party applications, like Blender, TouchDesigner, Rhino/Grasshopper, and Unity.

## Overview
This repo contains the project files to build the koriobot server from scratch. 
> If you just want to run the executable file, download this [.zip]() and follow _Setup_ instructions #3 to install the Teknic ClearView SDK.

### Limitations
The koriobot server is only compatible with Windows 10/11.
> Linux support coming soon.

Tested On:
- Windows 10/11
- openFrameworks v0.11.2
- Clearview SDK
- Visual Studio 2022

## Setup
The koriobot server is built with [openFrameworks](https://openframeworks.cc/) — an open source C++ toolkit for creative coding — and the Teknic's ClearView SDK for controlling ClearPath-SC Series motors.

1. Download and install [Microsoft Visual Studio 2022](https://visualstudio.microsoft.com/vs/community/)
     - Choose the Desktop Development with C/C++ workload
3. Download and install [openFrameworks v0.11.2](https://github.com/openframeworks/openFrameworks/releases/tag/0.11.2)
    - Additional addons: [ofxGizmo](https://github.com/NickHardeman/ofxGizmo)  
3. Download and install Teknic ClearView SDK [.zip file](https://teknic.com/files/downloads/ClearView_Install.zip)
    - Create an Environmental Variable for the ClearView SDK
![](https://github.com/madelinegannon/koriobots/blob/main/assets/koriobot_server/clearview_env_variable.PNG)

#### More Info: Teknic ClearPath Motors

- Teknic Downloads Website: https://teknic.com/downloads/
- Direct Download Teknic ClearView SDK [.zip file](https://teknic.com/files/downloads/ClearView_Install.zip)
- Examples are installed here: `C:\Program Files (x86)\Teknic\ClearView\sdk\examples`
- Source code & libraries are installed here: `C:\Program Files (x86)\Teknic\ClearView\sdk\sFoundation Source\sFoundation`

### Troubleshooting
- [How to install openFrameworks](https://openframeworks.cc/setup/vs/)
- [How to install an openFrameworks addon](https://openframeworks.cc/learning/01_basics/how_to_add_addon_to_project/)
- [How to set an environmental variable](https://www.c-sharpcorner.com/article/how-to-addedit-path-environment-variable-in-windows-11/)

# Configuration Files

Configure global settings for the koriobot system using the _koriobot_server/bin/data/settings.xml_ file. These parameters are loaded everytime you run the server:

```xml
<config>
     <!--1=1D, 2=2D, 3=3D-->
     <config_type>2</config_type>
     <!--Total number of koriobots-->
     <num_bots>8</num_bots>
     <!--Whether to automatically home the motors on startup-->
     <auto_homing>0</auto_homing>
     <!--IP Address & Ports to send/receive OSC messages-->
     <comms_osc_host>127.0.0.1</osc_send_address>
     <comms_osc_receive_port>55555</osc_receive_port>
     <comms_osc_send_port>55556</osc_send_port>
     <!--System origin (in world coordinates)-->
     <pos_origin>
               <X>0.000000000</X>
               <Y>0.000000000</Y>
               <Z>0.000000000</Z>
     </pos_origin>
     <!--Origin of each koriobot (in local coordinates)-->
     <pos_bots>
          <pos>
               <X>50.000000000</X>
               <Y>0.000000000</Y>
               <Z>0.000000000</Z>
          </pos>
          <pos>
               <X>2250.000000000</X>
               <Y>0.000000000</Y>
               <Z>0.000000000</Z>
          </pos>
          <pos>
               <X>50.000000000</X>
               <Y>0.000000000</Y>
               <Z>-140.000000000</Z>
          </pos>
          <pos>
               <X>2250.000000000</X>
               <Y>0.000000000</Y>
               <Z>-140.000000000</Z>
          </pos>
          <pos>
               <X>50.000000000</X>
               <Y>0.000000000</Y>
               <Z>-280.000000000</Z>
          </pos>
          <pos>
               <X>2250.000000000</X>
               <Y>0.000000000</Y>
               <Z>-420.000000000</Z>
          </pos>  
          <pos>
               <X>50.000000000</X>
               <Y>0.000000000</Y>
               <Z>-420.000000000</Z>
          </pos>          
     </pos_bots>
     <!--Y distance (beyond bounds) to lower koriobots before shutdown-->
     <pos_shutdown>4250.000000000</pos_shutdown>
     <!--Rectangular boundary for safe operation-->
     <limit_bounds>
          <X>3000.000000000</X>
          <Y>4000.000000000</Y>
     </limit_bounds>
     <!--Koriobot Velocity Limit-->
     <limit_vel>35.000000000</limit_vel>
     <!--Koriobot Acceleration Limit-->
     <limit_accel>200.000000000</limit_vel>
     <!--Cable Drum Configuration-->
     <cable_drum>
          <!--0=LEFT_HANDED, 1=RIGHT_HANDED Spiral-->
          <direction>1</direction>
          <!--Used to convert RPM to linear distance-->
          <diameter_drum>99.000000000</diameter_drum>
          <diameter_cable>0.304800000</diameter_cable>
          <length>30</length>
          <turns>40</turns>
    </cable_drum>
</config>
```


# Server API

| Message Address | Arg Format | Description |
| --- | --- | --- |
| /move | bool | Start / stop all koriobots |
| /reset | bool | Move all koriobots the reset point _(defaults to the center of the bounds)_ |
| /tgt_norm | float, float | Normalized {x,y} target coordinate for all koriobots |
| /tgt_norm/0 | float, float | Normalized {x,y} target coordinate for the first koriobot |
| /tgt_norm/# | float, float | Normalized {x,y} target coordinate for the _nth_ koriobot |
| /tgt_abs | float, float | Absolute {x,y} target coordinate for all koriobots |
| /tgt_abs/0 | float, float | Absolute {x,y} target coordinate for the first koriobot |
| /tgt_abs/# | float, float | Absolute {x,y} target coordinate for the _nth_ koriobot |
| /limit_vel | float | Sets the velocity limit for all koriobots |
| /limit_accel | float | Sets the acceleration limit for all koriobots |
| /limit_bounds | float, float | Sets the rectangular {w,h} boundary for the koriobots |


