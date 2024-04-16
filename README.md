# Blob Kart
### Authors
👤 Owen Cook (Myself)  
👤 [Aaron Perkel](https://github.com/aaronperkel?tab=overview&from=2023-12-01&to=2023-12-31)
---
## Description
This is a continuation of two of our previous projects that were also inspired by Mario Kart. We created a 2D graphical version of our take on Mario Kart: **Blob Kart**. It was created using concepts we learned by completing other projects from this class.

Graphics source files were provided as started code by our instructor.

We plan to implement a cart selection, as well as more power ups. We would also like to add specific handling for specific characters and vehicles (i.e. more weight = less acceleration = more speed). We also have almost finished code for an item box feature. This would allow the cart to drive over an item box that would give the player a random power up. This code is implemented, just commented out as we did not fully finish the idea.

### Requirements
1. Pygame
   1. `pip3 install pygame`

### How to Run
> The commands below should be run in terminal in the project directory.  The game can also be run from CLion or your normal IDE without sound.
1. Run `mkdir -p build`
2. Run `cd build`
3. Run `cmake ..`
   1. If this fails, run `brew install cmake` and try again
4. Run `make`
5. Run `echo "./Blob_Kart & python3 ../play_music.py &" > run.sh`
6. Run `bash run.sh`
> **NOTE:** A separate window will open that will play the music

### Instructions
1. When you run the program, press the `space` key to start the game.
2. Player 1 select a character
3. Player 2 select a character 
   1. Note: Both players are able to select the same character
4. Press the `Play` button
5. Player 1 (left, red car) use `W` `A` `S` `D`
6. Player 2 (right, blue car) use `UP` `LEFT` `DOWN` `RIGHT`
7. After the countdown race your vehicles to complete three laps first!
8. The yellow bar is a boost panel, drive the center of your car over it to gain a boost!

### Game Logic
Each cart is a vector of rectangle objects that move together. There are three invisible "checkpoints" spread out across the racetrack. Each cart must hit all three of these checkpoints for a lap to count. This prevents a player from doing small loops around the finish line to cheat the system. The carts move slower when cutting through the grass to prevent players from trying to take their own shortcuts. 

### Known Bugs
- If you drive into a wall while holding the opposite direction (hold `D` to drive in to right wall and then start holding down `A`), the top half of the cart "drives away" from the rest of the cart.

### Citations

| Issue                | Location                       | Source                                                                                                           |
|----------------------|--------------------------------|------------------------------------------------------------------------------------------------------------------|
| Creating 8-bit Music | [music.mp3](music.mp3)         | [Beepbox.co](https://www.beepbox.co/2_3/#6n31sbk7l04e07tbm0a7g0fj7i0r0o3210T0w6f1d1c1h0v0T0w4f1d0c1h0v2T0w4f1d1c0h0v1T2w0d0v1b004zhmu10000018id3g000004xd2hg004h8j5kd3000p26zFzU2h1i2A5EAMYN74sX4uibcPcPbPcAmpCpCnCsA5gaMm0F5AVelBVD95CpCpBVCib8OILcMbw8agkwwwg8I1OsHbPeibcPcPbPcAmpCpCnCsA5gaMm0F5AVelBVD95CpCpBVCib8OILcMaVBQ5xi5GOBP7j1j1hl0ECwkjgcJ6yCHvOGEt4Q38DYCG7hl0T_Ma9E54Q2yq1DVChl3GCwhkXUW9Etc-eyG0aoY0Gj3wd0qcpgh3A1E3hjbny2yyoZ8dwd0qem2I50a0lwHM79N74IXAV63p0q0QoMV63p0q0QtO6ESdz6aw64e8Uzymsaoy089C18mqC1mm2A5eTmm2LrXuxFAdcxFAdcBKO2LpPdDcI5gasGII5uTOMlrdpulmm2E5elmm2LrVoaIDbSrOMl0FOGOMi6E0)                             |
| Python Music         | [play_music.py](play_music.py) | [Stack Overflow](https://stackoverflow.com/questions/62295329/python-vlc-wont-play-the-audio)                           |