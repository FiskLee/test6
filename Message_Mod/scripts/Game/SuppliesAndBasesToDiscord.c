modded class SCR_XPHandlerComponent {
	static ref map<string, ref SupplyStats> supplyStats = new map<string, ref SupplyStats>();
	static ref map<string, ref BaseStats> baseStats = new map<string, ref BaseStats>();

	private RestContext supply_restContext;
	private string supply_channelId = string.Empty;
	private string supply_webhookToken = string.Empty;
	private ref WDN_RestCallback supplyCallback;
	
	private RestContext base_restContext;
	private string base_channelId = string.Empty;
	private string base_webhookToken = string.Empty;
	private ref WDN_RestCallback baseCallback;
	
	private string matchNumber;
	private bool didConfigure = false;
	private bool gameDidEnd = false;
	
	override void OnPostInit(IEntity owner) {
		super.OnPostInit(owner);
		
		if(IsProxy())
			return;
		
		if (!didConfigure) { 
			didConfigure = true;
			matchNumber = MatchNumber.GetMatchNumber();
			
			SCR_JsonLoadContext loadContext = new SCR_JsonLoadContext();
			string filepath = "$profile:discord_supply_bases.json";
			
			if (!loadContext.LoadFromFile(filepath)) {
				SCR_JsonSaveContext saveContext = new SCR_JsonSaveContext();
				saveContext.WriteValue("supplyChannel", "");
				saveContext.WriteValue("baseChannel", "");
				saveContext.SaveToFile(filepath);
				return;
			}
			
			string supplyChannel;
			loadContext.ReadValue("supplyChannel", supplyChannel);
			
			if (supplyChannel != string.Empty) {
				array<string> supply = new array<string>();
				supplyChannel.Split("/", supply, true);
					
				supply_channelId = supply[0];
				supply_webhookToken = supply[1];
			}
			
			string url_supply = string.Format("https://discord.com/api/webhooks/%1/", supply_channelId);
			supply_restContext = GetGame().GetRestApi().GetContext(url_supply);
			supply_restContext.SetHeaders("Content-Type, application/json");
			supplyCallback = new WDN_RestCallback("$profile:messageId_supply.txt");
			
			string baseChannel;
			loadContext.ReadValue("baseChannel", baseChannel);
			
			if (baseChannel != string.Empty) {
				array<string> base = new array<string>();
				baseChannel.Split("/", base, true);
			
				base_channelId = base[0];
				base_webhookToken = base[1];
			}
			
			string url_base = string.Format("https://discord.com/api/webhooks/%1/", base_channelId);
			base_restContext = GetGame().GetRestApi().GetContext(url_base);
			base_restContext.SetHeaders("Content-Type, application/json");
			baseCallback = new WDN_RestCallback("$profile:messageId_base.txt");
			
			
			SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
			gameMode.GetOnGameModeEnd().Insert(GetOnGameModeEnd);
			
			GetGame().GetCallqueue().CallLater(SetupRankSupliesAndBases, 30000, false);
		}
	}
	
	private void SetupRankSupliesAndBases() {
		GetGame().GetCallqueue().CallLater(UpdateLeaderboardSupply, Premium.rankTableUpdateTime(), true);
		GetGame().GetCallqueue().CallLater(UpdateLeaderboardBase, Premium.rankTableUpdateTime(), true);
	}
	
	protected void GetOnGameModeEnd() {
		GetGame().GetCallqueue().Remove(UpdateLeaderboardSupply);
		GetGame().GetCallqueue().Remove(UpdateLeaderboardBase);
		
		if (GetGame().GetPlayerManager().GetPlayerCount() < Premium.rankNumberOfPlayers()) {
			if (supplyCallback.messageId)
				supply_restContext.DELETE_now(supply_webhookToken + "/messages/" + supplyCallback.messageId, string.Empty);
		
			if (baseCallback.messageId)
				base_restContext.DELETE_now(base_webhookToken + "/messages/" + baseCallback.messageId, string.Empty);
		} else {
			gameDidEnd = true;
			GetGame().GetCallqueue().CallLater(UpdateLeaderboardSupply, 1500, false);
			GetGame().GetCallqueue().CallLater(UpdateLeaderboardBase, 3000, false);
		}
	}
	
	override void AwardXP(int playerId, SCR_EXPRewards rewardID, float multiplier = 1.0, bool volunteer = false, int customXP = 0) {
		super.AwardXP(playerId, rewardID, multiplier, volunteer, customXP);
		
		if (IsProxy())
			return;
		
		if (GetGame().GetPlayerManager().GetPlayerCount() < Premium.rankNumberOfPlayers()) {
			return;
		}
		
		int pointsGained = Math.Round(100 * multiplier);
		
		string playerName = GetGame().GetPlayerManager().GetPlayerName(playerId);
		
		if(rewardID == SCR_EXPRewards.SUPPLIES_DELIVERED) {
			if (SCR_XPHandlerComponent.supplyStats[playerName] == null) {
				SCR_XPHandlerComponent.supplyStats.Insert(playerName, new SupplyStats(playerName, pointsGained));
			} else {
				SCR_XPHandlerComponent.supplyStats[playerName].addPoints(pointsGained);
			}
		} else if (rewardID == SCR_EXPRewards.BASE_SEIZED) {
			if (SCR_XPHandlerComponent.baseStats[playerName] == null) {
				SCR_XPHandlerComponent.baseStats.Insert(playerName, new BaseStats(playerName, pointsGained));
			} else {
				SCR_XPHandlerComponent.baseStats[playerName].addPoints(pointsGained);
			}
		}
	}
	
    protected void UpdateLeaderboardSupply() {
		if (supply_webhookToken == string.Empty)
			return;
		
	    ref array<ref SupplyStats> suppliers = new array<ref SupplyStats>();

        foreach (string _, SupplyStats stats : SCR_XPHandlerComponent.supplyStats) {
           	suppliers.Insert(stats);
		}
		
		int biggestSupplierIndex;
		SupplyStats biggestSupplier;
		ref array<ref SupplyStats> sortedSuppliers = new array<ref SupplyStats>();
		
		int numberOfPlayers = Premium.rankNumberOfPlayersInTable();
		
		if (gameDidEnd)
			numberOfPlayers = Premium.rankNumberOfPlayersInFinalTable();
		
		while (suppliers.Count() > 0 && sortedSuppliers.Count() < numberOfPlayers) {
			for(int index = 0; index < suppliers.Count(); index++) {
				SupplyStats supplier = suppliers.Get(index);
				
				if (biggestSupplier == null) {
					biggestSupplier = supplier;
					biggestSupplierIndex = index;
				} else if (supplier.amount > biggestSupplier.amount) {
					biggestSupplier = supplier;
					biggestSupplierIndex = index;
				}
			}
			
			suppliers.Remove(biggestSupplierIndex);
			sortedSuppliers.Insert(new SupplyStats(biggestSupplier.name, biggestSupplier.amount));
			biggestSupplier = null;
		}
			
		string positions = "";
     	string players = "";
		string stats = "";
		
		for (int i = 0; i < sortedSuppliers.Count(); i++) {
            SupplyStats supply = sortedSuppliers.Get(i);
			
			positions += string.Format("```%1```", i + 1);
			
			if(Premium.isPremium() && DiscordPlayer.players[supply.name] != string.Empty) {
            	string name = string.Format("@%1 | %2", DiscordPlayer.players[supply.name], supply.name);
				
				if (name.Length() > 28) {
					name = name.Substring(0, 28);
				}
				
				players += "```" + name + "```";;
			} else
				players += string.Format("```%1```", supply.name);
			
			stats += string.Format("```%1```", supply.amount);
        }
		
		if (supplyCallback.messageId)
			supply_restContext.DELETE_now(supply_webhookToken + "/messages/" + supplyCallback.messageId, string.Empty);	
		
		if (gameDidEnd)
			supply_restContext.POST_now(supply_webhookToken, GetEmbedPayloadSupply(positions, players, stats));
		else
			supply_restContext.POST(supplyCallback, supply_webhookToken + "?wait=true", GetEmbedPayloadSupply(positions, players, stats));
    }
	
	private string GetEmbedPayloadSupply(string positions, string players, string stats) {
		if (gameDidEnd) {
			return string.Format("{\"embeds\": [{\"title\": \"───────────────── :trophy: TOP %1 • MATCH #%2 ─────────────────\"" + ","
				+ "\"color\": 15258703" + ","
				+ "\"fields\": [{\"name\": \"Position\", \"value\": \"%3\", \"inline\": true},"
				+ "{\"name\": \"Player\", \"value\": \"%4\", \"inline\": true},"
				+ "{\"name\": \"Supply Points\", \"value\": \"%5\", \"inline\": true}]"
				+ "}]}", Premium.rankNumberOfPlayersInFinalTable().ToString(), matchNumber, positions, players, stats);
		} else {
			return string.Format("{\"embeds\": [{\"title\": \"──────────────────── Top %1 Players ────────────────────\"" + "," 
				+ "\"color\": 2243554" + ","
				+ "\"fields\": [{\"name\": \"Position\", \"value\": \"%2\", \"inline\": true},"
				+ "{\"name\": \"Player\", \"value\": \"%3\", \"inline\": true},"
				+ "{\"name\": \"Supply Points\", \"value\": \"%4\", \"inline\": true}]"
				+ "}]}", Premium.rankNumberOfPlayersInTable().ToString(), positions, players, stats);
		}
	}
	
	protected void UpdateLeaderboardBase() {
		if (base_webhookToken == string.Empty)
			return;
		
	    ref array<ref BaseStats> bases = new array<ref BaseStats>();

        foreach (string _, BaseStats base : SCR_XPHandlerComponent.baseStats) {
           	bases.Insert(base);
		}
		
		int biggestBaseIndex;
		BaseStats biggestBase;
		ref array<ref BaseStats> sortedBases = new array<ref BaseStats>();
		
		int numberOfPlayers = Premium.rankNumberOfPlayersInTable();
		
		if (gameDidEnd)
			numberOfPlayers = Premium.rankNumberOfPlayersInFinalTable();
		
		while (bases.Count() > 0 && sortedBases.Count() < numberOfPlayers) {
			for(int index = 0; index < bases.Count(); index++) {
				BaseStats base = bases.Get(index);
				
				if (biggestBase == null) {
					biggestBase = base;
					biggestBaseIndex = index;
				} else if (base.seized > biggestBase.seized ) {
					biggestBase = base;
					biggestBaseIndex = index;
				} else if (base.seized == biggestBase.seized && base.points > biggestBase.points) {
					biggestBase = base;
					biggestBaseIndex = index;
				}
			}
			
			bases.Remove(biggestBaseIndex);
			sortedBases.Insert(new BaseStats(biggestBase.name, biggestBase.points, biggestBase.seized));
			biggestBase = null;
		}
			
		string positions = "";
     	string players = "";
		string stats = "";
		
		for (int i = 0; i < sortedBases.Count(); i++) {
            BaseStats base = sortedBases.Get(i);
			
			positions += string.Format("```%1```", i + 1);
			
			if(Premium.isPremium() && DiscordPlayer.players[base.name] != string.Empty) {
            	string name = string.Format("@%1 | %2", DiscordPlayer.players[base.name], base.name);
				
				if (name.Length() > 28) {
					name = name.Substring(0, 28);
				}
				
				players += "```" + name + "```";;
			} else
				players += string.Format("```%1```", base.name);
			
			stats += string.Format("```%1 (%2)```", base.seized, base.points);
        }
		
		if (baseCallback.messageId)
			base_restContext.DELETE_now(base_webhookToken + "/messages/" + baseCallback.messageId, string.Empty);
		
		if (gameDidEnd)
			base_restContext.POST_now(base_webhookToken, GetEmbedPayloadBase(positions, players, stats));
		else
			base_restContext.POST(baseCallback, base_webhookToken + "?wait=true", GetEmbedPayloadBase(positions, players, stats));
    }
	
	private string GetEmbedPayloadBase(string positions, string players, string stats) {
		if (gameDidEnd) {
			return string.Format("{\"embeds\": [{\"title\": \"───────────────── :trophy: TOP %1 • MATCH #%2 ─────────────────\"" + ","
				+ "\"color\": 15258703" + ","
				+ "\"fields\": [{\"name\": \"Position\", \"value\": \"%3\", \"inline\": true},"
				+ "{\"name\": \"Player\", \"value\": \"%4\", \"inline\": true},"
				+ "{\"name\": \"Bases Seized / Points\", \"value\": \"%5\", \"inline\": true}]"
				+ "}]}", Premium.rankNumberOfPlayersInFinalTable().ToString(), matchNumber, positions, players, stats);
		} else {
			return string.Format("{\"embeds\": [{\"title\": \"──────────────────── Top %1 Players ────────────────────\"" + "," 
				+ "\"color\": 2243554" + ","
				+ "\"fields\": [{\"name\": \"Position\", \"value\": \"%2\", \"inline\": true},"
				+ "{\"name\": \"Player\", \"value\": \"%3\", \"inline\": true},"
				+ "{\"name\": \"Bases Seized / Points\", \"value\": \"%4\", \"inline\": true}]"
				+ "}]}", Premium.rankNumberOfPlayersInTable().ToString(), positions, players, stats);
		}
	}
}

class SupplyStats {
	string name = "";
	int amount = 0;
	
	void SupplyStats(string Name, int Amount) {
		this.name = Name;
		this.amount = Amount;
	}
	
	void addPoints(int amt) {
		amount = amount + amt;
	}
}

class BaseStats {
	string name = "";
	int seized = 0;
	int points = 0;
	
	void BaseStats(string Name, int Points, int Seized = 1) {
		this.name = Name;
		this.points = Points;
		this.seized = Seized;
	}
	
	void addPoints(int pts) {
		seized++;
		points = points + pts;
	}
}