modded class SCR_EditorManagerEntity {
	static bool notifyGameMaster = false;
	static string channelId = string.Empty;
	static string webhookToken = string.Empty;
	static string adminRoleId = string.Empty;
	
	private RestContext restContext;
	private bool isAdminOpen = false;
	
	override void StartEvents(EEditorEventOperation type = EEditorEventOperation.NONE) {
		super.StartEvents(type);
	
		if (type == EEditorEventOperation.INIT) {
			SCR_NotificationsComponent.RequestChannel(GetGame().GetPlayerController().GetPlayerId(), false);
			GetGame().GetCallqueue().CallLater(SetupChatRest, 1500, false);
		}
	}
	
	protected void SetupChatRest() {
		if (SCR_EditorManagerEntity.channelId == string.Empty)
			return;
		
		string url = string.Format("https://discord.com/api/webhooks/%1/", SCR_EditorManagerEntity.channelId);
		restContext = GetGame().GetRestApi().GetContext(url);
		restContext.SetHeaders("Content-Type, application/json");
	}
	
	override void Open(bool showErrorNotification = true) {
		super.Open(showErrorNotification);
		
		bool isAdmin = m_CurrentMode == EEditorMode.ADMIN || m_CurrentMode == EEditorMode.EDIT;
		
		if(!isAdmin)
			return;
		
		isAdminOpen = true;
		
		if (SCR_EditorManagerEntity.webhookToken == string.Empty)
			return;
		
		int senderId = GetGame().GetPlayerController().GetPlayerId();

		string playerColor = GetFactionByPlayerId(senderId);
		string playerName = GetGame().GetPlayerManager().GetPlayerName(senderId);
		string message = playerColor + " **" + playerName + ":** " + "Opened Game Master.";

		if (SCR_EditorManagerEntity.adminRoleId != string.Empty && SCR_EditorManagerEntity.notifyGameMaster) {
			message = "<@&" + SCR_EditorManagerEntity.adminRoleId + ">" + ": " + message;
		}
		
		message = "> " + message;
		
		string msg = string.Format("{\"content\": \"%1\"}", message);
	   	restContext.POST_now(SCR_EditorManagerEntity.webhookToken, msg);
	}
	
	override void Close(bool showErrorNotification = true) {
		super.Close(showErrorNotification);
		
		if(!isAdminOpen)
			return;
		
		isAdminOpen = false;
		
		if (SCR_EditorManagerEntity.webhookToken == string.Empty)
			return;
		
		int senderId = GetGame().GetPlayerController().GetPlayerId();

		string playerColor = GetFactionByPlayerId(senderId);
		string playerName = GetGame().GetPlayerManager().GetPlayerName(senderId);
		string message = "> " + playerColor + " **" + playerName + ":** " + "Closed Game Master.";
		string msg = string.Format("{\"content\": \"%1\"}", message);
	   	restContext.POST_now(SCR_EditorManagerEntity.webhookToken, msg);
	}
	
	private string GetFactionByPlayerId(int playerId) {
		int factionIndex = GetGame().GetFactionManager().GetFactionIndex(SCR_FactionManager.SGetPlayerFaction(playerId));
		
		if (factionIndex == 0)
			return ":blue_circle:";
		else
			return ":red_circle:";
	}
}