class PlayerRank {
	static ref map<string, ref PlayerRankData> rank; 
	static ref map<string, int> rankPosition;
	
	private const string cryptograph = "::/::";
	private const string wins_path = "$profile:player_match_rank.txt";
	
	private ref WDN_RestCallback callback;
	private RestContext restContext;
	private string channelId = string.Empty;
	private string webhookToken = string.Empty;	
	
	void PlayerRank(int factionId) {
		callback = new WDN_RestCallback("$profile:messageId_overall.txt");
		SCR_JsonLoadContext loadContext = new SCR_JsonLoadContext();
		string filepath = "$profile:discord_premium_channels.json";
		loadContext.LoadFromFile(filepath);
		
		string overallRankChannel;
		loadContext.ReadValue("overallRankChannel", overallRankChannel);
		
		if (overallRankChannel != string.Empty) {
			array<string> player = new array<string>();
			overallRankChannel.Split("/", player, true);
				
			channelId = player[0];
			webhookToken = player[1];
		}
		
		string url = string.Format("https://discord.com/api/webhooks/%1/", channelId);
		restContext = GetGame().GetRestApi().GetContext(url);
		restContext.SetHeaders("Content-Type, application/json"); 
		
		array<string> match_winners = new array<string>();
		
		if (factionId >= 0) {
			foreach (string playerName, int playerFactionId : SCR_BaseGameMode.PlayerNameAndFaction) {
				if (playerFactionId == factionId)
					match_winners.Insert(playerName);
			}
		}
		
		UpdateScores(match_winners, factionId >= 0);
	}
	
	private void UpdateScores(array<string> match_winners, bool gameDidEnd) {
		ref map<string, ref PlayerRankData> playersSaved = new map<string, ref PlayerRankData>();
		
		FileHandle textFileR = FileIO.OpenFile(wins_path, FileMode.READ);
		
		if (textFileR) {
			string line = string.Empty;
			
			while(textFileR.ReadLine(line) >= 0) {
				array<string> player = new array<string>();
				line.Split(cryptograph, player, true);
				
				if (player.Count() != 3)
					continue;
				
				string playerName = player[0];
				int gamesWon = player[1].ToInt();
				int points = player[2].ToInt();
				
				playersSaved.Insert(playerName, new PlayerRankData(playerName, gamesWon, points));
			}
			
			textFileR.Close();
		}
		
		array<int> playersIdInServer = {};
		GetGame().GetPlayerManager().GetPlayers(playersIdInServer);
		
		array<string> playersInServer = new array<string>();
		
		foreach (int playerId: playersIdInServer) {
			string playerName = SCR_BaseGameMode.PlayerIdAndName[playerId];
			
			if (playerName != string.Empty)
				playersInServer.Insert(playerName);
		}
		
		foreach(int _, string playerName: SCR_BaseGameMode.PlayerIdAndName) {
			int gameWinPoints = 0;
			
			if (gameDidEnd && playersInServer.Contains(playerName) && match_winners.Contains(playerName))
				gameWinPoints = 1;
			
			int playerPoints = GetPlayerPoints(playerName);
			
			if (playersSaved[playerName] == null) {
				playersSaved.Insert(playerName, new PlayerRankData(playerName, gameWinPoints, playerPoints));
			} else {
				playersSaved[playerName].wins = playersSaved[playerName].wins + gameWinPoints;
				playersSaved[playerName].points = playersSaved[playerName].points + playerPoints;
			}
		}
		
		ref array<ref PlayerRankData> playerSavedArray = new array<ref PlayerRankData>();
		
        foreach (string _, PlayerRankData player : playersSaved) {
           	playerSavedArray.Insert(player);
		}

		SaveToFile(playerSavedArray);

		int biggestPlayerIndex;
		PlayerRankData biggestPlayer;
		ref array<ref PlayerRankData> sortedPlayers = new array<ref PlayerRankData>();
		
		if (Premium.isPlayerRankOrderedByWins()) {
			while (playerSavedArray.Count() > 0 && sortedPlayers.Count() < Premium.playerRankNumberOfPlayers()) {
				for(int index = 0; index < playerSavedArray.Count(); index++) {
					PlayerRankData player = playerSavedArray.Get(index);
					
					if (biggestPlayer == null) {
						biggestPlayer = player;
						biggestPlayerIndex = index;
					} else if (player.wins > biggestPlayer.wins) {
						biggestPlayer = player;
						biggestPlayerIndex = index;
					} else if (player.wins == biggestPlayer.wins && player.points > biggestPlayer.points) {
						biggestPlayer = player;
						biggestPlayerIndex = index;
					}
				}
				
				playerSavedArray.Remove(biggestPlayerIndex);
				sortedPlayers.Insert(new PlayerRankData(biggestPlayer.name, biggestPlayer.wins, biggestPlayer.points));
				biggestPlayer = null;
			}
		} else {
			while (playerSavedArray.Count() > 0 && sortedPlayers.Count() < Premium.playerRankNumberOfPlayers()) {
				for(int index = 0; index < playerSavedArray.Count(); index++) {
					PlayerRankData player = playerSavedArray.Get(index);
					
					if (biggestPlayer == null) {
						biggestPlayer = player;
						biggestPlayerIndex = index;
					} else if (player.points > biggestPlayer.points) {
						biggestPlayer = player;
						biggestPlayerIndex = index;
					} else if (player.points == biggestPlayer.points && player.wins > biggestPlayer.wins) {
						biggestPlayer = player;
						biggestPlayerIndex = index;
					}
				}
				
				playerSavedArray.Remove(biggestPlayerIndex);
				sortedPlayers.Insert(new PlayerRankData(biggestPlayer.name, biggestPlayer.wins, biggestPlayer.points));
				biggestPlayer = null;
			}
		}
		
		string positions = "";
     	string playerNames = "";
		string stats = "";
		
		for (int i = 0; i < sortedPlayers.Count(); i++) {
            PlayerRankData player = sortedPlayers.Get(i);
			
			positions += string.Format("```%1```", i + 1);
			
			if(DiscordPlayer.players[player.name] != string.Empty) {
            	string name = string.Format("@%1 | %2", DiscordPlayer.players[player.name], player.name);
				
				if (name.Length() > 28) {
					name = name.Substring(0, 28);
				}
				
				playerNames += "```" + name + "```";
			} else
				playerNames += string.Format("```%1```", player.name);
			
			if (Premium.isPlayerRankOrderedByWins())
				stats += string.Format("```%1 • %2```", player.wins, player.points);
			else 
				stats += string.Format("```%1 • %2```", player.points, player.wins);
        }
		
		if (webhookToken == string.Empty)
			return;
		
		if (callback.messageId)
			restContext.DELETE_now(webhookToken + "/messages/" + callback.messageId, string.Empty);
		
		if(Premium.isPremium())
			restContext.POST(callback, webhookToken + "?wait=true", GetEmbedPayload(positions, playerNames, stats));
	}
	
	private void SaveToFile(array<ref PlayerRankData> players) {
		if (players.Count() == 0)
			return;
		
		if (FileIO.FileExists(wins_path)) {
			if (FileIO.DeleteFile(wins_path)) {
				FileHandle textFileW = FileIO.OpenFile(wins_path, FileMode.WRITE);
				
				if (textFileW) {
					foreach(PlayerRankData player: players) {
						textFileW.WriteLine(player.name + cryptograph + player.wins + cryptograph + player.points);
					}
					
					textFileW.Close();
				}
			}
		} else {
			FileHandle textFileW = FileIO.OpenFile(wins_path, FileMode.WRITE);
			
			if (textFileW) {
				foreach(PlayerRankData player: players) {
					textFileW.WriteLine(player.name + cryptograph + player.wins + cryptograph + player.points);
				}
				
				textFileW.Close();
			}
		}
	}
	
	private int GetPlayerPoints(string playerName) {
		float supplyPoints = 0;
		float basePoints = 0;
		float killPoints = 0;
		
		SupplyStats supplyStats = SCR_XPHandlerComponent.supplyStats[playerName];
		BaseStats baseStats = SCR_XPHandlerComponent.baseStats[playerName];
		PlayerStats killStats = PVPToDiscord.playerStats[playerName];
		
		if(supplyStats != null && supplyStats.amount > 0) {
			supplyPoints = (supplyStats.amount / 4000) * 100;
		}
		
		if(baseStats != null && baseStats.points > 0) {
			basePoints = (baseStats.points / 4000) * 100;
		}
		
		if(killStats != null) {
			killPoints = Math.Max(killStats.kills - (killStats.deaths * 1.5), 0) * 10;
		}
		
		float totalPoints = (supplyPoints * 0.45) + (basePoints * 0.35) + (killPoints * 0.20);
		
		return Math.Round(totalPoints);
	}
	
	private string GetEmbedPayload(string positions, string players, string stats) {
		string description = "";
		
		if (Premium.isPlayerRankOrderedByWins())
			description = "Wins • Rank Points";
		else
			description = "Rank Points • Wins";
		
		return string.Format("{\"embeds\": [{\"title\": \"─────────────────── :trophy: Top %1 Players ───────────────────\"" + "," 
			+ "\"color\": 15258703" + ","
			+ "\"fields\": [{\"name\": \"Position\", \"value\": \"%2\", \"inline\": true},"
			+ "{\"name\": \"Player\", \"value\": \"%3\", \"inline\": true},"
			+ "{\"name\": \"%4\", \"value\": \"%5\", \"inline\": true}]"
			+ "}]}", Premium.playerRankNumberOfPlayers().ToString(), positions, players, description, stats);
	}
	
	static PlayerRankData GetPlayerRank(string playerName) {
		if (PlayerRank.rank == null) {
			PlayerRank.rank = new map<string, ref PlayerRankData>();
			
			FileHandle textFileR = FileIO.OpenFile(wins_path, FileMode.READ);
		
			if (textFileR) {
				string line = string.Empty;
				
				while(textFileR.ReadLine(line) >= 0) {
					array<string> player = new array<string>();
					line.Split(cryptograph, player, true);
					
					if (player.Count() != 3)
						continue;
					
					string name = player[0];
					int gamesWon = player[1].ToInt();
					int points = player[2].ToInt();
					
					PlayerRank.rank.Insert(name, new PlayerRankData(name, gamesWon, points));
				}
				
				textFileR.Close();
			}
		}
		
		PlayerRankData player = PlayerRank.rank[playerName];
		
		if (player)
			return player;
		else
			return PlayerRankData("", 0, 0);
		
	}
	
	static string GetPlayerRankPosition(string playerName) {
		if (PlayerRank.rankPosition == null) {
			PlayerRank.rankPosition = new map<string, int>();
			
			ref array<ref PlayerRankData> playerSavedArray = new array<ref PlayerRankData>();

	        foreach (string _, PlayerRankData player : PlayerRank.rank) {
	           	playerSavedArray.Insert(player);
			}
			
			int biggestPlayerIndex;
			PlayerRankData biggestPlayer;
			
			if (Premium.isPlayerRankOrderedByWins()) {
				while (playerSavedArray.Count() > 0) {
					for(int index = 0; index < playerSavedArray.Count(); index++) {
						PlayerRankData player = playerSavedArray.Get(index);
						
						if (biggestPlayer == null) {
							biggestPlayer = player;
							biggestPlayerIndex = index;
						} else if (player.wins > biggestPlayer.wins) {
							biggestPlayer = player;
							biggestPlayerIndex = index;
						} else if (player.wins == biggestPlayer.wins && player.points > biggestPlayer.points) {
							biggestPlayer = player;
							biggestPlayerIndex = index;
						}
					}
				
					playerSavedArray.Remove(biggestPlayerIndex);
					int position = PlayerRank.rankPosition.Count() + 1;
					PlayerRank.rankPosition.Insert(biggestPlayer.name, position);
					biggestPlayer = null;
				}
			} else {
				while (playerSavedArray.Count() > 0) {
					for(int index = 0; index < playerSavedArray.Count(); index++) {
						PlayerRankData player = playerSavedArray.Get(index);
						
						if (biggestPlayer == null) {
							biggestPlayer = player;
							biggestPlayerIndex = index;
						} else if (player.points > biggestPlayer.points) {
							biggestPlayer = player;
							biggestPlayerIndex = index;
						} else if (player.points == biggestPlayer.points && player.wins > biggestPlayer.wins) {
							biggestPlayer = player;
							biggestPlayerIndex = index;
						}
					}
				
					playerSavedArray.Remove(biggestPlayerIndex);
					int position = PlayerRank.rankPosition.Count() + 1;
					PlayerRank.rankPosition.Insert(biggestPlayer.name, position);
					biggestPlayer = null;
				}
			}
		}
		
		int position = PlayerRank.rankPosition[playerName];
		
		if (position)
			return position.ToString();
		else 
			return "0";
	}
}

class PlayerRankData {
	string name;
	int wins;
	int points;
	
	void PlayerRankData(string Name, int Wins, int Points) {
		name = Name;
		wins = Wins;
		points = Points;
	}
}

