modded class SCR_VotingManagerComponent {
	private RestContext restContext;
	private string channelId = string.Empty;
	private string webhookToken = string.Empty;
	private string adminRoleId = string.Empty;
	
	override void Vote(int playerID, EVotingType type, int value) {
		if (!GetGame().GetPlayerManager().IsPlayerConnected(playerID))
			return;
		
		if (restContext == null) {
			SCR_JsonLoadContext loadContext = new SCR_JsonLoadContext();
			string filepath = "$profile:discord_status_chat.json";
			loadContext.LoadFromFile(filepath);
			string chatChannel
			loadContext.ReadValue("chatChannel", chatChannel);
			loadContext.ReadValue("adminRoleId", adminRoleId);
			
			
			if (chatChannel != string.Empty) {
				array<string> chat = new array<string>();
				chatChannel.Split("/", chat, true);
					
				channelId = chat[0];
				webhookToken = chat[1];
			}
			
			string url = string.Format("https://discord.com/api/webhooks/%1/", channelId);
			restContext = GetGame().GetRestApi().GetContext(url);
			restContext.SetHeaders("Content-Type, application/json");
		}
		
		if (type != EVotingType.KICK || type != EVotingType.AUTO_KICK)
			return;
		
		if (webhookToken == string.Empty)
			return;
	
		if (!FindVoting(type, value)) {
			string playerName = GetGame().GetPlayerManager().GetPlayerName(playerID);
			string playerKickName = GetGame().GetPlayerManager().GetPlayerName(value);
			
			string discordName = "";
			string discordKickName = "";
			string adminName = "";
			
			if (DiscordPlayer.players[playerName] != string.Empty)
				discordName = "(@" + DiscordPlayer.players[playerName] + ") ";
			
			if (DiscordPlayer.players[playerKickName] != string.Empty)
				discordKickName = "(@" + DiscordPlayer.players[playerKickName] + ") ";
			
			if (adminRoleId != string.Empty)
				adminName = "<@&" + adminRoleId + ">: ";
			
			string message = adminName + discordName + playerName + " started a vote kick against " + discordKickName + playerKickName;
			string formattedMessage = string.Format("{\"content\": \"%1\"}", message);

			restContext.POST_now(webhookToken, formattedMessage);
		}
	}
}