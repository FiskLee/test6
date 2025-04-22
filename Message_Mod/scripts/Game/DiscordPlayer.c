modded class SCR_BaseGameMode {
	private ref DiscordPlayer discord;
	
	override void StartGameMode() {
		super.StartGameMode();
		
		if (!IsMaster())
			return;
		
		discord = new DiscordPlayer();
	}
	
	void AddPlayer(string playername, string discordId) {
		discord.AddPlayer(playername, discordId);
	}
}

class DiscordPlayer {
	static ref map<string, string> players = new map<string, string>();
	
	private const string cryptograph = "::/::";
	private const string discord_path = "$profile:player_discord.txt";
	
	void DiscordPlayer() {
		DiscordPlayer.players.Clear();
		
		FileHandle textFileR = FileIO.OpenFile(discord_path, FileMode.READ);
		
		if (textFileR) {
			string line = string.Empty;
			
			while(textFileR.ReadLine(line) >= 0) {
				array<string> player = new array<string>();
				line.Split(cryptograph, player, true);
				
				if (player.Count() <= 1)
					continue;
				
				string playerName = player[0];
				string discordId = player[1];
								
				DiscordPlayer.players[playerName] = discordId;
			}
			
			textFileR.Close();
		}
	}
	
	void AddPlayer(string playername, string discordId) {
		DiscordPlayer.players[playername] = discordId;
		SaveToFile();
	}
	
	private void SaveToFile() {
		FileHandle textFileW = FileIO.OpenFile(discord_path, FileMode.WRITE);
			
		if (textFileW) {
			foreach(string name, string discordId: DiscordPlayer.players) {
				textFileW.WriteLine(name + cryptograph + discordId);
			}
			
			textFileW.Close();
		}
	}
}