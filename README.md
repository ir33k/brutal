BRUTAL
======

Pebble watchface created during [Rebble][] [Hackathon #002][].


DEVLOG
======

2025.02.07 Fri 18:53
--------------------

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

I have completed the article with no problems.  I have `pebble`
command now.

	$ pebble --version
	Pebble Tool v4.6-rc2 (active SDK: v4.3)


[Rebble]: http://rebble.io/
[Hackathon #002]: https://rebble.io/hackathon-002/
[Installing the Pebble SDK]: https://help.rebble.io/sdk/
[Writing a Pebble app in 2020]: https://willow.systems/blog/pebble-sdk-installation-guide/
