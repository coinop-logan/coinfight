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

Only missing piece is for the C++ server to look for these deposit descriptors and consume them, which is tangled up with a conception of separate players, which I have yet to do. This is what I plan to work on for Mon.

### Mon

Spent the whole day implementing deposits on the C++ server and clients.

* Server watches a directory for deposit descriptor files, put there by the Python script
* Server encodes and sends this out to all clients
* Server and clients "execute" the deposit by updating the player balance
* Super basic conception of Players implemented

Next step will be to get a client connection to actually authenticate and connect to a specific player, probably with web3 sigs.

### Tue

Big milestone: web3 auth for client.

* Server sends "challenge text" for client to sign
* Client prints challenge text and asks user sign it with the address they deposited credit to
* Client returns user's submitted sig
* Server recovers user's address from sig+challenge
    * Server sends this address back to client
    * Server records this address as that client connection's address
* Relatedly, server now has separate players, which can only control their own units

Tomorrow the first goal is to iron out any issues if client is on another pc
Right after that, start making game game-like: re-implement commands like move or pickupGold

### Wed

Took quite a rest day today. Needed. But was able to complete many small tasks.

* Units can move around again, and point where they're going.
* Each player has a different color (derived from first 3 chars of ETH address, as RGB val in hex)
* Lots of work toward a better game "state" which has distinct pregame and game states

### Thu

Made progress toward starting a "real" multiplayer game

* Client can now specify server IP (or 'l' for localhost)
* Server waits in pregame state until X players have joined
* Starting the game checks if players can afford a Gateway, and spawns them in a circle around 0,0

Also started reverting back to 2D graphics - I'm running out of time! D:

### Fri/Sat/Sun

Forgot to take notes, but got quite a bit done before the deadline

* Got particle effects for transferring gold
* Deposit functionality, to bring gold back to gateway or make gold piles
* COMBAT! With a fighter, which the Gateway can build
* coinfight_local binary to more rapidly test gameplay tweaks
* Move camera with middle mouse