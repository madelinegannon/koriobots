# Examples
This folder has several useful starter projects in various programming environments. 
- Start the `Koriobot Server` before running these examples.
- See the descriptions below for usage details. 

## Processing
### koriobots_example_mouse2D.pde
Shows how to use your mouse to move the koriobots.
- Hover over the magenta rectangle to move the koriobot target.
- Press and drag the mouse to enable moving.
![](https://github.com/madelinegannon/koriobots/blob/main/assets/processing/koriobots_example_mouse2D.gif)

### koriobots_example_physics2D.pde
Shows how to use physics simulation to move the koriobots.
- Install the [Box2D-for-Processing](https://github.com/shiffman/Box2D-for-Processing/tree/master/Box2D-for-Processing/dist) library.
- Press and drag the mouse to throw the box around.
- Toggle the `Enable Move` button to enable moving.
![](https://github.com/madelinegannon/koriobots/blob/main/assets/processing/koriobots_example_physics2D.gif)

## Grasshopper
#### koriobots_example_follow_path.gh
Shows how to move the koriobots to follow a path. Useful for pick-and-place operations
- Open _koriobots_example_follow_path.3dm_, start Grasshopper, then open _koriobots_example_follow_path.gh_ inside Grasshopper.
- Update the `Evaulate Curve` slider to move the koriobot target.
- Toggle the `\move_vel` button `ON` to enable moving.
  
![](https://github.com/madelinegannon/koriobots/blob/main/assets/grasshopper/koriobots_example_follow_path.gif)
