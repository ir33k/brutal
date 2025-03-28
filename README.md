BRUTAL
======

Pebble watchface created during [Rebble][] [Hackathon #002][].

> This project has completely confiscated my life, darling.  My best
> work, I must admit.  Simple, elegant, yet bold.  You will die.

App store:

- [BRUTAL - aplite][]
- [BRUTAL - basalt][]
- [BRUTAL - diorite][]

![Quick view animation](picture/brutal.gif)
![White on black](picture/08.png)
![How quick view looks like](picture/16.jpg)


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


2025.03.01 Sat 11:31 The time has come
--------------------------------------

First day of hackathon.  I will focus on design.  Like last time I'm
planning on testing ideas by making a simple watchface that display
single BMP image.  That way I can look at watchface on the watch and
not only in Gimp which makes a huge difference.

Yes this time I'm not using Inkscape, we are not friends anymore </3


2025.03.01 Sat 13:10 Done properly, design is a heroic act
----------------------------------------------------------

... done properly.

I was thinking about design for a month now.  Initially I wanted to
have 3 rows of big numbers.  I liked the idea of 9 digits because I
could display text PEBBLE, and 3 rows play very nice with the quick
view popup as only the last bottom row disappear.

	.-------.    .-------.    .-------. 
	|  1 2  |    |  P E  |    |  1 2  |
	|  3 4  |    |  B B  |    |__3_4__|
	|  5 6  |    |  L E  |    | popup |
	'-------'    '-------'    '-------' 

But no matter what I tried the design just didn't worked.  The number
where way to small, they where either stretched or a lot of the screen
space was not used.  This wasn't the look and feel I'm going for.

I want my font to feel bold and heavy.  Then I realized that to make
font feel bigger I need the contrast.  I have to put big numbers next
to very small font.  With that the feeling of size will be emphasized.

![First test of design on Pebble time](picture/01.jpeg)

It took me long time to figure out the pixel perfect grid that would
fit 2 rows of big numbers that are build from repeated modular pieces
of the same size, and with third row for smaller moonspace font that
would fix perfectly from left to right and all margins between big and
small font and screen borders are the same.  So here is the recipe for
my perfect grid:

- 5px margin from screen borders which is as minimal as it can be to
  avoid getting too close to screen rounded corners
- 5px of space between first, second and third rows of text
- 70px of height for first 2 rows
- 5x5px blocks are used to build big numbers
- 5px of space between big number lines
- 8px of height for last row
- 134px of width in each row which is perfect for small monospace font
  in third row making it possible to fix exactly 17 characters that are
  6px wide and have 2px space between letters

It's perfect.  I had similar design at some point that looked almost
exactly the same.  But spaces between number, margins and letters where
very different.  With this new grid fonts feel more solid.  It's all
about the feeling.

> Don't think, feel! It is like a finger pointing away to the moon.
> Don't concentrate on the finger or you will miss all that heavenly
> glory.  Do you understand? - Bruce Lee


2025.03.01 Sat 16:32 Glyphs v1
------------------------------

I have first version of glyphs for both fonts.  The main big font has
only numbers and it's a proportional font.  The small monospace font
has uppercase letters, numbers and few special characters.

![First version of glyphs for both fonts](picture/02.png)

I'm not happy about few letters, mostly `M` and `W` but I want to do
some programming now.


2025.03.01 Sat 20:53 First tryyyyy!
-----------------------------------

Well, that was fast.  I just implemented the most basic version of
watchface using glyphs atlas.  Both fonts works as expected.

![First working version of watchface on Pebble Time](picture/03.jpeg)

Time for break.  Here is a rough list of things to do next:

1. Redesign number `0`, it's not heavy enough.
2. Settings, custom background and foreground color, data format etc.
3. Background reflecting battery status.
4. Refactor, use multiple layers, avoid numbers.
5. Animations.
6. Icon.


2025.03.01 Sat 20:53 Now, it is balanced
----------------------------------------

After using this watchface I realized that empty space on the left is
problematic.  It's there on purpose, give a space to breath and it's a
result of decision to make main font non monospace.  Proportional
numbers are more readable.  But I can't explain that to everyone who
see this watchface.  The design must defend itself and right now to
regular user the left side looks strange.  Everything is on the right
side making it heavier.  I need balance.

I tried to add some text using small font but even tho it's small it
was too big for the little space that is on the left edge when clock
shows time like 22:22.  So I decided to try with another, even smaller
font.  And I love it.  This is truly a challenge in typography.  The
tiny font still has some unique character and it's possible to read,
but it's not there for everyday practical reasons.  It's there to
balance the design.

![New design with tiny text on the left edge](picture/04.png)
![New design running on Pebble Time](picture/05.jpeg)

Now I'm satisfied with the composition.  I can proceed to work on
settings and refactor.


2025.03.03 Mon 18:11 Illusion of choice
---------------------------------------

I've made few improvements to fonts glyphs.  Code was heavily
refactored.  Watchface has now multiple layers.  But most importantly
I added configurations.  User can choose colors, date formats and
vibrations for Bluetooth disconnect event, Bluetooth connect event and
on each hour.

Another addition is the second background color showing battery
percent.  I tried many design ideas.  I rly liked my explorations of
different background color just under the big numbers.  But even tho I
liked that in Gimp, it didn't look good at all on watch.  So I ended
up with most basic solution.  I think that it looks excellent.

![Rectangle in background as battery percentage](picture/06.jpeg)


2025.03.03 Mon 18:59 I lied!
----------------------------

The battery level percent as background color was a short lived
feature.  No matter what color combination I tried I wasn't super
happy about it.  And the dithering on BW displays was messing with the
small fonts.  I tried to add background and border to those small
fonts to solve this issue but then watchface looked super ugly.

With that, I decided to terminate this feature.  But it will be back
as text in either left or bottom text area.


2025.03.03 Mon 20:20
--------------------

I really like how the tiny font looks like.  I took few better
pictures of white on black color combination with custom texts.

![Very close photo of tiny font](picture/07.png)
![High resolution photo of white on black font](picture/08.png)


2025.03.04 Tue 19:44 Everybody stay calm! OMG it's happening!
-------------------------------------------------------------

It took a longer while but I made it.  I had most done on day one.
But as usual the finishing touches take longer than quick gains.
Today I had to rewrite most of the code from bitmap rendering to
framebuffer rendering to support dithering in my fonts.  I needed that
to support quick view (unobstructed / popup).

![Photo of watchface with dithering effect](picture/09.png)

I also have all customisation in place.  Custom content for side and
bottom texts, colors and vibrations.  I'm ready for firs release.  I
need screenshots for app store page.

![How quick view looks like](picture/16.jpg)


2025.03.05 Wed 15:24 Ending has not yet been written
----------------------------------------------------

First version is out.  But this is not the end.  There is a problem
with settings on Aplite pebbles.  Nothing happens when user clicks the
submit button in [Clay][] web form.  I debugged this for hours but
found nothing.  There is only a generic error log from `pkjs`.

	[11:23:40] pkjs> Failed to send config data!
	[11:23:40] pkjs> {"data":{"transactionId":2}}

I found what print those logs in Clay source code.  It's nothing special.

	// Send settings to Pebble watchapp
	Pebble.sendAppMessage(self.getSettings(e.response), function() {
	   console.log('Sent config data to Pebble');
	}, function(error) {
	   console.log('Failed to send config data!');
	   console.log(JSON.stringify(error));
	});

I tried to find implementation of `Pebble.sendAppMessage`.  I searched
Pebble SDK and Pebble OS.  Nothing.  Then I thought that there is
something strange in my Clay configuration.  But after reducing it to
just a single toggle button and submit button I had the same results.
It works on all targets except for Aplite.  Last thing that I did was
to search for few very popular watchfaces that use Clay.  I read their
source code and found nothing.  It's all the same.

At this point I don't know what else I could try.  On Discord someone
asked me about my Pebble build tools suggesting that this can make a
difference.  OFC it can.  So maybe the last thing that I can try is to
use some of the prepared dev environments like VM.  I would hate to do
that tho.  I can also try to compile someones watchfaces and test them
to verify my local setup.  Eh...

Second "problem" is that the Brutal watchface don't support Diorite
target, the Pebble Round.  I rly tried to do a design for it earlier.
But the main numbers don't even fit the screen.  But 2 people already
asked me about it.  So I decided to give it a good second try.  Now I
have a complete design language.  I could use tricks from Quick View
layout in Pebble Round.  We will see.

Last smallest issue is with the App Store images.  They are not up to
date and don't demonstrate watchface as well as they could.  I will
fix that first because I know how to do it.

BTW I liked the dithering pattern visible in Quick View so much that I
found a way to include it in main view.  I called it the "Shadow"!
It's totally optional and it's strength can be adjusted.

![How quick view looks like](picture/19.jpg)


2025.03.07 Fri 11:46 Why Are We Still Here? Just To Suffer?
-----------------------------------------------------------

In last devlog I talked about an issue with configuration page in
Aplite platform.  One might think that somewhere there was just a
simple mistake that was difficult to spot.  But not this time.  This
time I was going to suffer hours of debugging and another hours of
rewriting font rendering ... again (-_- ).

I debugged this in so many way.  I even got help from @NiVZ user.  He
helped confirm that there was nothing wrong with my local build tools.
So eventually I created new super simple watchface with basic Clay
configuration.  And it worked with Aplite.  So I kept adding code from
Brutal source to that debug project until Clay configuration it was
broken.  Eventually I found it.

It was the resource bitmap image with font glyphs.  It was just too
fat.  Took to much space.  So now I was on quest to reduce size of it.

This is what I had at that time, the 256 x 256 px 1-bit bitmap image:

![1-bit bitmap image with glyphs](picture/36.png)

At first I thought that maybe it's enough to remove empty space and
pack glyphs as tight as possible.  I did that but it was not enough.
Basic observation shown that big font is the problem.  Single glyph
from that font takes as much space as it is needed for all glyphs from
small and tiny fonts together.  But how can I reduce size of that
font?  I can't just compress it and then decompress later as it will
still take the same heap memory.  I also can't use vectors for it
because it will not work with my custom dithering.  What to do?

The big font is very modular.  It's build from 5x5 px blocks that are
mostly the same: full block, empty block and one diagonal with 3
orientations.  Initially I wanted to encode information about those 3
blocks.  But closer inspection shown that encoding itself would take
almost the same space as current regular bitmap that is stored in
1-bit pixel format.  And then I saw it.  This font can be stored in
bitmap as it was but 5 times smaller.  Then when rendering I just have
to scale it up 5 times.  45 degree edges looked tricky but information
about where those edges are is still in this miniature version.  I
stored small tiles for each of those 45 degree edges and repainted
them when glyphs are scaled.

This is how bitmap image looked after drawing big font 5 times smaller
and rearranging glyphs to not waste any space:

![Packed bitmap image](picture/37.png)

Tiles for 45 deg edges are next to big number 4 and one full tile
below small digit 9.  With just those the big font is restored.  I
still have to reserve runtime memory that will hold one scaled up
glyph from big font but it's not nearly as expensive as the original
bitmap.

And OFC this is more heavy on CPU but still, thanks to all of that I
was able to fix memory issue in Aplite and thanks to that the Clay
configuration works.

2025.03.07 Fri 13:37 Choose what is right or what is easy
---------------------------------------------------------

No I can focus on some easy parts like battery charge percent.  This
is interesting as I want to have it as flexible as possible.  So it
would be the best to just extend syntax of Date format.  Looks like
the strftime() already tool all alphabet letters.  So I will use
different prefix.  I'm going for `#` as it is not even displayed in
the tiny font.  It still can be used when it's fallowed by lowercase
letter.  And fonts have only uppercase letters so it allows user to
use `#` if they want in the bottom text.

	#bottom_text	-> #bottom_text
	#Bottom_text	-> 80ottom_text
	Battery: #B	-> Battery: 85
	Battery: #B%%	-> Battery: 85%

I can now add more custom values.  I did the same thing with daily
steps counter.  I can add more like heart rate monitor but I think
simple weather stuff would be better.  Basic temperature would be
great to have.  I have never done this before.  From my experience of
using other watchfaces it's usually not so great.  I could never trust
it and I was always double checking the weather with second source.
That's why I'm not convinced.

The right thing to do now would be to adapt the design to Pebble Time
Round.  I tried this before few times.  Most recently 2 days ago.
It's very difficult.  Sure you can fit all numbers and text on round
display.  But it looks terrible.  Too many sharp corners.  And big
font is build from 5x5 px blocks making it impossible to center.
After doing some research I found that most watchfaces just center
everything which makes sense.  Fever designs tries to not do that.
Looks like I'm forced to be creative again.


[Rebble]: http://rebble.io/
[Hackathon #002]: https://rebble.io/hackathon-002/
[Installing the Pebble SDK]: https://help.rebble.io/sdk/
[Writing a Pebble app in 2020]: https://willow.systems/blog/pebble-sdk-installation-guide/
[Pebble development documentation]: https://developer.rebble.io/developer.pebble.com/index.html
[Clay]: https://github.com/pebble/clay
[PebbleOS]: https://github.com/google/Pebble
[Rebble Developer Portal]: https://dev-portal.rebble.io/
[BRUTAL - aplite]: https://apps.rebble.io/en_US/application/67c751c6d2acb30009a3c812?hardware=aplite
[BRUTAL - basalt]: https://apps.rebble.io/en_US/application/67c751c6d2acb30009a3c812?hardware=basalt
[BRUTAL - diorite]: https://apps.rebble.io/en_US/application/67c751c6d2acb30009a3c812?hardware=diorite
