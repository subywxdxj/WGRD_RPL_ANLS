#define NOMINMAX

#include <string>
#include <vector>
#include <iostream>
#include <filesystem>

#include <fstream>

#include <Windows.h>


struct DeckMatch
{
    std::string name;//name
    std::string id;//id
    std::string DeckName;
    std::string DeckContent;
};

struct NAME_PID
{
    std::string name;
    std::string pid;
};

struct player
{
    std::vector<std::pair<std::string, std::string>> decks;
    std::string name;
    std::string pid;
    int count = 1;
};

struct playerTarget
{
    std::string name;
    std::string pid;
    std::string deckName;
    std::string deckContent;
    int id = -1;//in case of multiple targets in same replay
};

void GetDesktopResolution(int& horizontal, int& vertical)
{
    RECT desktop;
    // Get a handle to the desktop window
    const HWND hDesktop = GetDesktopWindow();
    // Get the size of screen to the variable desktop
    GetWindowRect(hDesktop, &desktop);
    // The top left corner will have coordinates (0,0)
    // and the bottom right corner will have coordinates
    // (horizontal, vertical)
    horizontal = desktop.right;
    vertical = desktop.bottom;
}

int main()
{
   HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

   int horizontal = 0;
   int vertical = 0;
   GetDesktopResolution(horizontal, vertical);

   HWND hWnd = GetConsoleWindow();

   MoveWindow(hWnd, 0, 0, horizontal / 5 * 2, vertical - (vertical / 33), TRUE);

   SetConsoleOutputCP(CP_UTF8);

   //settings
   std::string showalldecks = "n";
   int mingames = 1;
   std::string compareAll = "n";

   const std::string Settings = "WG_SET.txt";//path to settings file
   std::string replayPath;//path to replays (stored in settings file)

   std::ifstream fileSettings;
   fileSettings.open(Settings);

   std::string text;
   if (!std::getline(fileSettings, text))
   {
       std::cout << "\n[ERROR] - Settings file not found";
       Sleep(4000);
       return 0;
   }
   std::cout << "Settings:\n" << text;

   //show all decks
   int scb = text.find("[");
   int scc;
   showalldecks = text.substr(scb + 1, 1);
   std::getline(fileSettings, text);

   //minimal games
   scb = text.find("[");
   scc = text.find("]");
   mingames = stoi(text.substr(scb + 1, scc - scb - 1));
   std::cout << "\n" << text;

   //compare decks
   std::getline(fileSettings, text);
   scb = text.find("[");
   compareAll = text.substr(scb + 1, 1);
   std::cout << "\n" << text;

   //replay files path
   std::getline(fileSettings, text);
   scb = text.find("[");
   scc = text.find("]");
   replayPath = text.substr(scb + 1, scc - scb - 1);
   std::cout << "\n" << text;

   std::cout << "\n\n";
   fileSettings.close();

   int restart = 1;
   while (restart)
   {
       std::string name = "";
       std::string server = "";
       std::string pid = "n";

        system("cls");

        std::cout << "Find player by name (name/n): ";//rip trying to find player with nickname "n"
        while (name == "")
        {
            std::getline(std::cin, name);
        }

        if (name == "n")
        {
            std::cout << "Find player by id (id/n): ";
            std::cin >> pid;
        }
        std::cout << "[autosearch] Find server by name (name/n) : ";
        while (server == "")
        {
            std::getline(std::cin, server);
        }
        std::transform(server.begin(), server.end(), server.begin(), ::tolower);//transform server name input to lower case for more general comparison


        std::string WGfile = "wargamerpl2";

        std::vector<player> players;//name, id, count


        std::vector<std::string> replayNames;
        std::vector<playerTarget> PlayersTarget;
        std::vector<std::string> serverNames;
        std::vector<std::string> GameInfoV;

        for (const auto& replayFullPath : std::filesystem::directory_iterator(replayPath))
        {
            std::vector<std::pair<std::string, std::string>> DecksReplay;
            std::vector<NAME_PID> playersReplay;//players from current replay
            std::pair<std::string, std::string> TargetDeck;//Deck name + Deck
            std::string serverName;
            std::string GameInfo;

            std::string replayName {replayFullPath.path().u8string()};

            int pos = replayName.find(".");
            std::string file = replayName.substr(pos + 1);




            bool abort = false;//abort adding player to playerlist

            if (file == WGfile)
            {
                std::ifstream fileRead;
                fileRead.open(replayName, std::ios::binary);

                char ch = 0;
                std::string FileStr = "";
                int iter = 0;
                int a, b;
                int maxReadSize = std::min((uint64_t)12000, std::filesystem::file_size(replayFullPath));//in case file size is lower than average 10v10 game replay header

                while (iter < maxReadSize && fileRead >> std::noskipws >> ch)//load the whole file header (under 12k bytes)
                {
                    FileStr += ch;
                    iter++;
                }
                fileRead.close();


                //quick general checks
                if (name != "n")
                {
                    if (FileStr.find(name) > 12000) { continue; }//skip if no name match in file header (!FileStr.find(name) doesn't work coz result value is not allocated if no match???)
                }
                if (pid != "n")
                {
                    if (FileStr.find(pid) > 12000) { continue; }//skip if no pid match in file header
                }

                int snS = FileStr.find("\",\"ServerName\":\"");
                if (snS == -1)//ignore offline games (no server name)
                {
                    continue;
                }
                int snE = FileStr.find("\",\"", snS + 16);
                serverName = FileStr.substr(snS + 16, snE - (snS + 16));
                

                if (server != "n")
                {
                    std::string serverNameLower = serverName;
                    std::transform(serverNameLower.begin(), serverNameLower.end(), serverNameLower.begin(), ::tolower);//transform server name to lower case aswell

                    if (serverNameLower.find(server) > 100) { continue; }//skip if no serverName match in serverName
                }

                int start = 0;

                bool foundTarget = false;

                while (FileStr.find("\"PlayerUserId\":\"", start) != -1)
                {
                    int puidS = FileStr.find("\"PlayerUserId\":\"", start);
                    int puidE = FileStr.find("\",\"", puidS + 16);
                    std::string playerUID = FileStr.substr(puidS + 16, puidE - (puidS + 16));//extract PUID

                    int pnS = FileStr.find("\"PlayerName\":\"", start);
                    int pnE = FileStr.find("\",\"", pnS + 14);
                    std::string playerName = FileStr.substr(pnS + 14, pnE - (pnS + 14));//extract player name


                    if (playerUID == "18446744073709551615")//took me a while to understand its not a bug
                    {
                        playerName = "Bot";
                    }

                    NAME_PID transf0;
                    transf0.name = playerName;
                    transf0.pid = playerUID;
                    playersReplay.push_back(transf0);//save players from current replay


                    int dnS = FileStr.find("\",\"PlayerDeckName\":\"", pnE);
                    int dnE = FileStr.find("\",\"", dnS + 20);

                    TargetDeck.first = FileStr.substr(dnS + 20, dnE - (dnS + 20));//get deck name

                    int dS = FileStr.find("\",\"PlayerDeckContent\":\"", pnE);
                    int dE = FileStr.find("\",\"", dS + 23);

                    TargetDeck.second = FileStr.substr(dS + 23, dE - (dS + 23));//get deck content

                    DecksReplay.push_back(std::pair(TargetDeck.first, TargetDeck.second));//save current replay decks (same order as players)


                    start = dE;
                    if (name != "n")//search by name
                    {
                        if (playerName.find(name, 0) < 100)//add to target list if name matches
                        {
                            playerTarget buffer;
                            buffer.name = playerName;
                            buffer.pid = playerUID;

                            buffer.deckName = TargetDeck.first;
                            buffer.deckContent = TargetDeck.second;

                            buffer.id = replayNames.size();
                            PlayersTarget.push_back(buffer);
                            foundTarget = true;
                        }
                    }
                    else if (pid != "n")//search by pid
                    {
                        if (playerUID.find(pid) < 100)//add to target list if pid matches
                        {
                            playerTarget buffer;
                            buffer.name = playerName;
                            buffer.pid = playerUID;

                            buffer.deckName = TargetDeck.first;
                            buffer.deckContent = TargetDeck.second;

                            buffer.id = replayNames.size();
                            PlayersTarget.push_back(buffer);
                            foundTarget = true;
                        }
                    }
                }

                if (foundTarget || (pid == "n" && name == "n"))//save replay data if the replay meets the requirements
                {
                    for (int r = 0; r < playersReplay.size(); r++)//update player list with players from current list
                    {
                        bool newPlayer = true;
                        for (int t = 0; t < players.size(); t++)//check for copies
                        {
                            if (players[t].pid == playersReplay[r].pid)//if pid already exists
                            {
                                newPlayer = false;
                                players[t].count++;//count++

                                bool newDeck = true;
                                for (int deckN = 0; deckN < players[t].decks.size(); deckN++)//check if the deck is new
                                {
                                    if (DecksReplay[r].second == players[t].decks[deckN].second)
                                    {
                                        newDeck = false;
                                    }
                                }
                                if (newDeck)
                                {
                                    players[t].decks.push_back(DecksReplay[r]);
                                }
                                break;
                            }
                        }
                        if (newPlayer)
                        {
                            player transf;
                            transf.decks.push_back(DecksReplay[r]);//save first deck
                            transf.pid = playersReplay[r].pid;
                            transf.name = playersReplay[r].name;
                            transf.count = 1;

                            players.push_back(transf);//save to global player list
                        }
                    }
                    int giS = FileStr.find("\",\"Map\":\"");
                    int giE = FileStr.find("\",\"", giS + 9);

                    GameInfo = FileStr.substr(giS + 9, giE - (giS + 9));


                    GameInfoV.push_back(GameInfo);
                    serverNames.push_back(serverName);
                    replayNames.push_back(replayName);//save replay name
                }
                

            }
        }
        SetConsoleTextAttribute(hConsole, 2);
        std::cout << "\n\n";

        for (int i = 0; i < replayNames.size(); i++)//Full game info + selected player (and his deck)
        {
            std::cout << "\n";
            SetConsoleTextAttribute(hConsole, 6);
            if (PlayersTarget.size() != 0)
            {
                for (int j = i; j < PlayersTarget.size(); j++)//in case of mulitple targets in same replay
                {
                    if (PlayersTarget[j].id == i)
                    {
                        std::cout << "\nPlayer: ";
                        SetConsoleTextAttribute(hConsole, 2);
                        std::cout << PlayersTarget[j].name;
                        SetConsoleTextAttribute(hConsole, 6);
                        std::cout << "\nPID: " << PlayersTarget[j].pid << "\nDeck: ";
                        SetConsoleTextAttribute(hConsole, 11);
                        std::cout << PlayersTarget[j].deckName;
                        SetConsoleTextAttribute(hConsole, 6);
                        std::cout << ": " << PlayersTarget[j].deckContent;
                    }
                }
            }
            SetConsoleTextAttribute(hConsole, 5);
            std::cout << "\nServer: " << serverNames[i] << "\nMAP: " << GameInfoV[i];
            SetConsoleTextAttribute(hConsole, 2);
            std::cout << "\n" << i + 1 << " " << replayNames[i];
        }
        SetConsoleTextAttribute(hConsole, 4);
        std::cout << "\n\nPLAYERS (" << players.size() << ")\nTARGETS (" << PlayersTarget.size() << ") FROM THE GAMES (" << replayNames.size() << ")\n";

        auto compCount = [](player a, player b)
        {
            return a.count > b.count;
        };
        sort(players.begin(), players.end(), compCount);

        for (int i = 0; i < players.size(); i++)
        {
            if (players[i].count < mingames)
            {
                continue;
            }
            SetConsoleTextAttribute(hConsole, 6);
            std::cout << "\n\n" << i << " PlayerName: ";
            SetConsoleTextAttribute(hConsole, 2);
            std::cout << players[i].name;
            SetConsoleTextAttribute(hConsole, 6);
            std::cout << "    PID: " << players[i].pid << " Count: ";
            SetConsoleTextAttribute(hConsole, 2);
            std::cout << players[i].count;

            if (showalldecks == "y")
            {
                SetConsoleTextAttribute(hConsole, 6);//Show decks
                for (int j = 0; j < players[i].decks.size(); j++)
                {
                    std::cout << "\nDeck: ";
                    SetConsoleTextAttribute(hConsole, 11);

                    std::cout << players[i].decks[j].first;
                    SetConsoleTextAttribute(hConsole, 6);
                    std::cout << ": " << players[i].decks[j].second;

                    if (compareAll == "y")//looking for matches in deck name/content from current list of replays
                    {
                        std::vector<DeckMatch> SameDeckName;
                        std::vector<DeckMatch> SameDeckCont;

                        for (int ii = 0; ii < players.size(); ii++)//simple compare deck name/content
                        {
                            for (int ij = 0; ij < players[ii].decks.size(); ij++)
                            {
                                if (i != ii)
                                {
                                    if (players[ii].decks[ij].first == players[i].decks[j].first)//found identical deck name
                                    {

                                        DeckMatch deck;
                                        deck.name = players[ii].name;
                                        deck.id = players[ii].pid;
                                        deck.DeckName = players[ii].decks[ij].first;
                                        deck.DeckContent = players[ii].decks[ij].second;

                                        SameDeckName.push_back(deck);//save info
                                        //Every comparison match will be replicated at least twice (on both ends)
                                    }
                                    if (players[ii].decks[ij].second == players[i].decks[j].second)//found identical deck content
                                    {
                                        DeckMatch deck;
                                        deck.name = players[ii].name;
                                        deck.id = players[ii].pid;
                                        deck.DeckName = players[ii].decks[ij].first;
                                        deck.DeckContent = players[ii].decks[ij].second;

                                        SameDeckCont.push_back(deck);//save info
                                        //Every comparison will be replicated potentially multiple times
                                    }
                                }
                            }

                        }


                        if (SameDeckName.size() > 0)
                        {
                            SetConsoleTextAttribute(hConsole, 20);
                            std::cout << "\nDeck name match\n";
                            for (int l = 0; l < SameDeckName.size(); l++)
                            {
                                SetConsoleTextAttribute(hConsole, 4);
                                std::cout << "\n" << SameDeckName[l].name << " (" << SameDeckName[l].id << ") ";
                                SetConsoleTextAttribute(hConsole, 6);
                                std::cout << SameDeckName[l].DeckContent;
                            }
                        }
                        if (SameDeckCont.size() > 0)
                        {
                            SetConsoleTextAttribute(hConsole, 84);
                            std::cout << "\nDeck content match\n";
                            for (int l = 0; l < SameDeckCont.size(); l++)
                            {
                                SetConsoleTextAttribute(hConsole, 4);
                                std::cout << "\n" << SameDeckCont[l].name << " (" << SameDeckCont[l].id << ") ";
                                SetConsoleTextAttribute(hConsole, 6);
                                std::cout << SameDeckCont[l].DeckName;
                            }
                        }
                    }
                }
            }
        }
        SetConsoleTextAttribute(hConsole, 7);
        std::cout << "\n\ninput \"1\" to restart, \"0\" to exit: ";
        std::cin >> restart;
    }
}