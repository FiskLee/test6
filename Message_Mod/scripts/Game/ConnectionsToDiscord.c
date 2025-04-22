modded class SCR_BaseGameMode {
	static ref map<string, int> PlayerNameAndFaction = new map<string, int>();
	static ref map<int, string> PlayerIdAndName = new map<int, string>();
	
	private ref WDN_RestCallback callback;
	private RestContext restContext;
	
	private string channelId = string.Empty;
	private string webhookToken = string.Empty;
	private string maxPlayers;
	private string mapName;
	private string serverName;
	
	override void StartGameMode() {
		super.StartGameMode();

		if (!IsMaster())
			return;

		SCR_JsonLoadContext loadContext = new SCR_JsonLoadContext();
		string filepath = "$profile:discord_status_chat.json";
		
		if (!loadContext.LoadFromFile(filepath)) {
			SCR_JsonSaveContext saveContext = new SCR_JsonSaveContext();
			saveContext.WriteValue("serverName", "");
			saveContext.WriteValue("maxPlayers", "");
			saveContext.WriteValue("mapName", "");
			saveContext.WriteValue("adminRoleId", "");
			saveContext.WriteValue("serverStatusChannel", "");
			saveContext.WriteValue("chatChannel", "");
			saveContext.SaveToFile(filepath);
			return;
		}
		
		loadContext.ReadValue("serverName", serverName);
		loadContext.ReadValue("maxPlayers", maxPlayers);
		loadContext.ReadValue("mapName", mapName);
		
		string serverStatusChannel;
		loadContext.ReadValue("serverStatusChannel", serverStatusChannel);
		
		if (serverStatusChannel != string.Empty) {
			array<string> status = new array<string>();
			serverStatusChannel.Split("/", status, true);
				
			channelId = status[0];
			webhookToken = status[1];
		}
		
		string url = string.Format("https://discord.com/api/webhooks/%1/", channelId);
		restContext = GetGame().GetRestApi().GetContext(url);
		restContext.SetHeaders("Content-Type, application/json");
		callback = new WDN_RestCallback("$profile:messageId_serverstatus.txt");
		
		SCR_FactionManager factionManager = SCR_FactionManager.Cast(GetGame().GetFactionManager());
		factionManager.GetOnPlayerFactionChanged_S().Insert(OnPlayerFaction);
		
		GetGame().GetCallqueue().CallLater(SetupServer, 15000, false);
		GetGame().GetCallqueue().CallLater(SetupConnection, 30000, false);
	}
	
	private void SetupServer() {
		string filePath = "$profile:serverId.txt";
		string serverId = string.Empty;
		
		if (FileIO.FileExists(filePath)) {
			FileHandle textFileR = FileIO.OpenFile(filePath, FileMode.READ);
		
			if (textFileR) {
				textFileR.ReadLine(serverId);
				textFileR.Close();
			}
		} else {
			for (int i = 0; i < 12; i++) {
				serverId += Math.RandomInt(1,9).ToString();
			}
			
			FileHandle textFileW = FileIO.OpenFile(filePath, FileMode.WRITE);
			
			if (textFileW) {
				textFileW.WriteLine(serverId);
				textFileW.Close();
			}
		}
		
		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		gameMode.SetServer(serverId, serverName, mapName, maxPlayers);
	}
	
	private void SetupConnection() {
		GetGame().GetCallqueue().CallLater(UpdateChannel, Premium.serverStatusUpdateTime(), true);
		
		Print("----------- PREMIUM CONFIGURATIONS -----------", LogLevel.WARNING);
		Print("PREMIUM IS ACTIVE: " + Premium.isPremium(), LogLevel.WARNING);
		Print("----------------------------------------------", LogLevel.WARNING);
		Print("Commands - !kills Command Enabled: " + Premium.isKillCommandEnabled(), LogLevel.WARNING);
		Print("Server - Auto Restart When Match Ends: " + Premium.autoRestartsAfterMatchEnds(), LogLevel.WARNING);
		Print("Server - Status Update Time In Minutes: " + Premium.serverStatusUpdateTime(), LogLevel.WARNING);
		Print("Server - Seeding Rules: " + Premium.isSeedingRules(), LogLevel.WARNING);
		Print("Player - Is Armband Locked: " + Premium.isArmbandLocked(), LogLevel.WARNING);
		Print("Admin - Notify When Opening Game Master: " + Premium.adminNotifyOpeningGameMaster(), LogLevel.WARNING);
		Print("Chat - Show All Chat Channels: " + Premium.showAllChatChannels(), LogLevel.WARNING);
		Print("AI - Kill Count is Enabled: " + Premium.AiKillCountIsEnabled(), LogLevel.WARNING);
		Print("----------------------------------------------", LogLevel.WARNING);
		Print("Rank - Leaderboard Update Time In Minutes: " + (Premium.rankTableUpdateTime() / 60) / 1000, LogLevel.WARNING);
		Print("Rank - Number Of Players To Enable Rank: " + Premium.rankNumberOfPlayers(), LogLevel.WARNING);
		Print("Rank - Number Of Players In Leaderboard: " + Premium.rankNumberOfPlayersInTable(), LogLevel.WARNING);
		Print("Rank - Number Of Players In Final Leaderboard: " + Premium.rankNumberOfPlayersInFinalTable(), LogLevel.WARNING);
		Print("----------------------------------------------", LogLevel.WARNING);
		Print("Player Overall Rank - Ordered By Wins: " + Premium.isPlayerRankOrderedByWins(), LogLevel.WARNING);
		Print("Player Overall Rank - Number Of Players In Leaderboard: " + Premium.playerRankNumberOfPlayers(), LogLevel.WARNING);
		Print("----------------------------------------------", LogLevel.WARNING);
		Print("Player PVP Rank - Number Of Players In Leaderboard: " + Premium.playerRankPVPNumberOfPlayers(), LogLevel.WARNING);
		Print("----------------------------------------------", LogLevel.WARNING);
		Print("Player Item Rank - Number Of Player In Leaderboard: " + Premium.playerRankItemNumberOfPlayers(), LogLevel.WARNING);
		Print("Player Item Rank - Item Description: " + Premium.playerRankItemDescription(), LogLevel.WARNING);
		Print("Player Item Rank - Command: " + Premium.playerRankItemCommand(), LogLevel.WARNING);
		Print("Player Item Rank - Item Stolen Message: " + Premium.playerRankItemStolenMessage(), LogLevel.WARNING);
		Print("Player Item Rank - Item Delivery Message: " + Premium.playerRankItemDeliveryMessage(), LogLevel.WARNING);
		Print("Player Item Rank - Item Names: " + Premium.playerRankItemItemNames(), LogLevel.WARNING);
		Print("Player Item Rank - Is Enable: " + Premium.playerRankItemIsAvailable(), LogLevel.WARNING);
		Print("----------------------------------------------", LogLevel.WARNING);
	}
	
	private void UpdateChannel() {
		if (webhookToken == string.Empty)
			return;
		
		int playerCount = GetGame().GetPlayerManager().GetPlayerCount();
		
		if (serverName == string.Empty)
			serverName = "NO NAME";
		
		if (maxPlayers == string.Empty)
			maxPlayers = "0";
		
		if (mapName == string.Empty)
			mapName = "NOT FOUND";
		
		string formattedMessage = "```" + serverName + "```";
		formattedMessage += "```Player Count: " + playerCount + "/" + maxPlayers + "```";
		formattedMessage += "```Map: " + mapName + "```";
		
		string msg = string.Format("{\"content\": \"%1\"}", formattedMessage);
		
		if (callback.messageId) {
			restContext.DELETE_now(webhookToken + "/messages/" + callback.messageId, string.Empty);
		}
		
        restContext.POST(callback, webhookToken + "?wait=true", msg);
	}
	
	protected void OnPlayerFaction(int playerID, SCR_PlayerFactionAffiliationComponent playerComponent, Faction faction) {
		string playername = GetGame().GetPlayerManager().GetPlayerName(playerID);
		int factionId = GetGame().GetFactionManager().GetFactionIndex(SCR_FactionManager.SGetPlayerFaction(playerID));
		SCR_BaseGameMode.PlayerNameAndFaction[playername] = factionId;
		SCR_BaseGameMode.PlayerIdAndName[playerID] = playername;
	}
}