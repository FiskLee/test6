modded class SCR_BaseGameMode {
	static ref map<string, ref array<string>> playersKills = new map<string, ref array<string>>();

	private RestContext restKillFeedContext;
	private ref PVPToDiscord killPVPToDiscord;
	
	private string killFeedChannelId = string.Empty;
	private string killFeedWebhookToken = string.Empty;	
	
	private string rankPVPChannelId = string.Empty;
	private string rankPVPWebhookToken = string.Empty;
	
	override void StartGameMode() {
		super.StartGameMode();
		
		if (!IsMaster())
			return;
		
		GetOnPlayerKilled().Insert(OnPlayersKilled);
		GetOnControllableDestroyed().Insert(OnAiKilled);
		
		SCR_JsonLoadContext loadContext = new SCR_JsonLoadContext();
		string filepath = "$profile:discord_pvp_killfeed.json";
		
		if (!loadContext.LoadFromFile(filepath)) {
			SCR_JsonSaveContext saveContext = new SCR_JsonSaveContext();
			saveContext.WriteValue("pvpChannel", "");
			saveContext.WriteValue("killFeedChannel", "");
			saveContext.SaveToFile(filepath);
			return;
		}
		
		string killFeedChannel;
		loadContext.ReadValue("killFeedChannel", killFeedChannel);
		
		if (killFeedChannel != string.Empty) {
			array<string> killfeed = new array<string>();
			killFeedChannel.Split("/", killfeed, true);
			killFeedChannelId = killfeed[0];
			killFeedWebhookToken = killfeed[1];
		}
		
		string killFeedUrl = string.Format("https://discord.com/api/webhooks/%1/", killFeedChannelId);
		restKillFeedContext = GetGame().GetRestApi().GetContext(killFeedUrl);
		restKillFeedContext.SetHeaders("Content-Type, application/json");
		
		string pvpChannel;
		loadContext.ReadValue("pvpChannel", pvpChannel);
		
		if (pvpChannel != string.Empty) {
			array<string> pvp = new array<string>();
			pvpChannel.Split("/", pvp, true);
			
			rankPVPChannelId = pvp[0];
			rankPVPWebhookToken = pvp[1];
		}
		
		killPVPToDiscord = new PVPToDiscord(rankPVPChannelId, rankPVPWebhookToken);
	}
	
	protected void OnPlayersKilled(notnull SCR_InstigatorContextData instigatorContextData) {
		if (instigatorContextData.GetKillerCharacterControlType() == SCR_ECharacterControlType.AI || 
			instigatorContextData.GetVictimCharacterControlType() == SCR_ECharacterControlType.AI) {
			return;
		}
		
		//~ Player killed self
		if (instigatorContextData.HasAnyVictimKillerRelation(SCR_ECharacterDeathStatusRelations.SUICIDE))
		{
			int victimId = instigatorContextData.GetVictimPlayerID();
			killPVPToDiscord.OnPlayerKilled(-1, victimId);
			
			string victimName = GetGame().GetPlayerManager().GetPlayerName(victimId);
			
			if (victimName) {
				SCR_BaseGameMode.playersKills[victimName] = {};
			}
			
			return;
		}
		
		//~ Killed by enemy player. 
		if (instigatorContextData.HasAnyVictimKillerRelation(SCR_ECharacterDeathStatusRelations.KILLED_BY_ENEMY_PLAYER))
		{
			int killerId = instigatorContextData.GetKillerPlayerID();
			int victimId = instigatorContextData.GetVictimPlayerID();
			killPVPToDiscord.OnPlayerKilled(killerId, victimId);
			
			string killerName = GetGame().GetPlayerManager().GetPlayerName(killerId);
			string victimName = GetGame().GetPlayerManager().GetPlayerName(victimId);
			
			if(killerName && victimName) {
				if (SCR_BaseGameMode.playersKills[killerName]) {
					if (!SCR_BaseGameMode.playersKills[killerName].Contains(victimName))
						SCR_BaseGameMode.playersKills[killerName].Insert(victimName);
				} else
					SCR_BaseGameMode.playersKills[killerName] = {victimName};
			}
			
			return;
		}
			
		//~ Killed by friendly player.
		if (instigatorContextData.HasAnyVictimKillerRelation(SCR_ECharacterDeathStatusRelations.KILLED_BY_FRIENDLY_PLAYER))
		{
			int killerId = instigatorContextData.GetKillerPlayerID();
			string killerColor = GetFactionByPlayerId(killerId);
			string killerName = GetGame().GetPlayerManager().GetPlayerName(killerId);
			
			int victimId = instigatorContextData.GetVictimPlayerID();
			string victimColor = GetFactionByPlayerId(victimId);
			string victimName = GetGame().GetPlayerManager().GetPlayerName(victimId);
			
			string message = "> " + killerColor + " **" + killerName + "** Team-Killed " + victimColor + " **" + victimName + "**";
			string formattedMessage = string.Format("{\"content\": \"%1\"}", message);
			
			if (killFeedWebhookToken != string.Empty)
				restKillFeedContext.POST_now(killFeedWebhookToken, formattedMessage);
			
			killPVPToDiscord.OnPlayerKilled(-1, victimId);

			if (victimName) {
				SCR_BaseGameMode.playersKills[victimName] = {};
			}
			
			return;
		}
	}
	
	protected void OnAiKilled(notnull SCR_InstigatorContextData instigatorContextData) {
		if (!Premium.AiKillCountIsEnabled())
			return;
		
		if (instigatorContextData.GetKillerCharacterControlType() == SCR_ECharacterControlType.AI) {
			int victimId = instigatorContextData.GetVictimPlayerID();
			killPVPToDiscord.OnPlayerKilled(-1, victimId);
			
			string victimName = GetGame().GetPlayerManager().GetPlayerName(victimId);
		
			if (victimName)
				SCR_BaseGameMode.playersKills[victimName] = {};
			
			return;
		}
		
		if (instigatorContextData.GetVictimCharacterControlType() == SCR_ECharacterControlType.AI) {
			int killerId = instigatorContextData.GetKillerPlayerID();
			killPVPToDiscord.OnPlayerKilled(killerId, -1);	
			return;
		}
	}
	
	private string GetFactionByPlayerId(int playerId) {
		int factionIndex = GetGame().GetFactionManager().GetFactionIndex(SCR_FactionManager.SGetPlayerFaction(playerId));
		
		if (factionIndex == 0)
			return ":blue_circle:";
		else
			return ":red_circle:";
	}
}