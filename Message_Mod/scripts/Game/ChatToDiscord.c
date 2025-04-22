modded class SCR_ChatComponent {
	static bool isPremium = false;
	static bool showAllChatChannels = false;
	static string chatChannelId = string.Empty;
	static string chatWebhookToken = string.Empty;
	static string adminRoleId = string.Empty;
	static string itemCommand = string.Empty;
	
	private RestContext restContext;
	private string playerMessage;
	
	private bool didTrySetupRest = false;
	private bool didStartSetupRest = false;
	
    override void OnNewMessage(string msg, int channelId, int senderId) {
		super.OnNewMessage(msg, channelId, senderId);

		if (GetGame().GetPlayerController().GetPlayerId() != senderId)
			return;
		
		if (restContext == null && didTrySetupRest == false) {
			if(didStartSetupRest)
				return;
			
			didStartSetupRest = true;
			playerMessage = msg;
			SCR_NotificationsComponent.RequestChannel(senderId, true);
			GetGame().GetCallqueue().CallLater(SetupChatRest, 1500, false);
			return;
		}
		
		if (SCR_ChatComponent.showAllChatChannels || channelId == 0) {
			ProcessChatMessage(msg);
		}
		
		Commands(msg);
    }
	
	protected void SetupChatRest() {
		didTrySetupRest = true;
		
		if (SCR_ChatComponent.chatChannelId != string.Empty) {
			string url = string.Format("https://discord.com/api/webhooks/%1/", SCR_ChatComponent.chatChannelId);
			restContext = GetGame().GetRestApi().GetContext(url);
			restContext.SetHeaders("Content-Type, application/json");
			ProcessChatMessage(playerMessage);
		}
		
		Commands(playerMessage);
	}

    private void ProcessChatMessage(string message) {
		if (SCR_ChatComponent.chatWebhookToken == string.Empty)
			return;
		
		int senderId = GetGame().GetPlayerController().GetPlayerId();

		string playerColor = GetFactionByPlayerId(senderId);
		string playerName = GetGame().GetPlayerManager().GetPlayerName(senderId);
		string formattedMessage = playerColor + " **" + playerName + ":** " + message;
		
		message.ToLower();
		
		if (message.Contains("!admin") && SCR_ChatComponent.isPremium && SCR_ChatComponent.adminRoleId != string.Empty) {
			formattedMessage = "<@&" + SCR_ChatComponent.adminRoleId + ">" + ": " + formattedMessage;
		}
		
		formattedMessage = "> " + formattedMessage;
		
		string msg = string.Format("{\"content\": \"%1\"}", formattedMessage);
	   	restContext.POST_now(SCR_ChatComponent.chatWebhookToken, msg);
    }
	
	private string GetFactionByPlayerId(int playerId) {
		int factionIndex = GetGame().GetFactionManager().GetFactionIndex(SCR_FactionManager.SGetPlayerFaction(playerId));
		
		if (factionIndex == 0)
			return ":blue_circle:";
		else
			return ":red_circle:";
	}
	
	private void Commands(string msg) {
		int senderId = GetGame().GetPlayerController().GetPlayerId();
		
		if (msg.StartsWith("!link") || msg.StartsWith("!Link")) {
			SCR_NotificationsComponent.SendCommandMessage(senderId, msg);
			return;
		}
		
		if (msg.StartsWith("!seeding")) {
			SCR_NotificationsComponent.SendCommandMessage(senderId, msg);
			return;
		}
		 
		msg.ToLower();

		if (msg == "!kills" || msg == "!rank" || msg == "!killrank" || msg == itemCommand) {
			SCR_NotificationsComponent.SendCommandMessage(senderId, msg);
			return;
		}
	}
	
	static void NewMessage(int playerId, string message) {
		if (!SCR_ChatComponent.isPremium)
			return;
		
		SCR_ChatPanelManager chatManager = SCR_ChatPanelManager.GetInstance();
		chatManager.OnNewMessagePrivate(message, playerId, playerId);
	}
}