# Pre- and Post-hackathon Work

See [this tag](https://github.com/coinop-logan/coinfight/releases/tag/pre-hackathon-work) to browse the work completed pre-hackathon. But here's a summary:

## Completed Before the Hackthon

* Network architecture: Server listens for client commands, and broadcasts this to all clients, occasionally sending a resync packet.
* A "coins" class to help responsibly move in-game currency and mitigate risk of "losing" it anywhere
* Two basic units to bring in credit, and move it around (Gateway and Worker/"Prime")
* Partway through a graphics redesign - can now import .obj models from Blender and use them in the code

**Note that the above does not include...**

* Multiplayer - networking works, but all clients are basically the same player as far as the server is concerned.
* Related to the above, player sign in
* Combat
* Full set of units - still need Factory, Fighter, and Turret
* Blockchain deposits/withdrawals
* Any kind of menu or UX other than a few hard-coded click commands for testing the above

## Completed during the hackathon

I'll fill this in later, near the end of the hackathon!