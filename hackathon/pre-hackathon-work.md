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

### Sun

* Built/deployed deposit/withdraw smart contract.

Shockingly, there isn't a solid Ethereum library for C++. So:

* Built Python "accounting" script to
    * Watch for deposits, then record these in files for the server to consume
    * Watch for files from server requesting withdrawals, and execute them

Only missing piece is for the C++ server to look for these deposit descriptors and consume them, which is tangled up with a conception of separate players, something still yet to do. This is what I'm working on for Mon.