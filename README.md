# Camera System

Currently WIP.

## Current Goal

I need to create a proof-of-concept that'll run on two pieces of target hardware.

These two components consist of a transmitter and receiver:

Transmitter:
- Uses a Luxonis camera
- Takes camera frames and sends over network via gstreamer's RTSP server
- Has a servo that is controlled via gRPC

Receiver:
- Uses SDL for graphics
- Takes RTSP data and displays to user
- Sends protobuf messages on button press

## Next Goal

I'd like to expand out to another transmitter and facilitate switching between them.

Due to the nature of the RTSP server, I can probably also create a "control center" that'll run
on a regular PC. I think I'll just use Qt for that one.

As for the Luxonis camera, I'd like to start leveraging its capabilities for running an AI model.

On top of all this I'd like to refactor the code in the project so that it's not extremely ugly.
