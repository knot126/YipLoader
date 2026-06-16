<img src="logo/yiploader_powered.png" style="width: 100%"/>

# YipLoader

YipLoader is a mod loader for Android games. It loads before the game's native library and uses a custom ELF loader and hooking library, Leaf, to provide the minimum needed tools to modify games at runtime on even the newest versions of Android (as of Android 16).

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

YipLoader is a very early fork of KnShim, so it should support those games to a very limited extent (keep in mind YipLoader doesn't provide anything by default, unlike KnShim).

## Building

Install the Android NDK, then build using:

```
ndk-build
```
