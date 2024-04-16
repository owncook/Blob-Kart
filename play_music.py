import pygame as pg
import time
import sys

# Initialize Window
pg.init()
screen = pg.display.set_mode((200, 20), pg.RESIZABLE)
pg.display.set_caption("Background Music")

# https://stackoverflow.com/questions/62295329/python-vlc-wont-play-the-audio
pg.mixer.music.load("../intro.wav")
pg.mixer.music.play(1)
start_time = time.time()
loop = False

while True:
    # Set Background Color
    screen.fill(0)

    # Pygame Window Quit Command
    for event in pg.event.get():
        if event.type == pg.QUIT:
            pg.quit()
            sys.exit()

    # Play music on loop
    if (time.time() - start_time >= 28 and not loop):
        pg.mixer.music.load("../loop.wav")
        pg.mixer.music.play(-1)
        loop = True

    # Update display
    pg.display.update()
