"GameMenu" [$WIN32]
{
	"1"
	{
		"label" "#GameUI_GameMenu_ResumeGame"
		"command" "ResumeGame"
		"OnlyInGame" "1"
	}

	"2"
	{
		"label" "#GameUI_GameMenu_Disconnect"
		"command" "Disconnect"
		"OnlyInGame" "1"
	}

	"3"
	{
		"label" "#GameUI_GameMenu_PlayerList"
		"command" "OpenPlayerListDialog"
		"OnlyInGame" "1"
	} 

	"4"
	{
		"label" "Call A Vote"
		"command" "engine callvote; gameui_hide"
		"OnlyInGame" "1"
	}

	"5"
	{
		"label" ""
		"OnlyInGame" "1"
	}

	"4"
	{
		"label" "#GameUI_GameMenu_FindServers" 
		"command" "OpenServerBrowser"
	} 

	"5"
	{
		"label" "#GameUI_GameMenu_CreateServer"
		"command" "OpenCreateMultiplayerGameDialog"
	}
	
	"4"
	{
		"label" ""
	}

	"7"
	{
		"label" "#GameUI_GameMenu_CharacterSetup"
		"command" "engine open_charinfo"
	}

	"8"
	{
		"label" "#GameUI_GameMenu_Achievements"
		"command" "OpenAchievementsDialog"
	}

	"9"
	{
		"label"	"#GameUI_LoadCommentary"
		"command" "OpenLoadSingleplayerCommentaryDialog"
		"NotInGame" "1"
	}

	"4"
	{
		"label" ""
	}

	"10"
	{
		"label" "#GameUI_GameMenu_Options"
		"command" "OpenOptionsDialog"
	}

	"11"
	{
		"label" "Advanced Options"
		"command" "engine opentf2options"
	}

	"12"
	{
		"label" "#GameUI_GameMenu_Quit"
		"command" "Quit"
	}
}
