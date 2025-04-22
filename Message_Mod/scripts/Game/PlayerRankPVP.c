class PlayerRankPVP {
	static ref map<string, ref PlayerStats> rank; 
	static ref map<string, int> rankPosition;
	
	private const string cryptograph = "::/::";
	private const string killers_path = "$profile:player_kills.txt";
	
	private ref WDN_RestCallback callback;
	private RestContext restContext;
	private string channelId = string.Empty;
	private string webhookToken = string.Empty;
	
	void PlayerRankPVP() {
		callback = new WDN_RestCallback("$profile:messageId_overallpvp.txt");
		SCR_JsonLoadContext loadContext = new SCR_JsonLoadContext();
		string filepath = "$profile:discord_premium_channels.json";
		loadContext.LoadFromFile(filepath);
		
		string pvpRankChannel;
		loadContext.ReadValue("pvpRankChannel", pvpRankChannel);
		
		if (pvpRankChannel != string.Empty) {
			array<string> player = new array<string>();
			pvpRankChannel.Split("/", player, true);
				
			channelId = player[0];
			webhookToken = player[1];
		}
		
		string url = string.Format("https://discord.com/api/webhooks/%1/", channelId);
		restContext = GetGame().GetRestApi().GetContext(url);
		restContext.SetHeaders("Content-Type, application/json"); 
		
		FileHandle textFileR = FileIO.OpenFile(killers_path, FileMode.READ);
		
		ref map<string, ref PlayerStats> playersSaved = new map<string, ref PlayerStats>();
		
		if (textFileR) {
			string line = string.Empty;
			
			while(textFileR.ReadLine(line) >= 0) {
				array<string> player = new array<string>();
				line.Split(cryptograph, player, true);
				
				if (player.Count() != 3)
					continue;
				
				string playerName = player[0];
				int kills = player[1].ToInt();
				int deaths = player[2].ToInt();
								
				playersSaved.Insert(playerName, new PlayerStats(playerName, kills, deaths));
			}
			
			textFileR.Close();
		}
		
		foreach(string playerName, PlayerStats player: PVPToDiscord.playerStats) {
			if (playersSaved[playerName] == null) {
				playersSaved.Insert(playerName, new PlayerStats(playerName, player.kills, player.deaths));
			} else {
				playersSaved[playerName].kills = playersSaved[playerName].kills + player.kills;
				playersSaved[playerName].deaths = playersSaved[playerName].deaths + player.deaths;
				playersSaved[playerName].UpdateRatio();
			}
		}
		
		ref array<ref PlayerStats> players = new array<ref PlayerStats>();
		
		foreach(string _, PlayerStats player: playersSaved) {
			players.Insert(player);
		}
		
		SaveToFile(players);
		UpdateLeaderboard(players);
	}
	
    private void UpdateLeaderboard(array<ref PlayerStats> players) {
		if (webhookToken == string.Empty)
			return;
		
		int topKillerIndex;
		PlayerStats topKiller;
		ref array<ref PlayerStats> sortedPlayers = new array<ref PlayerStats>();
		
		while (players.Count() > 0 && sortedPlayers.Count() < Premium.playerRankPVPNumberOfPlayers()) {
			for(int index = 0; index < players.Count(); index++) {
				PlayerStats player = players.Get(index);
				
				if (topKiller == null) {
					topKiller = player;
					topKillerIndex = index;
				} else if (player.kills > topKiller.kills) {
					topKiller = player;
					topKillerIndex = index;
				} else if (player.kills == topKiller.kills && player.deaths < topKiller.deaths) {
					topKiller = player;
					topKillerIndex = index;
				}
			}
			
			players.Remove(topKillerIndex);
			sortedPlayers.Insert(PlayerStats(topKiller.name, topKiller.kills, topKiller.deaths));
			topKiller = null;
		}
		
		string positions = "";
     	string playernames = "";
		string stats = "";
		
		for (int i = 0; i < sortedPlayers.Count(); i++) {
            PlayerStats player = sortedPlayers.Get(i);
			
			positions += string.Format("```%1```", i + 1);
			
			if (DiscordPlayer.players[player.name] != string.Empty) {
           		string name = string.Format("@%1 | %2", DiscordPlayer.players[player.name], player.name);
				
				if (name.Length() > 28) {
					name = name.Substring(0, 28);
				}
				
				playernames += "```" + name + "```";;
			} else
				playernames += string.Format("```%1```", player.name);
			
			string ratio = string.ToString(player.kdratio);
			
			if (ratio.Length() > 4)
				ratio = ratio.Substring(0, 4);
			
			stats += string.Format("```%1 | %2 | %3```", player.kills, player.deaths, ratio);
        }
		
		if (callback.messageId)
			restContext.DELETE_now(webhookToken + "/messages/" + callback.messageId, string.Empty);
	
		if(Premium.isPremium())
       		restContext.POST(callback, webhookToken + "?wait=true", GetEmbedPayload(positions, playernames, stats));
    }
	
	private string GetEmbedPayload(string positions, string players, string stats) {
		return string.Format("{\"embeds\": [{\"title\": \"─────────────────── :trophy: TOP %1 Players ───────────────────\"" + "," 
			+ "\"color\": 15258703" + ","
			+ "\"fields\": [{\"name\": \"Position\", \"value\": \"%2\", \"inline\": true},"
			+ "{\"name\": \"Player\", \"value\": \"%3\", \"inline\": true},"
			+ "{\"name\": \"Kills • Deaths • Ratio\", \"value\": \"%4\", \"inline\": true}]"
			+ "}]}", Premium.playerRankPVPNumberOfPlayers(), positions, players, stats);
	}
	
	private void SaveToFile(array<ref PlayerStats> players) {
		if (players.Count() == 0)
			return;
		
		if (FileIO.FileExists(killers_path)) {
			if (FileIO.DeleteFile(killers_path)) {
				FileHandle textFileW = FileIO.OpenFile(killers_path, FileMode.WRITE);
				
				if (textFileW) {
					foreach(PlayerStats player: players) {
						textFileW.WriteLine(player.name + cryptograph + player.kills + cryptograph + player.deaths);
					}
					
					textFileW.Close();
				}
			}
		} else {
			FileHandle textFileW = FileIO.OpenFile(killers_path, FileMode.WRITE);
			
			if (textFileW) {
				foreach(PlayerStats player: players) {
					textFileW.WriteLine(player.name + cryptograph + player.kills + cryptograph + player.deaths);
				}
				
				textFileW.Close();
			}
		}
	}
	
	static PlayerStats GetPlayerRank(string playerName) {
		if (PlayerRankPVP.rank == null) {
			PlayerRankPVP.rank = new map<string, ref PlayerStats>();
			
			FileHandle textFileR = FileIO.OpenFile(killers_path, FileMode.READ);
		
			if (textFileR) {
				string line = string.Empty;
				
				while(textFileR.ReadLine(line) >= 0) {
					array<string> player = new array<string>();
					line.Split(cryptograph, player, true);
					
					if (player.Count() != 3)
						continue;
					
					string name = player[0];
					int kills = player[1].ToInt();
					int deaths = player[2].ToInt();
					
					PlayerRankPVP.rank.Insert(name, new PlayerStats(name, kills, deaths));
				}
				
				textFileR.Close();
			}
		}
		
		PlayerStats player = PlayerRankPVP.rank[playerName];
		
		if (player)
			return player;
		else
			return PlayerStats("", 0, 0);
	}
	
	static string GetPlayerRankPosition(string playerName) {
		if (PlayerRankPVP.rankPosition == null) {
			PlayerRankPVP.rankPosition = new map<string, int>();
			
			ref array<ref PlayerStats> playerSavedArray = new array<ref PlayerStats>();

	        foreach (string _, PlayerStats player : PlayerRankPVP.rank) {
	           	playerSavedArray.Insert(player);
			}
			
			int biggestPlayerIndex;
			PlayerStats biggestPlayer;
		
			while (playerSavedArray.Count() > 0) {
				for(int index = 0; index < playerSavedArray.Count(); index++) {
					PlayerStats player = playerSavedArray.Get(index);
					
					if (biggestPlayer == null) {
						biggestPlayer = player;
						biggestPlayerIndex = index;
					} else if (player.kills > biggestPlayer.kills) {
						biggestPlayer = player;
						biggestPlayerIndex = index;
					} else if (player.kills == biggestPlayer.kills && player.kdratio > biggestPlayer.kdratio) {
						biggestPlayer = player;
						biggestPlayerIndex = index;
					}
				}
			
				playerSavedArray.Remove(biggestPlayerIndex);
				int position = PlayerRankPVP.rankPosition.Count() + 1;
				PlayerRankPVP.rankPosition.Insert(biggestPlayer.name, position);
				biggestPlayer = null;
			}
		}
		
		int position = PlayerRankPVP.rankPosition[playerName];
		
		if (position)
			return position.ToString();
		else 
			return "0";
	}
}