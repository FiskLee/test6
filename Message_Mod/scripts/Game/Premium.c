modded class SCR_BaseGameMode {
	private ref Premium premium;
	
	override void StartGameMode() {
		super.StartGameMode();
		
		if (!IsMaster())
			return;
		
		premium = Premium();
	}
}

class Premium {
	private static bool isPremium;
	
	private static string playerRankOverall_OrderedByWins = "TRUE";
	private static string playerRankOverall_NumberOfPlayersInTable = "40";
	private static string playerRankPVP_NumberOfPlayersInTable = "40";
	private static string playerRankItem_NumberOfPlayers = "40";
	private static string playerRankItem_ItemDescription = "Items Stolen";
	private static string playerRankItem_ItemCommand = "!item";
	private static string playerRankItem_ItemStolenMessage = "I stole %1 item!";
	private static string playerRankItem_ItemDeliveryMessage = "I delivered %1 item!";
	private static string playerRankItem_ItemNames = "trouser,pant,jean";
	private static string playerRankItem_IsAvailable = "FALSE";
	private static string chat_showAllChannels = "FALSE";
	private static string ai_KillCountEnabled = "FALSE";
	private static string rank_MinimumPlayersInServer = "5";
	private static string rank_NumberOfPlayersInTable = "20";
	private static string rank_NumberOfPlayersInFinalTable = "3";
	private static string rank_TableUpdateInMinutes = "10";
	private static string command_EnableKillsCommand = "TRUE";
	private static string server_StatusUpdateInMinutes = "10";
	private static string server_AutoRestartAfterMatchEnds = "FALSE";
	private static string server_SeedingRules = "FALSE";
	private static string admin_NotifyOpeningGameMaster = "TRUE";
	private static string player_LockArmband = "FALSE";

	
	void Premium() {
		SCR_JsonLoadContext loadContextConfigs = new SCR_JsonLoadContext();
		string filepath_config = "$profile:discord_premium_config.json";
		
		if (!loadContextConfigs.LoadFromFile(filepath_config)) {
			SCR_JsonSaveContext saveContextConfig = new SCR_JsonSaveContext();
			saveContextConfig.WriteValue("PlayerRankOverall_OrderedByWins", "TRUE");
			saveContextConfig.WriteValue("PlayerRankOverall_NumberOfPlayersInTable", "40");
			saveContextConfig.WriteValue("PlayerRankPVP_NumberOfPlayersInTable", "40");
			saveContextConfig.WriteValue("PlayerRankItem_NumberOfPlayersInTable", "40");
			saveContextConfig.WriteValue("PlayerRankItem_ItemDescription", "Items Stolen");
			saveContextConfig.WriteValue("PlayerRankItem_ItemCommand", "!item");
			saveContextConfig.WriteValue("PlayerRankItem_ItemStolenMessage", "I stole %1 item!");
			saveContextConfig.WriteValue("PlayerRankItem_ItemDeliveryMessage", "I delivered %1 item!");
			saveContextConfig.WriteValue("PlayerRankItem_ItemNames", "trouser,pant,jean");
			saveContextConfig.WriteValue("PlayerRankItem_IsAvailable", "FALSE");
			saveContextConfig.WriteValue("Rank_MinimumPlayersInServer", "5");
			saveContextConfig.WriteValue("Rank_NumberOfPlayersInTable", "20");
			saveContextConfig.WriteValue("Rank_NumberOfPlayersInFinalTable", "3");
			saveContextConfig.WriteValue("Rank_TableUpdateInMinutes", "10");
			saveContextConfig.WriteValue("Chat_ShowAllChannels", "FALSE");
			saveContextConfig.WriteValue("Ai_KillCountEnabled", "FALSE");
			saveContextConfig.WriteValue("Command_EnableKillsCommand", "TRUE");
			saveContextConfig.WriteValue("Server_StatusUpdateInMinutes", "10");
			saveContextConfig.WriteValue("Server_AutoRestartAfterMatchEnds", "FALSE");
			saveContextConfig.WriteValue("Server_SeedingRules", "FALSE");
			saveContextConfig.WriteValue("Admin_NotifyOpeningGameMaster", "TRUE");
			saveContextConfig.WriteValue("Player_LockArmband", "FALSE");
			saveContextConfig.SaveToFile(filepath_config);
		} else {
			loadContextConfigs.ReadValue("PlayerRankOverall_OrderedByWins", Premium.playerRankOverall_OrderedByWins);
			loadContextConfigs.ReadValue("PlayerRankOverall_NumberOfPlayersInTable", Premium.playerRankOverall_NumberOfPlayersInTable);
			loadContextConfigs.ReadValue("PlayerRankPVP_NumberOfPlayersInTable", Premium.playerRankPVP_NumberOfPlayersInTable);
			loadContextConfigs.ReadValue("PlayerRankItem_NumberOfPlayersInTable", Premium.playerRankItem_NumberOfPlayers);
			loadContextConfigs.ReadValue("PlayerRankItem_ItemDescription", Premium.playerRankItem_ItemDescription);
			loadContextConfigs.ReadValue("PlayerRankItem_ItemCommand", Premium.playerRankItem_ItemCommand);
			loadContextConfigs.ReadValue("PlayerRankItem_ItemStolenMessage", Premium.playerRankItem_ItemStolenMessage);
			loadContextConfigs.ReadValue("PlayerRankItem_ItemDeliveryMessage", Premium.playerRankItem_ItemDeliveryMessage);
			loadContextConfigs.ReadValue("PlayerRankItem_ItemNames", Premium.playerRankItem_ItemNames);
			loadContextConfigs.ReadValue("PlayerRankItem_IsAvailable", Premium.playerRankItem_IsAvailable);
			loadContextConfigs.ReadValue("Rank_MinimumPlayersInServer", Premium.rank_MinimumPlayersInServer);
			loadContextConfigs.ReadValue("Rank_NumberOfPlayersInTable", Premium.rank_NumberOfPlayersInTable);
			loadContextConfigs.ReadValue("Rank_NumberOfPlayersInFinalTable", Premium.rank_NumberOfPlayersInFinalTable);
			loadContextConfigs.ReadValue("Rank_TableUpdateInMinutes", Premium.rank_TableUpdateInMinutes);
			loadContextConfigs.ReadValue("Chat_ShowAllChannels", Premium.chat_showAllChannels);
			loadContextConfigs.ReadValue("Ai_KillCountEnabled", Premium.ai_KillCountEnabled);
			loadContextConfigs.ReadValue("Command_EnableKillsCommand", Premium.command_EnableKillsCommand);
			loadContextConfigs.ReadValue("Server_StatusUpdateInMinutes", Premium.server_StatusUpdateInMinutes);
			loadContextConfigs.ReadValue("Server_AutoRestartAfterMatchEnds", Premium.server_AutoRestartAfterMatchEnds);
			loadContextConfigs.ReadValue("Server_SeedingRules", Premium.server_SeedingRules);
			loadContextConfigs.ReadValue("Admin_NotifyOpeningGameMaster", Premium.admin_NotifyOpeningGameMaster);
			loadContextConfigs.ReadValue("Player_LockArmband", Premium.player_LockArmband);
		}
	}
	
	static void setPremium(bool value) {
		Premium.isPremium = value
	}
	
	static bool isPremium() {
		return Premium.isPremium;
	}
	
	static bool isPlayerRankOrderedByWins() {
		return Premium.playerRankOverall_OrderedByWins == "TRUE";
	}
	
	static int playerRankNumberOfPlayers() {
		return Premium.playerRankOverall_NumberOfPlayersInTable.ToInt();
	}
	
	static int playerRankPVPNumberOfPlayers() {
		return Premium.playerRankPVP_NumberOfPlayersInTable.ToInt();
	}
	
	static int playerRankItemNumberOfPlayers() {
		return Premium.playerRankItem_NumberOfPlayers.ToInt();
	}
	
	static string playerRankItemDescription() {
		return Premium.playerRankItem_ItemDescription;
	}
	
	static string playerRankItemCommand() {
		return Premium.playerRankItem_ItemCommand;
	}
	
	static string playerRankItemStolenMessage() {
		return Premium.playerRankItem_ItemStolenMessage;
	}
	
	static string playerRankItemDeliveryMessage() {
		return Premium.playerRankItem_ItemDeliveryMessage;
	}
	
	static bool playerRankItemIsAvailable() {
		if (!Premium.isPremium()) {
			return false;
		}
		
		return Premium.playerRankItem_IsAvailable == "TRUE";
	}
	
	static array<string> playerRankItemItemNames() {
		array<string> values = {};
		Premium.playerRankItem_ItemNames.Split(",", values, false);
		return values;
	}
	
	static bool showAllChatChannels() {
		if (!Premium.isPremium()) {
			return false;
		}
		
		return Premium.chat_showAllChannels == "TRUE";
	}
	
	static int rankNumberOfPlayers() {
		if (!Premium.isPremium()) {
			return 5;
		}
		
		return Premium.rank_MinimumPlayersInServer.ToInt();
	}
	
	static int rankNumberOfPlayersInTable() {
		if (!Premium.isPremium()) {
			return 20;
		}
		
		return Premium.rank_NumberOfPlayersInTable.ToInt();
	}
	
	static int rankNumberOfPlayersInFinalTable() {
		if (!Premium.isPremium()) {
			return 3;
		}
		
		return Premium.rank_NumberOfPlayersInFinalTable.ToInt();
	}
	
	static int rankTableUpdateTime() {
		int randomTime = Math.RandomInt(0, 29);
		
		if (!Premium.isPremium()) {
			int time = 600 + randomTime;
			return time * 1000;
		}
		
		int time = (Premium.rank_TableUpdateInMinutes.ToInt() * 60) + randomTime;
		
		return time * 1000;
	}
	
	static bool isKillCommandEnabled() {
		return Premium.command_EnableKillsCommand == "TRUE";
	}
	
	static bool autoRestartsAfterMatchEnds() {
		if (!Premium.isPremium()) {
			return false;
		}
		
		return Premium.server_AutoRestartAfterMatchEnds == "TRUE";
	}
	
	static int serverStatusUpdateTime() {
		int randomTime = Math.RandomInt(0, 29);
		
		if (!Premium.isPremium()) {
			int time = 600 + randomTime;
			return time * 1000;
		}
		
		int time = (Premium.server_StatusUpdateInMinutes.ToInt() * 60) + randomTime;
		
		return time * 1000;
	}
	
	static bool isArmbandLocked() {
		if (!Premium.isPremium()) {
			return false;
		}
		
		return Premium.player_LockArmband == "TRUE";
	}
	
	static bool isSeedingRules() {
		if (!Premium.isPremium()) {
			return false;
		}
		
		return Premium.server_SeedingRules == "TRUE";
	}
	
	static bool AiKillCountIsEnabled() {
		if (!Premium.isPremium()) {
			return false;
		}
		
		return Premium.ai_KillCountEnabled == "TRUE";
	}
	
	static bool adminNotifyOpeningGameMaster() {
		return Premium.admin_NotifyOpeningGameMaster == "TRUE";
	}
}