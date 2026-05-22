#define NOMINMAX

#include <string>
#include <vector>
#include <iostream>
#include <filesystem>
#include <stdio.h>

#include <fstream>

#include <locale>

#include <Windows.h>


struct DeckMatch
{
    std::wstring name;//name
    std::wstring id;//id
    std::wstring DeckName;
    std::wstring DeckContent;
};

struct NAME_PID
{
    std::wstring name;
    std::wstring pid;
};

struct player
{
    std::vector<std::pair<std::wstring, std::wstring>> decks;
    std::wstring name;
    std::wstring pid;
    int count;
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

   std::wstring name = L"n";
   std::wstring server = L"n";
   std::wstring pid = L"n";
   std::string showalldecks = "n";
   int mingames = 1;
   std::string compareAll = "n";

   char result[MAX_PATH];
   std::string Settings = "WG_SET.txt";//path to settings file
   std::string replayPath;//path to replays (stored in settings file)

   std::ifstream fileSettings;
   fileSettings.open(Settings);

   wchar_t chS = 0;

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
   mingames = stoi(text.substr(scb + 1, 1));
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
        system("cls");

        std::cout << "Find player by name (name/n): ";
        std::wcin >> name;
        if (name == L"n")
        {
            std::cout << "Find player by id (id/n): ";
            std::wcin >> pid;
        }
        std::cout << "[autosearch] Find server by name (name/n) : ";

        std::wcin >> server;



        std::string WGfile = "wargamerpl2";

        std::vector<player> players;//name, id, count


        std::vector<std::string> replayNames;
        std::vector<std::pair<std::wstring, std::wstring>> PlayersCompare1;
        std::vector<std::wstring> serverNames;
        std::vector<std::wstring> GameInfoV;
        std::vector<std::pair<std::wstring, std::wstring>> TargetDecks;

        for (const auto& replayFullPath : std::filesystem::directory_iterator(replayPath))
        {
            std::vector<std::pair<std::wstring, std::wstring>> DecksReplay;
            std::vector<NAME_PID> playersReplay;//players from current replay
            std::pair<std::wstring, std::wstring> TargetDeck;//Deck name + Deck
            std::wstring serverName;
            std::wstring GameInfo;

            std::string replayName {replayFullPath.path().u8string()};

            int pos = replayName.find(".");
            std::string file = replayName.substr(pos + 1);




            bool abort = false;

            if (file == WGfile)
            {
                std::wifstream fileRead;
                fileRead.open(replayName, std::ios::binary);

                wchar_t ch = 0;
                std::wstring FileStr = L"";
                int iter = 0;
                int a, b;
                int maxReadSize = std::min((uint64_t)12000, std::filesystem::file_size(replayFullPath));//in case file size is lower than average 10v10 game replay header

                while (iter < maxReadSize && fileRead >> std::noskipws >> ch)//load the whole file header (under 12k bytes)
                {
                    FileStr += ch;
                    iter++;
                }
                fileRead.close();


                if (name != L"n")
                {
                    if (!FileStr.find(name)) { continue; }
                }
                if (pid != L"n")
                {
                    if (!FileStr.find(pid)) { continue; }
                }
                if (server != L"n")
                {
                    continue;
                }

                int snS = FileStr.find(L"\",\"ServerName\":\"");
                if (snS == -1)//ignore offline games (no server name)
                {
                    continue;
                }
                int snE = FileStr.find(L"\",\"", snS + 16);

                serverName = FileStr.substr(snS + 16, snE - (snS + 16));


                for (int j = 0; j < serverName.size(); j++)
                {
                    for (int i = 0; i < server.size(); i++)
                    {
                        if (tolower(server[i]) != tolower(serverName[j + i]))
                        {
                            break;
                        }
                        else if (i == server.size() - 1)
                        {
                            abort = false;
                        }
                    }
                }

                abort = true;
                int start = 0;

                while (FileStr.find(L"\"PlayerName\":\"", start) != -1)
                {
                    int pnS = FileStr.find(L"\"PlayerName\":\"", start);
                    int pnE = FileStr.find(L"\",\"", pnS + 14);
                    std::wstring playerName = FileStr.substr(pnS + 14, pnE - (pnS + 14));//extract player name

                    int puidS = FileStr.find(L"\"PlayerUserId\":\"", pnS - 160);
                    int puidE = FileStr.find(L"\",\"", puidS + 16);
                    std::wstring playerUID = FileStr.substr(puidS + 16, puidE - (puidS + 16));//extract PUID

                    NAME_PID transf0;
                    transf0.name = playerName;
                    transf0.pid = playerUID;
                    playersReplay.push_back(transf0);//save players from current replay


                    int dnS = FileStr.find(L"\",\"PlayerDeckName\":\"", pnE);
                    int dnE = FileStr.find(L"\",\"", dnS + 20);

                    TargetDeck.first = FileStr.substr(dnS + 20, dnE - (dnS + 20));//get deck name

                    int dS = FileStr.find(L"\",\"PlayerDeckContent\":\"", pnE);
                    int dE = FileStr.find(L"\",\"", dS + 23);

                    TargetDeck.second = FileStr.substr(dS + 23, dE - (dS + 23));//get deck content

                    DecksReplay.push_back(std::pair(TargetDeck.first, TargetDeck.second));//save current replay decks (same order as players)


                    start = pnE;
                    if (name != L"n" && abort)//search by name
                    {
                        for (int j = 0; j < playerName.size(); j++)
                        {
                            for (int i = 0; i < name.size(); i++)
                            {
                                if (tolower(name[i]) != tolower(playerName[j + i]))
                                {
                                    break;
                                }
                                else if (i == name.size() - 1)//found the target
                                {
                                    TargetDecks.push_back(std::pair(TargetDeck.first, TargetDeck.second));
                                    PlayersCompare1.push_back(std::pair(playerName, playerUID));
                                    abort = false;
                                    break;
                                }
                            }
                            if (!abort)
                            {
                                break;//for extra break from double loop to avoid double name results i guess
                            }
                        }
                    }
                    else if (pid != L"n")//search by pid
                    {
                        for (int i = 0; i < pid.size(); i++)
                        {
                            if (tolower(pid[i]) != tolower(playerUID[i]))
                            {
                                break;
                            }
                            else if (i == pid.size() - 1)//found the target
                            {
                                TargetDecks.push_back(std::pair(TargetDeck.first, TargetDeck.second));
                                PlayersCompare1.push_back(std::pair(playerName, playerUID));
                                abort = false;
                                break;
                            }
                        }
                    }
                    else
                    {
                        abort = false;
                    }
                }

                if (!abort)
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
                            transf.count = 0;

                            players.push_back(transf);//save to global player list
                        }
                    }
                    int giS = FileStr.find(L"\",\"Map\":\"");
                    int giE = FileStr.find(L"\",\"", giS + 9);

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
            if (PlayersCompare1.size() != 0)
            {
                std::wcout << "\nPlayer: ";
                SetConsoleTextAttribute(hConsole, 2);
                std::wcout << PlayersCompare1[i].first;
                SetConsoleTextAttribute(hConsole, 6);
                std::wcout << "\nPID: " << PlayersCompare1[i].second << "\nDeck: ";
                SetConsoleTextAttribute(hConsole, 11);
                std::wcout << TargetDecks[i].first;
                SetConsoleTextAttribute(hConsole, 6);
                std::wcout << ": " << TargetDecks[i].second;
            }
            SetConsoleTextAttribute(hConsole, 5);
            std::wcout << "\nServer: " << serverNames[i] << "\nMAP: " << GameInfoV[i];
            SetConsoleTextAttribute(hConsole, 2);
            std::cout << "\n" << i + 1 << " " << replayNames[i];
        }
        SetConsoleTextAttribute(hConsole, 4);
        std::cout << "\n\nPLAYERS (" << players.size() - 1 << ") FROM THE GAMES (" << replayNames.size() << ")\n";

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
            std::wcout << "\n\n" << i << " PlayerName: ";
            SetConsoleTextAttribute(hConsole, 2);
            std::wcout << players[i].name;
            SetConsoleTextAttribute(hConsole, 6);
            std::wcout << "    PID: " << players[i].pid << " Count: ";
            SetConsoleTextAttribute(hConsole, 2);
            std::cout << players[i].count;

            if (showalldecks == "y")
            {
                SetConsoleTextAttribute(hConsole, 6);//Show decks
                for (int j = 0; j < players[i].decks.size(); j++)
                {
                    std::vector<DeckMatch> SameDeckName;
                    std::vector<DeckMatch> SameDeckCont;

                    std::wcout << "\nDeck: ";
                    SetConsoleTextAttribute(hConsole, 11);

                    if (compareAll == "y")//looking for matches is deck name/content from current list of replays
                    {
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
                                        //Every comparison will be replicated potentially multiple times
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
                    }


                    std::wcout << players[i].decks[j].first;
                    SetConsoleTextAttribute(hConsole, 6);
                    std::wcout << ": " << players[i].decks[j].second;

                    if (SameDeckName.size() > 0)
                    {
                        SetConsoleTextAttribute(hConsole, 20);
                        std::wcout << "\nDeck name match\n";
                        for (int l = 0; l < SameDeckName.size(); l++)
                        {
                            SetConsoleTextAttribute(hConsole, 4);
                            std::wcout << "\n" << SameDeckName[l].name << " (" << SameDeckName[l].id << ") ";
                            SetConsoleTextAttribute(hConsole, 6);
                            std::wcout << SameDeckName[l].DeckContent;
                        }
                    }
                    if (SameDeckCont.size() > 0)
                    {
                        SetConsoleTextAttribute(hConsole, 84);
                        std::wcout << "\nDeck content match\n";
                        for (int l = 0; l < SameDeckCont.size(); l++)
                        {
                            SetConsoleTextAttribute(hConsole, 4);
                            std::wcout << "\n" << SameDeckCont[l].name << " (" << SameDeckCont[l].id << ") ";
                            SetConsoleTextAttribute(hConsole, 6);
                            std::wcout << SameDeckCont[l].DeckName;
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