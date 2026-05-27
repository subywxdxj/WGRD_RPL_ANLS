# "Wargame: Red Dragon" replay analysis tool<br />
![image alt](https://github.com/subywxdxj/WGRD_RPL_ANLS/blob/c3f55be11bb733201596122a328ee4d441d4ceed/tab1.png)
# Usage: <br />
0) Both .exe (application) and .txt (settings) are avaliable in latest release ->
1) Put .exe in the same folder as WG_SET.txt
2) In WG_SET.txt replace path to your replay folder and optionally change other settings in yes-[y]/no-[n] format
3) After application is succesfully lauched input requested data for search or 'n' to ignore
<br />


# Compilation:
- `cd WG_RPL_ANLS`<br />
- `mkdir build`<br />
- `cd build`<br />
- `cmake ..`<br /><br />
# General Information:<br />
This is a console application for sorting data from replay files made for Windows.<br />
Game replays contain a lot of potentially interesting information such as player names/id's/decks/deck names.<br /><br />
Console output consists of two sections:<br />
1) Replays that passed the filters
2) All the players from those replays ("count" represents amount of times player is present in the list of filtered replays)
<br /><br />
# Personal notes:<br />
It's an old project and at the time it went quite a bit further than I initially anticipated,<br />
which resulted in it being huge blob of barely cohesive code.<br />
I don't feel like rewriting it from the scratch and there's only so much you can do with information from the replays.<br />
Only possible update (besides small quality updates) is adding more detailed analysis of decks which are basically base64 encoded array of unit ID's,<br />
