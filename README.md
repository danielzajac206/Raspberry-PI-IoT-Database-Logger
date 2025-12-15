## Fall 2025 CS341 Honors Project

Project: Database on Raspberry Pi, logging SNES controller input

Simple project that allows a raspberry pi to log input from an SNES controller, saving that data to an SQLite database so that input data will be saved across executions.


## To use: 

Download the github source code

Compile with:

  ###gcc main.c sqlite3.c controller.c -o main.exe -I. -Wall

Then run

  ### sudo ./main.exe

