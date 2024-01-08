# Grasshopper Template

[Grasshopper](https://www.rhino3d.com/#:~:text=Kay%2C%20Kiteboard%20Shaper-,Grasshopper,-Grasshopper%20is%20a) is a visual programming environment inside of the popular CAD software, [Rhinoceros 3D](https://www.rhino3d.com/). 

We can use Grasshopper and the [python-osc](https://pypi.org/project/python-osc/) python library to talk to the Koriobot Sever:


![](https://github.com/madelinegannon/koriobots/blob/main/assets/grasshopper/grasshopper_template.png)

## Setup

1. Download and install Rhino 8
> There's a free 90 day trial.
2. Open Grasshopper and add a Python 3 Script to the canvas. 
3. Double Click to open the Python Script Editor.
4. Run the following script to locate where Rhino's contained version of python is stored on your computer:

```
import sys
println(sys.path)
```
> The path should be something like `~\.rhinocode\py39-rh8\`

![](https://github.com/madelinegannon/koriobots/blob/main/assets/grasshopper/grasshopper_python_path.gif)

5. The `~\.rhinocode\py39-rh8\lib` folder is where we need to add external python libraries for Rhino.
6. Install `python-osc` via pip. Open a terminal, and run the command:

```shell
cd ~\.rhinocode\py39-rh8
pip install --target=.\lib python-osc
```
> You will be prompted to install `pip` if you haven't yet.

### Optional Step:
<details>
<summary>How to manually copy/paste system-wide python library into Rhino's version of python:</summary>
<br>
If the local <b>pip install</b> doesn't work, you can install the library system-wide and copy/paste into Rhino's internal version of python.
<brGo to where `python-osc` was installed, copy the folder, and paste it back into Rhino's internal python library folder.
<br><br> The system-wide library path should be something like <i>~\AppData\Local\Programs\Python\Python310\Lib\site-packages</i>
<br><br>
<img src="https://github.com/madelinegannon/koriobots/blob/main/assets/grasshopper/grasshopper_python_library.gif">
</details>


7. Restart Rhino, load Grasshopper, and open `koriobot_controller.gh`. Interact with the sliders or buttons to send a value to the Koriobot Server.

## Notes

- Tested on Windows 10/11, Rhino 8, python 3.9, and python-osc 1.8.3

## Code Reference
These python snippets are the code running inside each of the grasshopper template's _OscSender_ nodes.

```python
# Controls: /stop and /move_vel code
from pythonosc import udp_client

# set up the OSC Sender
client = udp_client.SimpleUDPClient(host, port) 
# send the message       
client.send_message(msg_addr, data)
```

```python
# Motion: /norm code
from pythonosc import udp_client

# set up the OSC Sender
client = udp_client.SimpleUDPClient(host, port)
# cast index from float to int
data[0] = int(data[0])   
# send the message       
client.send_message(msg_addr, data)
```

```python
# Motion: /reset code
from pythonosc import udp_client

# set up the OSC Sender
client = udp_client.SimpleUDPClient(host, port)
# cast index from float to int
data[0] = int(data[0])   
# send the message       
client.send_message(msg_addr, data)
```

```python
# Bounds: /bounds code
from pythonosc import udp_client

# set up the OSC Sender
client = udp_client.SimpleUDPClient(host, port)
 
# send the message
addr = msg_addr + "/pos"  
val = [data[0], data[1]]
client.send_message(addr, val)

addr = msg_addr + "/width"  
val = data[2]
client.send_message(addr, val)

addr = msg_addr + "/height"  
val = data[3]
client.send_message(addr, val)
```

```python
# Motion: /limits code
from pythonosc import udp_client

# set up the OSC Sender
client = udp_client.SimpleUDPClient(host, port)
 
# send the message
addr = msg_addr + "/velocity"  
val = data[0]
client.send_message(addr, val)

addr = msg_addr + "/acceleration"  
val = data[1]
client.send_message(addr, val)
```
