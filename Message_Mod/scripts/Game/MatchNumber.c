class MatchNumber {
	private static string match_path = "$profile:match_number.txt";
	
	static string GetMatchNumber() {
		FileHandle textFileR = FileIO.OpenFile(match_path, FileMode.READ);
		
		if (textFileR) {
			string match = string.Empty;
			
			textFileR.ReadLine(match);
			textFileR.Close();
			
			return match;
		} else {
			return "1";
		}
	}
	
	static void SetNextMatchNumber(string number) {
		if (FileIO.FileExists(match_path)) {
			if (FileIO.DeleteFile(match_path)) {
				FileHandle textFileW = FileIO.OpenFile(match_path, FileMode.WRITE);
				
				if (textFileW) {
					textFileW.WriteLine(number);
					textFileW.Close();
				}
			}
		} else {
			FileHandle textFileW = FileIO.OpenFile(match_path, FileMode.WRITE);
			
			if (textFileW) {
				textFileW.WriteLine(number);
				textFileW.Close();
			}
		}
	}
}