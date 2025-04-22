class PVPToDiscord {
	static ref map<string, ref PlayerStats> playerStats = new map<string, ref PlayerStats>();
	
	private ref PlayerRank playerRank;
	private ref PlayerRankPVP playerRankPVP;
	
	private RestContext restContext;
	private string webhookToken;
	private bool gameDidEnd = false;
	private ref WDN_RestCallback callback;
	private string matchNumber;

    void PVPToDiscord(string channelId, string webhook) {		
		callback = new WDN_RestCallback("$profile:messageId_pvp.txt");
		matchNumber = MatchNumber.GetMatchNumber();
		
		string url = string.Format("https://discord.com/api/webhooks/%1/", channelId);
		restContext = GetGame().GetRestApi().GetContext(url);
		restContext.SetHeaders("Content-Type, application/json");
		webhookToken = webhook;
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		gameMode.GetOnGameModeEnd().Insert(GetOnGameModeEnd);
		
		GetGame().GetCallqueue().CallLater(SetupRankPVP, 30000, false);
    }
	
	private void SetupRankPVP() {
		GetGame().GetCallqueue().CallLater(UpdateLeaderboard, Premium.rankTableUpdateTime(), true);
	}

    void OnPlayerKilled(int killerId, int victimId) {
		if (GetGame().GetPlayerManager().GetPlayerCount() < Premium.rankNumberOfPlayers()) {
			return;
		}
		
		PlayerManager playerManager = GetGame().GetPlayerManager();
		
		if (killerId > 0) {
			string killerName = playerManager.GetPlayerName(killerId);
			
			if (PVPToDiscord.playerStats[killerName] == null) {
				PVPToDiscord.playerStats.Insert(killerName, new PlayerStats(killerName, 1));
			} else {
				PVPToDiscord.playerStats[killerName].AddKill();
			}
		}
		
		if (victimId > 0) {
			string victimName = playerManager.GetPlayerName(victimId);
			
			if (PVPToDiscord.playerStats[victimName] == null) {
				PVPToDiscord.playerStats.Insert(victimName, new PlayerStats(victimName, 0 , 1));
			} else {
				PVPToDiscord.playerStats[victimName].AddDeath();
			}
		}
    }
	
	protected void GetOnGameModeEnd(SCR_GameModeEndData endData) {
		GetGame().GetCallqueue().Remove(UpdateLeaderboard);
		
		if (GetGame().GetPlayerManager().GetPlayerCount() < Premium.rankNumberOfPlayers()) {
			if (callback.messageId)
				restContext.DELETE_now(webhookToken + "/messages/" + callback.messageId, string.Empty);
		} else {
			GetGame().GetCallqueue().CallLater(SendRankPlayer, 5000, false, endData);
			GetGame().GetCallqueue().CallLater(OnGameEnded, 20000, false);		
			
			gameDidEnd = true;
			UpdateLeaderboard();
		}
	}
	
	protected void SendRankPlayer(SCR_GameModeEndData endData) {
		playerRank = new PlayerRank(endData.GetWinnerFactionId());
		playerRankPVP = new PlayerRankPVP();
	}
	
	protected void OnGameEnded() {
		if (gameDidEnd)
			MatchNumber.SetNextMatchNumber((matchNumber.ToInt() + 1).ToString());
		
		if (Premium.autoRestartsAfterMatchEnds()) {
			GetGame().RequestClose();
			GetGame().RequestReload();
		} else
			ResetStats();
	}
	
	void ResetStats() {
		gameDidEnd = false;
		playerRank.rank = null;
		playerRank.rankPosition = null;
		PVPToDiscord.playerStats.Clear();
		SCR_XPHandlerComponent.supplyStats.Clear();
		SCR_XPHandlerComponent.baseStats.Clear();
		SCR_BaseGameMode.PlayerIdAndName.Clear();
		SCR_BaseGameMode.PlayerNameAndFaction.Clear();
	}

    void UpdateLeaderboard() {
		if (webhookToken == string.Empty)
			return;
		
       	ref array<ref PlayerStats> players = new array<ref PlayerStats>();

        foreach (string _, PlayerStats stats : PVPToDiscord.playerStats) {
           	players.Insert(stats);
		}
		
		int topKillerIndex;
		PlayerStats topKiller;
		ref array<ref PlayerStats> sortedPlayers = new array<ref PlayerStats>();
		
		int numberOfPlayers = Premium.rankNumberOfPlayersInTable();
		
		if (gameDidEnd)
			numberOfPlayers = Premium.rankNumberOfPlayersInFinalTable();
		
		while (players.Count() > 0 && sortedPlayers.Count() < numberOfPlayers) {
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
			
			if(Premium.isPremium() && DiscordPlayer.players[player.name] != string.Empty) {
            	string name = string.Format("@%1 | %2", DiscordPlayer.players[player.name], player.name);
				
				if (name.Length() > 28) {
					name = name.Substring(0, 28);
				}
				
				playernames += "```" + name + "```";
			} else
				playernames += string.Format("```%1```", player.name);
			
			string ratio = string.ToString(player.kdratio);
			
			if (ratio.Length() > 4)
				ratio = ratio.Substring(0, 4);
			
			stats += string.Format("```%1 | %2 | %3```", player.kills, player.deaths, ratio);
        }
		
		if (callback.messageId)
			restContext.DELETE_now(webhookToken + "/messages/" + callback.messageId, string.Empty);
		
		if (gameDidEnd)
			restContext.POST_now(webhookToken, GetEmbedPayload(positions, playernames, stats));
		else
			restContext.POST(callback, webhookToken + "?wait=true", GetEmbedPayload(positions, playernames, stats));
    }
	
	private string GetEmbedPayload(string positions, string players, string stats) {
		if (gameDidEnd) {
			return string.Format("{\"embeds\": [{\"title\": \"───────────────── :trophy: TOP %1 • MATCH #%2 ─────────────────\"" + "," 
				+ "\"color\": 15258703" + ","
				+ "\"fields\": [{\"name\": \"Position\", \"value\": \"%3\", \"inline\": true},"
				+ "{\"name\": \"Player\", \"value\": \"%4\", \"inline\": true},"
				+ "{\"name\": \"Kills • Deaths • Ratio\", \"value\": \"%5\", \"inline\": true}]"
				+ "}]}", Premium.rankNumberOfPlayersInFinalTable().ToString(), matchNumber, positions, players, stats);
		} else {
			return string.Format("{\"embeds\": [{\"title\": \"──────────────────── Top %1 Players ────────────────────\"" + ","
				+ "\"color\": 2243554" + ","
				+ "\"fields\": [{\"name\": \"Position\", \"value\": \"%2\", \"inline\": true},"
				+ "{\"name\": \"Player\", \"value\": \"%3\", \"inline\": true},"
				+ "{\"name\": \"Kills • Deaths • Ratio\", \"value\": \"%4\", \"inline\": true}]"
				+ "}]}", Premium.rankNumberOfPlayersInTable().ToString(), positions, players, stats);
		}
	}
}

class PlayerStats {
	string name = "";
	int kills = 0;
	int deaths = 0;
	float kdratio = 0;
	
	void PlayerStats(string Name, int Kills = 0, int Deaths = 0) {
		this.name = Name;
		this.kills = Kills;
		this.deaths = Deaths;
		UpdateRatio();
	}
	
	void AddKill() {
		kills += 1;
		UpdateRatio();
	}
	
	void AddDeath() {
		deaths += 1;
		UpdateRatio();
	}
	
	void UpdateRatio() {
		if (deaths == 0 || kills == 0) {
			kdratio = kills;
			return;
		}
		
		kdratio = (float)kills / deaths;
	}
}