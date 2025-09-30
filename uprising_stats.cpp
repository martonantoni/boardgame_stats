#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <format>
#include <print>

#include "string_vector.h"

namespace
{

struct cGame
{
    struct cPlayerData
    {
        std::string mName;
        std::string mLeader;
        int mPlace;
        int mPoints;
        bool mWorms;
    };
    std::vector<cPlayerData> mPlayers;
    int mTurns;
    int mGamePoints = 0;
};

std::vector<cGame> games;

struct cPerformance
{
    int mPlays;
    int mWins;
    int mLosses;
    int mPoints;
    int mGamePoints;
    [[nodiscard]] std::string toString() const
    {
        return std::format("wins: {:2} ({:3}%), avg. points: {:.2f}",
            mWins, mPlays == 0 ? 0 : mWins * 100 / mPlays, /*mLosses, mPlays == 0 ? 0 : mLosses * 100 / mPlays,*/  mPoints / static_cast<double>(mPlays));
    };
    int winPercentage() const
    {
        return mPlays == 0 ? 0 : mWins * 100 / mPlays;
    }
};


struct cPlayer
{
    cPerformance mPerformance;
    std::array<cPerformance, 3> mPerformanceByPosition;
    std::unordered_map<std::string, cPerformance> mAfterPlayer;
    std::unordered_map<std::string, cPerformance> mLeaders;
};

struct cLeaderPerformance
{
    struct cPositionData
    {
        int mPicked = 0;
        int mWins = 0;
    };
    std::vector<cPositionData> mPositions;
    cLeaderPerformance() : mPositions(4) {}
};

std::unordered_map<std::string, cLeaderPerformance> leaderStats;


std::unordered_map<std::string, cPlayer> players;

}

void printLeadersStats(std::ofstream& out, const std::vector<std::string>& playerNames)
{
    std::unordered_map<std::string, cPerformance> leaders;

    for (auto& game : games)
    {
        for (auto& player : game.mPlayers | std::views::filter([&playerNames](const cGame::cPlayerData& player) { return std::ranges::find(playerNames, player.mName) != playerNames.end(); }))
        {
            auto& leaderData = leaders[player.mLeader];
            leaderData.mPlays++;
            leaderData.mPoints += player.mPoints;
            leaderData.mGamePoints += game.mGamePoints;
            if (player.mPlace == 1)
            {
                leaderData.mWins++;
            }
            else if (player.mPlace == 3)
            {
                leaderData.mLosses++;
            }
        }
    }
    std::vector<std::pair<int, std::string>> sortedLeaders(leaders.size());
    for (auto&& [leaderName, leaderData] : leaders)
    {
//        std::print("{} picked {} times, {}\n", leaderName, leaderData.mPlays, leaderData.toString());
        sortedLeaders.emplace_back(leaderData.winPercentage(), std::format("{:8} picked {:2} times, {}\n", leaderName, leaderData.mPlays, leaderData.toString()));
    }
    std::sort(sortedLeaders.begin(), sortedLeaders.end(), std::greater<>());
    for (auto&& [winPercentage, leaderData] : sortedLeaders)
    {
        std::print(out, "{}", leaderData);
    }
}


void uprising()
{
    std::vector<std::string> lines;
    {
        std::ifstream file("uprising.csv");
        std::string line;
        while (std::getline(file, line))
        {
            lines.push_back(line);
        }
    }
    std::ofstream out("stats_uprising.txt");

    // per player fields: name, leader, place, points, worms

    for(auto& line: lines | std::views::drop(2))
    {
        auto& game = games.emplace_back();
        cStringVector fields(line, ",", true);
        for (int playerIdx = 0; playerIdx < 4; ++playerIdx)
        {
            if(fields[1 + playerIdx * 5].empty())
                break;
            auto& player = game.mPlayers.emplace_back();
            player.mName = fields[1 + playerIdx * 5];
            player.mLeader = fields[2 + playerIdx * 5];
            player.mPlace = std::stoi(fields[3 + playerIdx * 5]);
            player.mPoints = fields[4 + playerIdx * 5].empty() ? 0 : std::stoi(fields[4 + playerIdx * 5]);
            player.mWorms = fields[5 + playerIdx * 5] != "no";  // if field is not set, assume "yes"
            game.mGamePoints += player.mPoints;
        }
        game.mTurns = std::stoi(fields[21]);
    }

    games.erase(std::remove_if(games.begin(), games.end(), [](const cGame& game) 
        { 
            return game.mPlayers.size() != 3 
                || game.mGamePoints == 0
                || std::ranges::any_of(game.mPlayers, [](const cGame::cPlayerData& player) { return player.mName != "Zsolt" && player.mName != "Evi" && player.mName != "Marci"; });
        }), games.end());

    for (auto& game : games)
    {
        for (auto&& [positionIdx, player] : std::views::enumerate(game.mPlayers))
        {
            auto& playerData = players[player.mName];
            playerData.mPerformance.mPlays++;
            playerData.mPerformance.mPoints += player.mPoints;
            playerData.mPerformance.mGamePoints += game.mGamePoints;
            if (player.mPlace == 1)
            {
                playerData.mPerformance.mWins++;
            }
            else if(player.mPlace == 3)
            {
                playerData.mPerformance.mLosses++;
            }
            playerData.mPerformanceByPosition[positionIdx].mPlays++;
            playerData.mPerformanceByPosition[positionIdx].mPoints += player.mPoints;
            playerData.mPerformanceByPosition[positionIdx].mGamePoints += game.mGamePoints;
            if (player.mPlace == 1)
            {
                playerData.mPerformanceByPosition[positionIdx].mWins++;
            }
            else if (player.mPlace == 3)
            {
                playerData.mPerformanceByPosition[positionIdx].mLosses++;
            }
            playerData.mLeaders[player.mLeader].mPlays++;
            playerData.mLeaders[player.mLeader].mPoints += player.mPoints;
            playerData.mLeaders[player.mLeader].mGamePoints += game.mGamePoints;
            if (player.mPlace == 1)
            {
                playerData.mLeaders[player.mLeader].mWins++;
            }
            else if (player.mPlace == 3)
            {
                playerData.mLeaders[player.mLeader].mLosses++;
            }
            leaderStats[player.mLeader].mPositions[positionIdx].mPicked++;
            leaderStats[player.mLeader].mPositions[positionIdx].mWins += player.mPlace == 1 ? 1 : 0;

            auto& previousPlayer = game.mPlayers[(positionIdx + 2) % 3].mName;
            auto& afterPlayerData = playerData.mAfterPlayer[previousPlayer];
            afterPlayerData.mPlays++;
            afterPlayerData.mPoints += player.mPoints;
            afterPlayerData.mGamePoints += game.mGamePoints;
            if (player.mPlace == 1)
            {
                afterPlayerData.mWins++;
            }
            else if (player.mPlace == 3)
            {
                afterPlayerData.mLosses++;
            }
        }
    }

    std::print(out, "Number of games: {}\n\n", games.size());

    std::vector<int> gameLengths(11, 0);
    for (auto& game : games)
    {
        gameLengths[game.mTurns]++;
    }
    for(auto&& [gameLength, count]: std::views::enumerate(gameLengths))
    {
        if(count == 0)
            continue;
        std::print(out, "{:-2} turns: {:-2} games ({:-3}%)\n", gameLength, count, count * 100 / static_cast<int>(games.size()));
    }
    std::print(out, "\n");
    std::vector<int> scoreDiffs(30, 0);
    for (auto& game : games)
    {
        if (game.mPlayers.size() < 2)
            continue;
        auto winnerIt = std::ranges::find_if(game.mPlayers, [](const cGame::cPlayerData& player) { return player.mPlace == 1; });
        auto secondIt = std::ranges::find_if(game.mPlayers, [](const cGame::cPlayerData& player) { return player.mPlace == 2; });
        if (winnerIt != game.mPlayers.end() && secondIt != game.mPlayers.end())
        {
            int diff = winnerIt->mPoints - secondIt->mPoints;
            if (diff < static_cast<int>(scoreDiffs.size()))
                scoreDiffs[diff]++;
        }
    }
    for (auto&& [diff, count] : std::views::enumerate(scoreDiffs))
    {
        if (count == 0)
            continue;
        std::print(out, "winner won by {} points: {:-2} games ({:-3}%)\n", diff, count, count * 100 / static_cast<int>(games.size()));
    }
    std::print(out, "\n");
    std::vector<int> winnerPoints(30, 0);
    for (auto& game : games)
    {
        auto winnerIt = std::ranges::find_if(game.mPlayers, [](const cGame::cPlayerData& player) { return player.mPlace == 1; });
        if (winnerIt != game.mPlayers.end() && winnerIt->mPoints < static_cast<int>(winnerPoints.size()))
            winnerPoints[winnerIt->mPoints]++;
    }
    for (auto&& [points, count] : std::views::enumerate(winnerPoints))
    {
        if (count == 0)
            continue;
        std::print(out, "winner with {:2} points: {:-2} games ({:-3}%)\n", points, count, count * 100 / static_cast<int>(games.size()));
    }
    

    std::print(out, "\n----- PLAYERS -----\n");
    for (auto&& [playerName, playerData] : players)
    {
        std::print(out, "-- {} --\n", playerName);
        std::print(out, "{}\n", playerData.mPerformance.toString());
        for (auto&& [positionIdx, positionData] : std::views::enumerate(playerData.mPerformanceByPosition))
        {
            std::print(out, "  - in position {} played {} {}\n", positionIdx + 1, positionData.mPlays, positionData.toString());
        }
        for(auto&& [previousPlayer, afterPlayerData]: playerData.mAfterPlayer)
        {
            std::print(out, "  - after {:5} played {} {}\n", previousPlayer, afterPlayerData.mPlays, afterPlayerData.toString());
        }
    }

    std::print(out, "\n\n\n----- LEADERS (Zsolt, Marci) -----\n");
    printLeadersStats(out, { "Zsolt", "Marci" });

    std::print(out, "\n\n\n----- LEADERS (all) -----\n");
    printLeadersStats(out, { "Zsolt", "Marci", "Evi" });

    std::print(out, "\n\n\n----- LEADERS (Evi) -----\n");
    printLeadersStats(out, { "Evi" });
    std::print(out, "\n\n\n----- LEADERS (Zsolt) -----\n");
    printLeadersStats(out, { "Zsolt" });
    std::print(out, "\n\n\n----- LEADERS (Marci) -----\n");
    printLeadersStats(out, { "Marci" });

    std::print(out, "\n\n\n----- Leader Position / Win -----\n");
    for(auto&& [leaderName, leaderData]: leaderStats)
    {
        std::print(out, "-- {} --\n", leaderName);
        for(auto&& [positionIdx, positionData]: std::views::enumerate(leaderData.mPositions) | std::views::take(3))
        {
            std::print(out, "  - seat #{} picked {:2} times, wins: {:2} ({:3}%)\n", positionIdx + 1, positionData.mPicked, positionData.mWins, positionData.mPicked == 0 ? 0 : positionData.mWins * 100 / positionData.mPicked);
        }
    }


}