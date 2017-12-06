# Project 3: Pong Game
## The following directory contains:

- Code that implemnts a pong video game using the lcd display and the speaker on the msp430.
- Multiple header files for initialization.
- Watchdog implementation for interrupt handling.
- Assembly code that implements a state transition for game noises
- A pong main file that utilizes all of these files

# Instructions on how to play
## Player 1 intstuctions
- Press button S1 to move left paddle (Player 1) down.
- Press button S2 to move left paddle (Player 1) up.
## Player 2 instructions
- Press button S3 to move right paddle (Player 2) down.
- Press button S4 to move right paddle (Player 2) up.
## To win the game
- If the ball collides with the left edge of the fence then player 1 gets a point.
- If the ball collides with the right edge of the fence then player 2 gets a oint.
- First player to the score of 5 wins.

# How to use this program
## To compile
- $make
## To delete binaries
- $make clean
## To load into msp430
- $make load

# Sources
## For layers code and ideas
- Demo code provided by Dr. Freudenthal for project 3.
## For collisions
- https://github.com/utep-2017-spr-arch1/2017-cobos85
- project from last semester.
- google
## For makefile and readme
- Examples provided by Dr. Freudenthal.
## Other
- All of the ideas were not entirely mine, as I used many different sources.