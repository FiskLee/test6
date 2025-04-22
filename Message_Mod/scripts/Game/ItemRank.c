modded class SCR_BaseGameMode {
	static ref ItemRank itemRank;
	
	override void StartGameMode() {
		super.StartGameMode();
		
		if (!IsMaster())
			return;
		
		itemRank = new ItemRank();
	}
}

class ItemRank {
	private ref map<string, ref PlayerItems> players = new map<string, ref PlayerItems>();
	
	private const string cryptograph = "::/::";
	private const string item_path = "$profile:player_items.txt";
	private const string filepath = "$profile:discord_premium_channels.json";
	
	private ref WDN_RestCallback callback;
	private RestContext restContext;
	private string channelId = string.Empty;
	private string webhookToken = string.Empty;
	
	void ItemRank() {
		callback = new WDN_RestCallback("$profile:messageId_item.txt");
		
		SCR_JsonLoadContext loadContext = new SCR_JsonLoadContext();
		
		if (!loadContext.LoadFromFile(filepath)) {
			SCR_JsonSaveContext saveContext = new SCR_JsonSaveContext();
			saveContext.WriteValue("overallRankChannel", "");
			saveContext.WriteValue("pvpRankChannel", "");
			saveContext.WriteValue("itemChannel", "");
			saveContext.WriteValue("adminLogsChannel", "");
			saveContext.SaveToFile(filepath);
			return;
		}
		
		loadContext.LoadFromFile(filepath);
		
		string itemChannel;
		loadContext.ReadValue("itemChannel", itemChannel);
		
		if (itemChannel != string.Empty) {
			array<string> channel = new array<string>();
			itemChannel.Split("/", channel, true);
				
			channelId = channel[0];
			webhookToken = channel[1];
		}
		
		string url = string.Format("https://discord.com/api/webhooks/%1/", channelId);
		restContext = GetGame().GetRestApi().GetContext(url);
		restContext.SetHeaders("Content-Type, application/json"); 
		
		FileHandle textFileR = FileIO.OpenFile(item_path, FileMode.READ);
		
		if (textFileR) {
			string line = string.Empty;
			
			while(textFileR.ReadLine(line) >= 0) {
				array<string> player = new array<string>();
				line.Split(cryptograph, player, true);
				
				if (player.Count() != 2)
					continue;
				
				string playerName = player[0];
				int items = player[1].ToInt();
								
				players.Insert(playerName, PlayerItems(playerName, items));
			}
			
			textFileR.Close();
		}
	}
	
	void AddStolenItem(string playerName) {
		if (GetGame().GetPlayerManager().GetPlayerCount() < Premium.rankNumberOfPlayers()) {
			return;
		}
		
		if (players[playerName])
			players[playerName].addItem();
		else
			players[playerName] = PlayerItems(playerName, 1);
		
		SaveToFile();
		UpdateLeaderboard();
	}
	
	int GetItemsStolen(string playerName) {
		if (players[playerName])
			return players[playerName].items;
		else 
			return 0;
	}
	
    protected void UpdateLeaderboard() {
		if (webhookToken == string.Empty)
			return;
		
		ref array<ref PlayerItems> playersItems = new array<ref PlayerItems>();
		
		foreach(string _, PlayerItems player: players) {
			playersItems.Insert(PlayerItems(player.name, player.items));
		}
		
		int topItemsIndex;
		PlayerItems topItems;
		ref array<ref PlayerItems> sortedPlayers = new array<ref PlayerItems>();
		
		while (playersItems.Count() > 0 && sortedPlayers.Count() < Premium.playerRankItemNumberOfPlayers()) {
			for(int index = 0; index < playersItems.Count(); index++) {
				PlayerItems player = playersItems.Get(index);
				
				if (topItems == null) {
					topItems = player;
					topItemsIndex = index;
				} else if (player.items > topItems.items) {
					topItems = player;
					topItemsIndex = index;
				}
			}
			
			playersItems.Remove(topItemsIndex);
			sortedPlayers.Insert(PlayerItems(topItems.name, topItems.items));
			topItems = null;
		}
		
		string positions = "";
     	string playernames = "";
		string stats = "";
		
		for (int i = 0; i < sortedPlayers.Count(); i++) {
            PlayerItems player = sortedPlayers.Get(i);
			
			positions += string.Format("```%1```", i + 1);
			
			if (DiscordPlayer.players[player.name] != string.Empty) {
           		string name = string.Format("@%1 | %2", DiscordPlayer.players[player.name], player.name);
				
				if (name.Length() > 28) {
					name = name.Substring(0, 28);
				}
				
				playernames += "```" + name + "```";;
			} else
				playernames += string.Format("```%1```", player.name);
			
			stats += string.Format("```%1```", player.items);
        }
		
		if (callback.messageId)
			restContext.DELETE_now(webhookToken + "/messages/" + callback.messageId, string.Empty);
	
		restContext.POST(callback, webhookToken + "?wait=true", GetEmbedPayload(positions, playernames, stats));
    }
	
	private string GetEmbedPayload(string positions, string names, string stats) {
		return string.Format("{\"embeds\": [{\"title\": \"─────────────────── :trophy: TOP %1 Players ───────────────────\"" + "," 
			+ "\"color\": 15258703" + ","
			+ "\"fields\": [{\"name\": \"Position\", \"value\": \"%2\", \"inline\": true},"
			+ "{\"name\": \"Player\", \"value\": \"%3\", \"inline\": true},"
			+ "{\"name\": \"%4\", \"value\": \"%5\", \"inline\": true}]"
			+ "}]}", Premium.playerRankItemNumberOfPlayers(), positions, names, Premium.playerRankItemDescription(), stats);
	}
	
	private void SaveToFile() {
		if (FileIO.FileExists(item_path)) {
			if (FileIO.DeleteFile(item_path)) {
				FileHandle textFileW = FileIO.OpenFile(item_path, FileMode.WRITE);
				
				if (textFileW) {
					foreach(string _, PlayerItems player: players) {
						textFileW.WriteLine(player.name + cryptograph + player.items);
					}
					
					textFileW.Close();
				}
			}
		} else {
			FileHandle textFileW = FileIO.OpenFile(item_path, FileMode.WRITE);
			
			if (textFileW) {
				foreach(string _, PlayerItems player: players) {
					textFileW.WriteLine(player.name + cryptograph + player.items);
				}
				
				textFileW.Close();
			}
		}
	}
}

class PlayerItems {
	string name;
	int items;
	
	void PlayerItems(string Name, int Items) {
		name = Name;
		items = Items;
	}
	
	void addItem() {
		items += 1;
	}
}