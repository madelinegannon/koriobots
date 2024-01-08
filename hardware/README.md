# Koriobots Hardware

## Overview
The hardware of a Koriobots system is made up of the following components:

- Grooved Cable Drum
- Motor Controller
- Cable & Pulley
- Mounting Components
- End-Effector

Assemble the parts and prints following the [Assembly Tutorial](https://github.com/madelinegannon/koriobots/tree/main/hardware).

### Grooved Cable Drum
Each Koriobot uses a [grooved cable drum](https://wirerope.net/azwr/drums/#:~:text=%EF%80%82-,Drums%20%E2%80%93%20Grooved,-Drums%20are%20the) to reliably wind and unwind their cable. This cable-drum design was developed with mechanical simplicity and ease of production in mind. It features a [ClearPath-SC](https://www.teknic.com/files/downloads/Clearpath-SC%20User%20Manual.pdf) all-in-one servo motor system with full software control, a 3D printed grooved cable drum designed for 0.33mm diameter cable, and a shaft, couplings, standoffs and 3D printed mounts to connect everything together.

| ![](https://github.com/madelinegannon/koriobots/blob/main/assets/hardware/koriobots_cable_drum.jpg) | ![](https://github.com/madelinegannon/koriobots/blob/main/assets/hardware/koriobots_cable_drum_design.gif) |
| --- | --- |

The provided 3D printed grooved cable drum holds over 6 meters (20 feet) of cable. You can modify the design of the drum to hold more or less cable based on your needs. 

### Motor Controller
Koriobots use the boards, cables, and power supplies specified by Teknic for their [ClearPath-SC](https://teknic.com/products/clearpath-brushless-dc-servo-motors/clearpath-sc/scsk-models/) motors. See the [BOM](https://github.com/madelinegannon/koriobots/tree/main/hardware/BOM.md) and Teknic's [ordering guide](https://teknic.com/clearpath-accessories-guide/accessories-guide-sc-dc/#overview) for complete list of required parts.

> The [`Manual_12oz_800_L.mtr`](https://github.com/madelinegannon/koriobots/blob/main/hardware/Manual_12oz_800_L.mtr) configuration file is here only as a reference. You should calibrate each of your motors based on your specific build, following Teknic's [instructions](https://www.teknic.com/files/downloads/Clearpath-SC%20User%20Manual.pdf) (page 52).

| [Communication Hub](https://teknic.com/SC4-HUB/) | [Power Hub](https://teknic.com/POWER4-HUB/) | [Motor Power Supply]() | [Logic Power Supply]() |
| --- | --- | --- | --- |
| ![](https://teknic.com/images/accessories/SC4-HUB_lg.jpg) | ![](https://teknic.com/images/accessories/POWER4-HUB_lg.jpg) | ![](https://teknic.com/images/accessories/IPC-5_lg.jpg) | ![](https://teknic.com/images/accessories/PWR-IO-24VDC_lg.jpg) |
| Provides network connection for up to four ClearPath-SC motors. | Distributes DC bus power to up to four ClearPath motors. | Low Voltage (75 VDC) power supply for motor drive bus power. | General purpose logic power supply. |

### Cable & Pulley
Koriobots precisely position their end-effector by synchronously winding and un-winding their cables. We recommend using braided fishing line for light-weight applications. We have found that it is stiffer and stronger than conventional monofilament, and more available and pliable than thin-diameter wire cable. We have tested with [Sufix Advanced Superline 832 Braid 80lb](https://www.rapala.com/us_en/832-advanced-superline?childSku=us-660-080GH).

We integrate a small pulley for more precise positioning and routing of the cable. We've found it also helps prevent the cable drum from unspooling should the line lose tension. 
> Position the pulley at least 450mm (~18") from the cable drum for optimium spooling and unspooling.

### Mounting Components
We provide 3D printed files for mounting Koriobots to 3/8"-16 and M8 compatible fixtures, such as Unistrut, 45mm t-slot aluminum, and standard grip gear (like truss clamps).

| ![](https://cdn.shopify.com/s/files/1/0614/5546/6701/t/3/assets/Weldless-Connections-With-Unistrut1-520x532.png) | ![](https://8020.net/media/catalog/product/cache/9208ae76a38c408ac391cef6ce87b454/8/0/8020_aluminum_t-slot_profile_45-4545.jpg) | ![](https://www.bhphotovideo.com/images/images500x500/global_truss_pro_clamp_blk_heavy_duty_clamp_with_1450885120_931222.jpg) |
| --- | --- | --- |

### End-Effectors
The end-effector is the thing that the koriobots are moving. You design this component based on your use case. Here are a few things to consider with your design:
- The Koriobots should connect at the Center of Mass of your end-effector.
- The end-effector will move more fluidly if its weight is symmetrically and bilaterally balanced.
- The end-effector should be heavy enough to keep the cable in tension throughout operation.
- The end-effector should not exceed the following recommend payloads that we have tested with this hardware design.

Maximum recommended payloads per configuration:
- 1D: 340g (0.75lb)
- 2D: 680g (1.50lb)
> _Note:_ The motors will overheat and shut off if overloaded, which will result in your payload falling.

[Plumb bobs](https://en.wikipedia.org/wiki/Plumb_bob) are a great default end-effector to start: they can be purchased in a number of weights and are symmetrical.

## Getting Started

1. Reference the [BOM](https://github.com/madelinegannon/koriobots/tree/main/hardware/BOM.md) to order the required parts.
2. Print the 3D models in the [`/cad_files`](https://github.com/madelinegannon/koriobots/tree/main/hardware/cad_files) folder.
3. Assemble the parts and prints following the [Assembly Tutorial]().

## Contributing
This design optimizes for mechanical simplicity, not efficiency. We welcome additional designs that maximize the torque, velocity, and mechanical efficiency of the cable drum design.
