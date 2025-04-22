//Server Side
modded class SCR_BaseGameMode {
	static bool seedingRules = false;
}

//Player Side
modded class SCR_PlayerController {
	static bool didSetupDisplay = false;
	
	DisplaySeedingText displaySeedingText;
	
	override void SetPossessedEntity(IEntity entity) {
		super.SetPossessedEntity(entity);
		
		if(!SCR_PlayerController.didSetupDisplay) {
			SCR_PlayerController.didSetupDisplay = true;
			
			displaySeedingText = DisplaySeedingText();
			
			PlayerController playerController = GetGame().GetPlayerController();
			SCR_NotificationsComponent.SendCommandMessage(playerController.GetPlayerId(), "!seedingrules");
		}
	}
	
	static void SetSeedingRules(bool isSeeing) {
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
		
		playerController.displaySeedingText.SetSeeding(isSeeing);
	}
}

class DisplaySeedingText : ScriptedWidgetComponent {
	private RichTextWidget m_textWidget;
	
	void DisplaySeedingText() {
		Widget layout = GetGame().GetWorkspace().CreateWidgets("{64C4C79C66472DE8}layout/DisplayText.layout");
		m_textWidget = RichTextWidget.Cast(layout.FindWidget("RichText0"));
	}
	
	void SetSeeding(bool isSeeding) {
		if (isSeeding)
			m_textWidget.SetText("Seeding Rules: ON");
		else
			m_textWidget.SetText("Seeding Rules: OFF");
	}
}