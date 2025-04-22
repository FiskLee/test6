modded class SCR_PlayerController {
	static bool didSetupIventory = false;
	
	override void SetPossessedEntity(IEntity entity) {
		super.SetPossessedEntity(entity);
		
		if(!SCR_PlayerController.didSetupIventory) {
			SCR_PlayerController.didSetupIventory = true;
			
			PlayerController playerController = GetGame().GetPlayerController();
			SCR_NotificationsComponent.SendCommandMessage(playerController.GetPlayerId(), "!armband");
			SCR_NotificationsComponent.SendCommandMessage(playerController.GetPlayerId(), "!clearkills");
			SCR_NotificationsComponent.SendCommandMessage(playerController.GetPlayerId(), "!itemnames");
		}
	}
}

modded class SCR_InventorySlotUI {
	static bool isArmbandLocked = false;
	
    override void SetSlotVisible( bool bVisible ) {
        super.SetSlotVisible(bVisible);
        if (m_sItemName.Contains("Armband") && SCR_InventorySlotUI.isArmbandLocked) {	
            m_bVisible = false;
            m_widget.SetEnabled(false);
            m_widget.SetVisible(true);
        }
    }
}

modded class SCR_ArsenalInventoryStorageManagerComponent {
	static bool canDeposit = false;
	
	override bool IsPrefabInArsenalStorage(ResourceName item) {
		SCR_ArsenalInventoryStorageManagerComponent.canDeposit = true;
		return super.IsPrefabInArsenalStorage(item);
	}
}

modded class SCR_InventoryStorageManagerComponent {
	static ref array<string> itemNames = {};
	static ref array<string> playerKills = {};

	private bool didSetNotification = false;
	private ref array<string> stolenEnemyItem = {};
	
	static void SendMessage(string message, int playerId) {
		SCR_ChatPanelManager chat = SCR_ChatPanelManager.GetInstance();
		chat.OnNewMessageGeneral(message, 0, playerId);
	}
	
	static void SetPlayerKills(int playerId, array<string> kills) {
		if(kills != null)
			SCR_InventoryStorageManagerComponent.playerKills = kills;
	}
		
	override void OpenInventory() {
		super.OpenInventory();
		
		if (!didSetNotification) {
			didSetNotification = true;
			m_OnItemRemovedInvoker.Insert(OnItemRemovedCheck);
		}
		
		SCR_NotificationsComponent.SendCommandMessage(GetGame().GetPlayerController().GetPlayerId(), "!playerkills");
	}
	
	override void OnInventoryMenuClosed() {
		super.OnInventoryMenuClosed();
		
		SCR_ArsenalInventoryStorageManagerComponent.canDeposit = false;
	}
	
	override void InsertItem(IEntity pItem, BaseInventoryStorageComponent pStorageTo = null, BaseInventoryStorageComponent pStorageFrom = null, SCR_InvCallBack cb = null) {
		super.InsertItem(pItem, pStorageTo, pStorageFrom, cb);

		InventoryItemComponent pComponent = GetItemComponentFromEntity(pItem);
			
		if (!pComponent)
			return;
		
		UIInfo info = pComponent.GetUIInfo();
		
		if (!info)
			return;
		
		string itemName = info.GetName();
		itemName.ToLower();
		
		bool isItem = false;
		
		foreach(string item: SCR_InventoryStorageManagerComponent.itemNames) {
			if (itemName.Contains(item)) {
				isItem = true;
				break;
			}
		}
		
		if (!isItem)
			return;
		
		if (pStorageFrom) {
			if (!IsFactionEnemy(SCR_ChimeraCharacter.Cast(pStorageFrom.GetOwner())))
				return;
		} else
			return;
		
		#ifdef WORKBENCH
			SCR_InventoryStorageManagerComponent.playerKills = SCR_BaseGameMode.playersKills[GetGame().GetPlayerManager().GetPlayerName(GetGame().GetPlayerController().GetPlayerId())];
		#endif
		
		if (SCR_InventoryStorageManagerComponent.playerKills) {	
			if (SCR_InventoryStorageManagerComponent.playerKills.Count() == 0)
				return;
			
			string stolenPlayer = SCR_InventoryStorageManagerComponent.playerKills[SCR_InventoryStorageManagerComponent.playerKills.Count() - 1];
			
			if (!stolenEnemyItem.Contains(stolenPlayer)) {
				stolenEnemyItem.Insert(stolenPlayer);
				SCR_NotificationsComponent.SendCommandMessage(GetGame().GetPlayerController().GetPlayerId(), "!itemstolen" + stolenPlayer);
				return;
			}
		}
	}

	protected void OnItemRemovedCheck(IEntity item, BaseInventoryStorageComponent storageOwner) {
		if (!SCR_ArsenalInventoryStorageManagerComponent.canDeposit)
			return;
		
		InventoryItemComponent pComponent = GetItemComponentFromEntity(item);
		
		if (!pComponent)
			return;
		
		UIInfo info = pComponent.GetUIInfo();
		
		if (!info)
			return;
		
		string itemName = info.GetName();
		itemName.ToLower();
		
		bool isItem = false;
		
		foreach(string itn: SCR_InventoryStorageManagerComponent.itemNames) {
			if (itemName.Contains(itn)) {
				isItem = true;
				break;
			}
		}
		
		if (!isItem)
			return;
		
		if (stolenEnemyItem.Count() == 0)
			return;
		
		int index = stolenEnemyItem.Count() - 1;
		
		string stolenPlayer = stolenEnemyItem[index];
		stolenEnemyItem.Remove(index);
		
		if(!SCR_InventoryStorageManagerComponent.playerKills.Contains(stolenPlayer))
			return;
		
		SCR_NotificationsComponent.SendCommandMessage(GetGame().GetPlayerController().GetPlayerId(), "!itemdelivered" + stolenPlayer);
	}
	
	private InventoryItemComponent GetItemComponentFromEntity(IEntity pEntity) {
		if (pEntity == null)
			return null;
		
		return InventoryItemComponent.Cast(pEntity.FindComponent(InventoryItemComponent));
	}
	
	private bool IsFactionEnemy(SCR_ChimeraCharacter character) {	
		if (character == null)
			return false;
		
		Faction deadPlayerFaction = character.GetFaction();
		
		if (deadPlayerFaction == null)
			return false;
		
		SCR_ChimeraCharacter player = SCR_ChimeraCharacter.Cast(GetGame().GetPlayerController().GetControlledEntity());
		Faction playerFaction = player.GetFaction();

		return deadPlayerFaction.IsFactionEnemy(playerFaction);
	}
}