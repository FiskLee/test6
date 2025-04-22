class WDN_RestCallback: RestCallback {
	string messageId = string.Empty;
	string filePath;
	
	void WDN_RestCallback(string path) {
		filePath = path;
		
		FileHandle textFileR = FileIO.OpenFile(filePath, FileMode.READ);
		
		if (textFileR) {
			textFileR.ReadLine(messageId);
			textFileR.Close();
		}
	}

    override void OnSuccess(string data, int dataSize) {
		SCR_JsonLoadContext loadContext = new SCR_JsonLoadContext();
		loadContext.ImportFromString(data);
		loadContext.ReadValue("id", messageId);
		
		if (messageId == string.Empty)
			return;
		
		if (FileIO.FileExists(filePath)) {
			if (FileIO.DeleteFile(filePath)) {
				FileHandle textFileW = FileIO.OpenFile(filePath, FileMode.WRITE);
				
				if (textFileW) {
					textFileW.WriteLine(messageId);
					textFileW.Close();
				}
			}
		} else {
			FileHandle textFileW = FileIO.OpenFile(filePath, FileMode.WRITE);
			
			if (textFileW) {
				textFileW.WriteLine(messageId);
				textFileW.Close();
			}
		}
    }
	
	override void OnError(int errorCode) {
		Print(errorCode, LogLevel.WARNING);
    }
}