modded class SCR_BaseGameMode {
	private ref DatabaseConnection database;
	
	override void StartGameMode() {
		super.StartGameMode();
		
		if (!IsMaster())
			return;
		
		database = DatabaseConnection();
	}
	
	void SetServer(string id, string name, string gameMap, string _maxPlayers) {
		database.SetServer(id, name, gameMap, _maxPlayers);
	}
	
	void SetServer_Callback(string serverId) {
		database.SetServer_Callback(serverId);
	}
}

class DatabaseConnection {
	private RestContext restContext;
	private ref Database_Server_RestCallback callback = new Database_Server_RestCallback();
	
	private string server_id;
	private string server_name;
	private string server_gameMap;
	private string server_maxPlayers;
	
	void DatabaseConnection() {
		string header = string.Format("apikey,%1,Authorization, Bearer %2,Content-Type,application/json", Api.key, Api.key);
		restContext = GetGame().GetRestApi().GetContext(Api.url);
		restContext.SetHeaders(header);
	}
	
	void SetServer(string id, string name, string gameMap, string maxPlayers) {
		server_id = id;
		server_name = name;
		server_gameMap = gameMap;
		server_maxPlayers = maxPlayers;
		
		string server = string.Format("Server?id=eq.%1", id);
		restContext.GET(callback, server);
	}
	
	void SetServer_Callback(string serverId) {
		if (server_name == string.Empty)
			server_name = "NO NAME";
		
		if (server_gameMap == string.Empty)
			server_gameMap = "NO NAME";
		
		string content = string.Format("{"
			+ "\"id\": \"%1\","
			+ "\"name\": \"%2\","
			+ "\"gamemap\": \"%3\","
			+ "\"maxplayers\": \"%4\""
			+ "}", server_id, server_name, server_gameMap, server_maxPlayers
		);
		
		if (server_id == serverId) {
			string server = string.Format("Server?id=eq.%1", server_id);
			restContext.PUT_now(server, content);
		} else
			restContext.POST_now("Server", content);
	}
}

class Database_Server_RestCallback: RestCallback {
    override void OnSuccess(string data, int dataSize) {
		data.Replace("[", "");
		data.Replace("]", "");
		
		int serverId;
		bool isPremium;
		
		SCR_JsonLoadContext loadContext = new SCR_JsonLoadContext();
		loadContext.ImportFromString(data);
		loadContext.ReadValue("id", serverId);
		loadContext.ReadValue("isPremium", isPremium);

		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		gameMode.SetServer_Callback(serverId.ToString());
		
		Premium.setPremium(isPremium);
    }
}