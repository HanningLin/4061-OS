# CSCI4061:Introduction to Operating Systems Final Programming Project
- login: lin00116 tessm061 
- date: 04/30/18
- name: Hanning Lin, Tyler Tessmann
- Extra credit [YES]

## The purpose of your program
 - It will have 1 main server to hold the election data
 - Many clients can conenct to the server, and send it commands
  - Like adding votes, opening polls, etc.
 - The server will keep track of all the votes, and be able to produce a winner
 - Clients can connect from any machine if they know the server IP and port

## How to compile the program
 - You can simply use <make> in the shell to compile the program

## How to use the program from the syntax
 - ./server <DAG FILE> <Server Port> 
 - ./client <REQ FILE> <Server IP> <Server Port>

## Your x500 and the x500 of your partner
 - lin00116 tessm061 

## Your lecture section and your partner's lecture section
 - Section 1,Section 1 (8am)

## Your partner's and your individual contributions
 - Hanning - server.c, createDAG, dispatch of server.c
 - Tyler - client.c , and socket, thread spawning, and dispatch of server.c
 - Both did minor bug fixes during code review of others code.

## Specify whether you are doing the extra credit or not.
 Yes we have succesfully implented the EC 
