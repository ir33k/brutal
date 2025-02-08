BRUTAL
======

Pebble watchface created during [Rebble][] [Hackathon #002][].

> This project has completely confiscated my life, darling.  My best
> work, I must admit.  Simple, elegant, yet bold.  You will die.


DEVLOG
======

2025.02.07 Fri 18:53 Ah shit, here we go again
----------------------------------------------

I'm going to prepare in advance, like last time.  Starting by setting
up development environment.  I'm not using premade VM.  I prefer to
have local setup.  According to official [Installing the Pebble SDK][]
guide I'm supposed to follow article [Writing a Pebble app in 2020][].
So I do.

But I'm already in trouble.  I need Python2 and I can't install it the
way it is done in the article.  I'm running Debian 12 Bookworm now.
This might be thought.

Ok, I have it.  Here is what I did:

	$ sudo ed /etc/apt/sources.list
	a
	deb http://archive.debian.org/debian/ stretch contrib main non-free
	.
	w
	$ sudo apt update
	$ sudo apt install python2.7

Then I completed the article with no problems.  The `pebble`
command now works.

	$ pebble --version
	Pebble Tool v4.6-rc2 (active SDK: v4.3)


2025.02.08 Sat 08:53 Help me source code, you're my only hope
-------------------------------------------------------------

I created a "Hello World" watchface with white background layer, time
and date printed using default fonts and setup for [Clay][] config.

I remember [Pebble development documentation][] being very helpful but
difficult to navigate.  This is because it's divided into specific
sections and text search input does not work.  So as usual it's just
better to grep the source code.  I couldn't find the `pebble.h` file
at first.  I was searching in files downloaded when installing Pebble
SDK.  But actually those are just tools that install the actual SDK.
By default Pebble SDK is places in `~/.pebble-sdk` and there I found
all header files.

Header files are nice but now we have full access to [PebbleOS][]
source code.  I can finally read implementations of `pebble.h` API
functions which makes programming much easier.  For example I can see
if passing a NULL pointer will cause trouble or it will be just
ignored.  With that I don't have to add extra conditions.

Anyhow, I'm more than ready for Hackathon.


[Rebble]: http://rebble.io/
[Hackathon #002]: https://rebble.io/hackathon-002/
[Installing the Pebble SDK]: https://help.rebble.io/sdk/
[Writing a Pebble app in 2020]: https://willow.systems/blog/pebble-sdk-installation-guide/
[Pebble development documentation]: https://developer.rebble.io/developer.pebble.com/index.html
[Clay]: https://github.com/pebble/clay
[PebbleOS]: https://github.com/google/Pebble
