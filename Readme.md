<img src="logo/FoxthingText.png" style="width: 100%"/>

# YipLoader

YipLoader is a mod loader for Android games. It loads before the game's native library and uses a custom ELF loader and hooking library, Leaf, to provide the needed tools to modify games at runtime on even the newest versions of Android (as of Android 16).

It also provides some "out of the box" goodies for specific games and libraries, like utility functions in Lua scripts.

## But first: A special message to the assholes at Google

Dear Google,

I'd like to take a second to reflect on the fact that you seem to want to abandon everything good about Android in the name of chasing after your competitor: iOS.

First, we see Material "Expressive": a design language that is designed to top Apple in user hostility and unreadability in the name of "being bold" or some equally stupid bullcrap. Seriously, who the fuck thought it was a good idea to round out all of the fonts in the UI so it looks like some shitty 80s PSA? And add a blur that is so hideous and terrible that my device would literally be unusable without the acessibility setting to turn it off? Is there anyone left at this company that is competent?

No, apparently not, because then we hear news of Google adding developer verification: the same totalitarian security system promised to us by Apple, now unwillingly forced down Android's throat by way of Google Play services, the parasite built-in to nearly every Android phone which essentially acts as a creepy stalker that gives Google leverage to track its users and issue any updates it wishes without their users consent.

Google can say that developer verification increases platform security, protecting from malicous apps. They can say that it will reduce the amount of scams that people fall for. They can claim it won't affect user choice and that there will always be alternative options. Except for the fact that none of these claims hold in truth.

It is well known that Google is shit at determining what is malware and what isn't. Just look at any report on the number of malicous apps found on Google Play. Do you think they got there because Google wanted to have some malware on their app store? No, it's because they fucking suck at detecting it, and forcing developers to verify won't somehow make this any better, I will assure you.

It's also well known that humans will always find a way to get scammed out of their life savings, even in times of great security. Even if Google is somehow able to stop all scam apps by way of developer verification, attackers will just move to other - and very often easier - ways of scamming: websites, emails, or perhaps even hacked YouTube channels advertising crypto scams. Funny how that works, isn't it?

And of course claiming that it "won't stop developers" is the biggest load of bullshit I've ever heard. Firstly, to reach basically any users at all, you must now divulge tonnes of personal information to Google. And on top of that, you'll need to pay them 25 dollars for the privlege of an "AI-powered" malware detection system rejecting your perfectly fine weather app as malware because it tries to find the user's location.

Of course, in a society where it is considered generally acceptable to be and/or support a genocidal maniac, what am I to expect?

So fuck you, Google,<br/>
Knot.

P.S. Fuck capitialism. Fuck the ruling class. The world is ready for something better.

## Supported Games

Currently, only games by Mediocre AB are supported, and many built-in features assume they are running with one of their games loaded. It should be possible to add support for other games (especially as I've been doing work to make the shim more generic), though I don't yet have a personal interest in any other games.

| Game | Developer |
| ---- | --------- |
| *Granny Smith* (2012) | Mediocre AB |
| *Smash Hit* (2014) | Mediocre AB |

## Showcase

KnShim has been in several *Smash Hit* mods to provide features not possible in the base game. Here are some of my favourite:

* **Shatter Client** is the primary development target for KnShim. It uses KnShim's utlities to download levels from a server and test them.
* **[Smash Hit Flatbread](https://sites.google.com/view/smashhitlab/mods/shl-mods/smash-hit-flatbread)** uses the shim to create an extremely high effort shitpost that you can't not enjoy.
* **[Mod Blueprints](https://sites.google.com/view/smashhitlab/documentation/mod-blueprints)** include KnShim by default too!

## Docs

The documentation is kept on [the new Smash Hit Wiki](https://smashhit.miraheze.org/wiki/KnShim/Documentation). Feel free to contribute if you have an account. :3

## Building

Build using:

```
ndk-build
```

from the Android NDK.

Any version of the NDK not horrendously oudated should be fine. I usually build using r18.
