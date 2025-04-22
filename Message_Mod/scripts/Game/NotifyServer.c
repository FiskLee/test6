modded class SCR_NotificationsComponent {
	static bool SendCommandMessage(int playerId, string message, bool value = false, array<string> customMessage = null) {
		if (Replication.IsServer()) {
			SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId));
				
			if (!playerController)
				return false;
			
			SCR_NotificationsComponent notification = SCR_NotificationsComponent.Cast(playerController.FindComponent(SCR_NotificationsComponent));
			
			if (!notification)
				return false;
			
			notification.SendToOwnerCommandStatus(playerId, message, value, customMessage); 
			return true;
		}  else {
			SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId));
			if (!playerController)
				return false;
			
			SCR_NotificationsComponent notification = SCR_NotificationsComponent.Cast(playerController.FindComponent(SCR_NotificationsComponent));
			
			if (!notification)
				return false;
			
			notification.Rpc(notification.Rpc_SendStatusMessage, playerId, message, value, customMessage);
			return true;
		}
		return true;
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void Rpc_SendStatusMessage(int playerId, string message, bool value, array<string> customMessage) {
		string playerName = GetGame().GetPlayerManager().GetPlayerName(playerId);
		
		if (message == "!kills") {
			if (!Premium.isKillCommandEnabled()) {
				string disableMessage = "!kills command is disabled.";
				SendCommandMessage(playerId, disableMessage);
				return;
			}
			
			PlayerStats playerStats = PVPToDiscord.playerStats[playerName];
			string msg;
			
			if(playerStats != null) {
				string ratio = string.ToString(playerStats.kdratio);
			
				if (ratio.Length() > 4)
					ratio = ratio.Substring(0, 4);
				
				msg = "Kills [" + playerStats.kills + "]" 
					+ " • Deaths [" + playerStats.deaths + "]" 
					+ " • K/D [" + ratio + "]";
			} else {
				msg = "Kills [0]" + " • Deaths [0]" + " • K/D [0]";
			}
			
			SendCommandMessage(playerId, msg);
			return;
		} 
		
		if (message == "!rank") {
			PlayerRankData playerRank = PlayerRank.GetPlayerRank(playerName);
			string msg = "";
			
			if (Premium.isPlayerRankOrderedByWins()) {
				msg = "Wins [" + playerRank.wins + "]" 
					+ " • Ranking Points [" + playerRank.points + "]" 
					+ " • Position in Rank [" + PlayerRank.GetPlayerRankPosition(playerName) + "]";
			} else {
				msg = "Ranking Points [" + playerRank.points + "]" 
					+ " • Wins [" + playerRank.wins + "]" 
					+ " • Position in Rank [" + PlayerRank.GetPlayerRankPosition(playerName) + "]";
			}
			
			SendCommandMessage(playerId, msg);
			return;
		}
		
		if (message == "!killrank") {
			PlayerStats playerStats = PlayerRankPVP.GetPlayerRank(playerName);
			
			string ratio = string.ToString(playerStats.kdratio);
			
			if (ratio.Length() > 4)
				ratio = ratio.Substring(0, 4);
			
			string msg = "Kills [" + playerStats.kills + "]" 
				+ " • Deaths [" + playerStats.deaths + "]" 
				+ " • K/D [" + ratio + "]"
				+ " • Position in Rank [" + PlayerRankPVP.GetPlayerRankPosition(playerName) + "]";
			
			SendCommandMessage(playerId, msg);
			return;
		}
		
		if (message == "!armband") {
			SendCommandMessage(playerId, message, Premium.isArmbandLocked());
			return;
		}
		
		if (message == "!itemnames") {
			SendCommandMessage(playerId, message, value, Premium.playerRankItemItemNames());
			return;
		}
		
		if (message == "!seedingrules" && Premium.isSeedingRules()) {
			SendCommandMessage(playerId, message, SCR_BaseGameMode.seedingRules);
			return;
		}
		
		if (Premium.playerRankItemIsAvailable()) {
			if (message == "!playerkills") {
				if (SCR_BaseGameMode.playersKills[playerName])
					SendCommandMessage(playerId, message, value, SCR_BaseGameMode.playersKills[playerName]);
				return;
			}
			
			if (message == "!clearkills") {
				SCR_BaseGameMode.playersKills[playerName] = {};
				return;
			}
			
			if (message == Premium.playerRankItemCommand()) {
				int itemsCount = SCR_BaseGameMode.itemRank.GetItemsStolen(playerName);
				SendCommandMessage(playerId, Premium.playerRankItemDescription() + ": " + itemsCount);	
				return;
			}
			
			if (message.Contains("!itemstolen")) {
				message.Replace("!itemstolen", "");
			
				string msg = string.Format(Premium.playerRankItemStolenMessage(), message);
				SendCommandMessage(playerId, "!itemstolen" + msg);
				return;
			}
			
			if (message.Contains("!itemdelivered")) {
				message.Replace("!itemdelivered", "");
				
				SCR_BaseGameMode.itemRank.AddStolenItem(playerName);
				SCR_BaseGameMode.playersKills[playerName].RemoveItem(message);
				
				string msg = string.Format(Premium.playerRankItemDeliveryMessage(), message);
				SendCommandMessage(playerId, "!itemdelivered" + msg);
				return;
			}
		}
		
		if (message.Contains("!link")) {
			message.Replace("!link", "");
			message.Replace("@", "");
			message.Replace(" ", "");
			message.Replace("\"", "");
			message.Replace("\\", "");
			message.Replace("<", "");
			message.Replace(">", "");
			message.Replace("|", "");
			message.Replace("#", "");
			message.Replace("{", "");
			message.Replace("}", "");
			message.Replace("[", "");
			message.Replace("]", "");
			message.Replace("‎ ", "");
			message.Replace("‎`", "");
			message.Replace("/", "");
			message.Replace("_", "");

			if (message.Length() > 0) {
				SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
				gameMode.AddPlayer(playerName, message);
				SendCommandMessage(playerId, "Discord linked Succefully!");
			} else
				SendCommandMessage(playerId, "Fail to link Discord.");
			
			return;
		}	
	}
	
	bool SendToOwnerCommandStatus(int playerId, string message, bool value, array<string> customMessage) {
		Rpc(ReceiveNotificationCommandStatus, playerId, message, value, customMessage);
		return true;
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void ReceiveNotificationCommandStatus(int playerId, string message, bool value, array<string> customMessage) {
		if (message == "!clearkills")
			return;
		else if (message == "!seedingrules")
			SCR_PlayerController.SetSeedingRules(value);
		else if (message == "!armband")
			SCR_InventorySlotUI.isArmbandLocked = value;
		else if (message == "!itemnames")
			SCR_InventoryStorageManagerComponent.itemNames = customMessage;
		else if (message == "!playerkills") {
			SCR_InventoryStorageManagerComponent.SetPlayerKills(playerId, customMessage);
		} else if (message.Contains("!itemdelivered") || message.Contains("!itemstolen")) {
			message.Replace("!itemdelivered", "");
			message.Replace("!itemstolen", "");
			SCR_InventoryStorageManagerComponent.SendMessage(message, playerId);
		} else
			SCR_ChatComponent.NewMessage(playerId, message);
	}
	
	static bool RequestChannel(
		int playerId, 
		bool forChat,
		string _channelId = string.Empty, 
		string _webhook = string.Empty, 
		string adminId = string.Empty, 
		bool isPremium = false, 
		bool showAllChatChannels = false,
		bool notifyGameMaster = false,
		string itemCommand = string.Empty
	) {
		if (Replication.IsServer()) { 
			SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId));
			
			if (!playerController)
				return false;
			
			SCR_NotificationsComponent notification = SCR_NotificationsComponent.Cast(playerController.FindComponent(SCR_NotificationsComponent));
			
			if (!notification)
				return false;
			
			notification.SendToOwnerChannel(forChat, _channelId, _webhook, adminId, isPremium, showAllChatChannels, notifyGameMaster, itemCommand);
			return true;
		} else {
			SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerId));
			
			if (!playerController)
				return false;
			
			SCR_NotificationsComponent notification = SCR_NotificationsComponent.Cast(playerController.FindComponent(SCR_NotificationsComponent));
			
			if (!notification)
				return false;
			
			notification.Rpc(notification.Rpc_RequestChannel, playerId, forChat);
			return true;
		}
		
		return true;
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void Rpc_RequestChannel(int playerId, bool forChat) {
		string _channelId = string.Empty;
		string _webhookToken = string.Empty;
		string adminId = string.Empty;
		
		SCR_JsonLoadContext loadContext = new SCR_JsonLoadContext();
		string filepath = "$profile:discord_status_chat.json";
		loadContext.LoadFromFile(filepath);
		loadContext.ReadValue("adminRoleId", adminId);
		
		if (forChat) {
			string chatChannel;
			loadContext.ReadValue("chatChannel", chatChannel);
			
			if (chatChannel != string.Empty) {
				array<string> chat = new array<string>();
				chatChannel.Split("/", chat, true);
					
				_channelId = chat[0];
				_webhookToken = chat[1];
			}
		} else {
			SCR_JsonLoadContext loadContext1 = new SCR_JsonLoadContext();
			string filepath1 = "$profile:discord_premium_channels.json";
			loadContext1.LoadFromFile(filepath1);
			
			string adminLogsChannel;
			loadContext1.ReadValue("adminLogsChannel", adminLogsChannel);
			
			if (adminLogsChannel != string.Empty) {
				array<string> channel = new array<string>();
				adminLogsChannel.Split("/", channel, true);
					
				_channelId = channel[0];
				_webhookToken = channel[1];
			}
		}
		
		bool isPremium = Premium.isPremium();
		bool showAllChatChannels = Premium.showAllChatChannels();
		bool notifyGameMaster = Premium.adminNotifyOpeningGameMaster();
		string itemCommand = Premium.playerRankItemCommand();
		
		RequestChannel(playerId, forChat, _channelId, _webhookToken, adminId, isPremium, showAllChatChannels, notifyGameMaster, itemCommand);
	}
	
	bool SendToOwnerChannel(
		bool forChat, 
		string _channelId, 
		string _webhook, 
		string adminId, 
		bool isPremium, 
		bool showAllChatChannels, 
		bool notifyGameMaster, 
		string itemCommand
	) {
		Rpc(ReceiveNotificationChannel, forChat, _channelId, _webhook, adminId, isPremium, showAllChatChannels, notifyGameMaster, itemCommand);
		return true;
	}
	
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void ReceiveNotificationChannel(
		bool forChat, 
		string _channelId, 
		string _webhook, 
		string adminId, 
		bool isPremium, 
		bool showAllChatChannels, 
		bool notifyGameMaster, 
		string itemCommand
	) {
		if(forChat) {
			SCR_ChatComponent.chatChannelId = _channelId;
			SCR_ChatComponent.chatWebhookToken = _webhook;
			SCR_ChatComponent.adminRoleId = adminId;
			SCR_ChatComponent.isPremium = isPremium;
			SCR_ChatComponent.showAllChatChannels =  showAllChatChannels;
			SCR_ChatComponent.itemCommand = itemCommand;
		} else {
			SCR_EditorManagerEntity.channelId = _channelId;
			SCR_EditorManagerEntity.webhookToken = _webhook;
			SCR_EditorManagerEntity.adminRoleId = adminId;
			SCR_EditorManagerEntity.notifyGameMaster = notifyGameMaster;
		}
	}
}