#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <stdio.h>
#include <vector>
#include <memory>
#include <algorithm>
#include <numeric>
#include <string>
#include <set>
#include <tuple>
#include <functional>
#include <cstring>
#include <deque>
#include <array>
#include <map>
#include <random>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <ranges>

#include "string_vector.h"

using namespace std;
using namespace std::string_literals;

#define FOR(var, max_value) for(int var=0;var<max_value;++var)


char buf[1000];

struct cPlayerResult
{
    int game;
    int match;
    int player;

    int position;
    bool isFirstPlay;
    bool isLastPosition;
};
vector<cPlayerResult> playerResults;

struct cPlayer
{
	string name;
    vector<int> results;
	cPlayer(string name): name(name) {}
};
vector<cPlayer> players;

struct cGame
{
	string name;
	int played = 0;
};
vector<cGame> games;

struct cMatch
{
	int game;
	vector<int> results; 
};

vector<cMatch> matches;

int longest_game_name = 0;
FILE* out = nullptr;

void printGameNameHeading(int game)
{
    fprintf(out, "%s%.*s", games[game].name.c_str(), longest_game_name - (int)games[game].name.length() + 1, ".......................................");
}

void printMatchCountByGame()
{
    fprintf(out, "Number of matches: %d\n", (int)matches.size());
    fprintf(out, "-------------------------\n\nMatch count by game:\n\n");
    vector<pair<int, int>> games_played;
    for(int i=0;i<games.size();++i)
    {
        auto& game = games[i];
        games_played.emplace_back(i, game.played);
        longest_game_name = max(longest_game_name, (int)game.name.length());
    }
    sort(games_played.begin(), games_played.end(), [](auto& a, auto& b) { return a.second > b.second; });
    for (auto& p : games_played)
    {
        printGameNameHeading(p.first);
        fprintf(out, ": %d\n", p.second);
    }
    fprintf(out, "\n-------------------------------------------------------------\n\n\n");
}

struct cGamePositionStat
{
    int firstCount = 0;
    int lastCount = 0;
    double score = 0.0;
    int plays = 0;
    int game;
};

void printGamesByPlayer()
{
    fprintf(out, "-=#=- Player statistics -=#=-\n\n");
    for (int i = 0; i < players.size(); ++i)
    {
        vector<cGamePositionStat> positionStats(games.size());
        for (int i = 0; i < games.size(); ++i)
            positionStats[i].game = i;
        for (auto resultIndex : players[i].results)
        {
            auto& result = playerResults[resultIndex];
            auto& stat = positionStats[result.game];
            auto& match = matches[result.match];

            ++stat.plays;
            if (result.position == 1)
                ++stat.firstCount;
            if (result.isLastPosition)
                ++stat.lastCount;

            stat.score += static_cast<int>(match.results.size() + 1) - result.position;
        }

        sort(positionStats.begin(), positionStats.end(), [](auto& l, auto& r) { return l.plays > r.plays; });
        positionStats.erase(remove_if(positionStats.begin(), positionStats.end(), [](auto& stat) { return stat.plays == 0; }), positionStats.end());
        fprintf(out, "--- %s ---\n", players[i].name.c_str());
        for (auto& stat : positionStats)
        {
            printGameNameHeading(stat.game);
            fprintf(out, "  played: %2d first: %2d (%3d%%), last: %2d (%3d%%), avg. score: %.2f\n",
                stat.plays,
                stat.firstCount, stat.firstCount * 100 / stat.plays,
                stat.lastCount, stat.lastCount * 100 / stat.plays,
                stat.score / stat.plays);
        }
        fprintf(out, "\n\n");

    }
}

void uprising();

int main()
{
    uprising();

	auto f = fopen("games.csv", "r");
    out = fopen("stats.txt", "wt");
	fgets(buf, 1000, f);
	cStringVector header(buf, ",\n", false);
	players.reserve(header.size()-1);
	for(int i = 2; i<header.size(); ++i)  // first two coloumns: game, date
	{
		players.emplace_back(header[i]);
	}
	int match_count = 0;
	while(!feof(f))
	{
		fgets(buf, 1000, f);
		++match_count;
		cStringVector parts(buf, ",\n", true);
		auto gameIt = find_if(games.begin(), games.end(), [name = parts[0]](auto& g) { return g.name==name; });
		if(gameIt ==games.end())
		{
			games.emplace_back();
			games.back().name = parts[0];
            gameIt = games.begin() + games.size() - 1;
		}
		cGame& game = *gameIt;
        int gameIndex = (int)(gameIt - games.begin());
		++game.played;

        matches.emplace_back();
        matches.back().game = gameIndex;

        int lastPosition = 1, playerCount = 0;
        for (int playerIndex = 0; playerIndex < players.size(); ++playerIndex)
        {
            auto& posText = parts[playerIndex + 2];
            if (!posText.empty())
            {
                ++playerCount;
                cPlayerResult result;
                result.player = playerIndex;
                result.game = gameIndex;
                result.match = match_count - 1;
                result.position = posText[0] - '0';
                result.isFirstPlay = posText.size() > 1 && posText[1]=='f';
                lastPosition = max(lastPosition, result.position);
                playerResults.emplace_back(result);
                int resultIndex = (int)(playerResults.size() - 1);
                players[playerIndex].results.emplace_back(resultIndex);
                matches.back().results.emplace_back(resultIndex);
            }
        }
        for (int i = 0; i < playerCount; ++i)
        {
            auto& result = playerResults[playerResults.size() - 1 - i];
            result.isLastPosition = result.position == lastPosition;
        }
	}

    printMatchCountByGame();
    printGamesByPlayer();
//    printPlayerVSPlayerStats();
    fclose(out);
}