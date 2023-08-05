## Currently Shelved

I've halted the development of Coinfight to pursure more directly gainful employment for the time being.

# Coinfight

Build an army out of crypto (pCKB at the moment via Godwoken), fight other armies, pick up the pieces, and get out with more crypto! Unless you blunder and lose your army. Then all that money's gone!

As of May 2023 the game is in beta. You can try the tutorial and demo without needing a web3 wallet or funds. For now we're arranging real matches via [the Discord](https://discord.gg/hdhCbCqf5m). For these matches you *will* need a web3 wallet like Metamask connected to https://coinfight.io/.

Some vids if you're not about to download some random guy's "crypto game":
* [Introducing Coinfight](https://medium.com/@coinop.logan/introducing-coinfight-db55c3f918ed) - 10 min read
* [v0.1.6 showcase/demo](https://youtu.be/QRzH7jZX7B4) - 4 min video
* [April 2023 devlog](https://medium.com/@coinop.logan/coinfight-devlog-3-472636deec57) - 4 min read

# To Play Coinfight

* Go to https://coinfight.io/ and download the game launcher from there.
* Try out the tutoriral and the demo. In the demo you can switch teams via the backtick (`) key. You don't need crypto or a wallet for this step!
* [Join the Discord](https://discord.gg/hdhCbCqf5m) to hear about the next open match.
* Before the match starts, connect a web3 wallet like [Metamask](https://metamask.io/) to https://coinfight.io/, make sure you have some pCKB, and follow the instructions there.

If you have no pCKB, someone in the Discord might generously gift you some starting units. You will however still need a web3 wallet to receive them.

# Client Build Instructions (for developers only!)

## Mac and Linux

If using Mac, make sure `brew` is installed.

In the root directory of the project, run `./configure` to install a couple dependecies, then run `make client`.

This should populate `bin/` with the `client` and `coinfight_local` binaries and a few resources. Running `./coinfight_local` is a good way to test that the game runs smoothly without connecting to a server.

## Windows

TODO
