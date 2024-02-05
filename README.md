![image](https://github.com/caiovpsilveira/Multiplayer-Pong/assets/86082269/59310b40-688c-42c4-b3bd-fc114e59981a)

This is my implementation of a multiplayer game of Pong.
It uses the linux socket API to serialize and deserialize messages, 
and openGL to render, provided by the glfw library.

Requirements to build: glfw3 (libglfw3-dev)

The provided Makefile will build two targets: server.out and client.out.

This game is not polished, and I do not intend to make so.
This was great to learn principles of network development,
such as server ticks and serialization.
