"Wargame: Red Dragon" replay analysis tool<br /><br />

Usage: <br />
1) Put .exe in the same folder as WG_SET.txt
2) In WG_SET.txt replace path to your replay folder and optionally change other settings in yes-[y]/no-[n] format
<br />
General Information:<br />
This is a console application for sorting data from replay files made for Windows<br />
Game replays contain a lot of potentially interesting information such as player names/decks/deck names
<br /><br />
Personal notes:<br />
It's an old project and at the time it went quite a bit further than I initially anticipated,<br />
which resulted in it being huge blob of barely cohesive code.<br />
I don't feel like rewriting it from the scratch and there's only so much you can do with information from the replays.<br />
Only possible update is adding more detailed analysis of decks which are basically base64 encrypted array of unit ID's,<br />
but I'm not sure wether will I do that.
