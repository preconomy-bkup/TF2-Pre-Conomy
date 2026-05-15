//========= Copyright (c) 2026 TF2 Pre-Conomy Team, All rights reserved. ============//
//
// Purpose: Discord RPC implementation for Pre-Conomy.
//
//====================================================================================//

#include "cbase.h"
#include "tf_shareddefs.h"
#include "discord_rpc.h"
#include "discord_register.h"
#include "igamesystem.h"
#include "c_tf_playerresource.h"
#include "tf_gamerules.h"
#include <ctime>
#include "inetchannelinfo.h"

#define TF_DISCORD_APPID "1504455286377807992"

ConVar tf_discord_rpc( "tf_discord_rpc", "1", FCVAR_ARCHIVE, "Toggles Discord RPC." );

#define DISCORD_LOG(...) ConColorMsg(Color(114, 137, 218, 255), "[ Rich Presence ] " __VA_ARGS__);

// update once every 10 seconds. discord has an internal rate limiter of 15 seconds as well
#define DISCORD_UPDATE_RATE 10.0f

/**
 * @brief Discord RPC implementation
 */
class CTFDiscordRPC : public CAutoGameSystemPerFrame
{
public:
	CTFDiscordRPC();

	bool Init() OVERRIDE;
	void Shutdown() OVERRIDE;

	void Update( float framerate ) OVERRIDE;
	void LevelInitPreEntity() OVERRIDE;
	void LevelInitPostEntity() OVERRIDE;
	void LevelShutdownPreEntity() OVERRIDE;
	void LevelShutdownPostEntity() OVERRIDE;

	void Reset();
	void UpdateRPC();

private:
	// discord events
	static void OnReady( const DiscordUser *user );
	static void OnDiscordError( int errorCode, const char *szMessage );
	static void OnJoinGame( const char *joinSecret );
	static void OnSpectateGame( const char *spectateSecret );
	static void OnJoinRequest( const DiscordUser *joinRequest );

private:
	bool NeedToUpdate();

	void UpdateNetworkInfo();
	void UpdatePlayerInfo();

	DiscordRichPresence m_RPC;
	float m_flLastUpdatedTime;
};

// @PracticeMedicine:
// This does NOT need to be manually initialized as the base
// class' constructor will automatically register ourselves in
// the game systems and will initialize this as well in cdll_client_int.cpp
static CTFDiscordRPC s_DiscordRpc;
CTFDiscordRPC *g_pDiscordRpc = &s_DiscordRpc;

/**
 * @brief Discord RPC "ready" event
 */
void CTFDiscordRPC::OnReady( const DiscordUser *user )
{
	DISCORD_LOG( "Ready!\n" );
	DISCORD_LOG( "User %s#%s - %s\n", user->username, user->discriminator, user->userId );
	g_pDiscordRpc->Reset();
}

/**
 * @brief Discord RPC "errored" event
 */
void CTFDiscordRPC::OnDiscordError( int errorCode, const char *szMessage )
{
	char buff[ 1024 ];
	Q_snprintf( buff, 1024, "[Rich Presence] Init failed. code %d - error: %s\n", errorCode, szMessage );
	Warning( buff );
}

/**
 * @brief Discord RPC "joinGame" event
 */
void CTFDiscordRPC::OnJoinGame( const char *joinSecret )
{
	ConColorMsg( Color( 114, 137, 218, 255 ), "[Rich Presence] Join Game: %s\n", joinSecret );

	char szCommand[ 128 ];
	Q_snprintf( szCommand, sizeof( szCommand ), "connect %s\n", joinSecret );
	engine->ExecuteClientCmd( szCommand );
}

/**
 * @brief Discord RPC "spectateGame" event
 */
void CTFDiscordRPC::OnSpectateGame( const char *spectateSecret )
{
	ConColorMsg( Color( 114, 137, 218, 255 ), "[Rich Presence] Spectate Game: %s\n", spectateSecret );
}

/**
 * @brief Discord RPC "joinRequest" event
 */
void CTFDiscordRPC::OnJoinRequest( const DiscordUser *joinRequest )
{
	ConColorMsg( Color( 114, 137, 218, 255 ), "[Rich Presence] Join Request: %s#%s\n", joinRequest->username, joinRequest->discriminator );
	ConColorMsg( Color( 114, 137, 218, 255 ), "[Rich Presence] Join Request Accepted\n" );
	Discord_Respond( joinRequest->userId, DISCORD_REPLY_YES );
}

/**
 * @brief Constructor
 */
CTFDiscordRPC::CTFDiscordRPC()
	: CAutoGameSystemPerFrame( "tf_discord_rpc" )
	, m_flLastUpdatedTime( 0.0f )
	, m_RPC()
{
}

/**
 * @brief Determines whether the implementation is ready to update.
 * @returns true if the current real time has passed the update rate.
 */
bool CTFDiscordRPC::NeedToUpdate()
{
	return gpGlobals->realtime >= m_flLastUpdatedTime + DISCORD_UPDATE_RATE;
}

/**
 * @brief Initialization.
 * @returns Always true.
 */
bool CTFDiscordRPC::Init()
{
	DiscordEventHandlers events;
	Q_memset( &events, 0, sizeof( events ) );
	events.ready = &CTFDiscordRPC::OnReady;
	events.errored = &CTFDiscordRPC::OnDiscordError;
	events.joinGame = &CTFDiscordRPC::OnJoinGame;
	events.spectateGame = &CTFDiscordRPC::OnSpectateGame;
	events.joinRequest = &CTFDiscordRPC::OnJoinRequest;

	char command[ 512 ];
	Q_snprintf( command, sizeof( command ), "%s -game \"%s\" -novid -steam\n", CommandLine()->GetParm( 0 ), CommandLine()->ParmValue( "-game" ) );
	Discord_Register( TF_DISCORD_APPID, command );

	Discord_Initialize( TF_DISCORD_APPID, &events, 0, "" );
	Reset();
	return true;
}

/**
 * @brief Update function
 */
void CTFDiscordRPC::Update( float framerate )
{
	if ( !NeedToUpdate() )
		return;

	UpdateRPC();
	Discord_RunCallbacks();
}

/**
 * @brief
 */
void CTFDiscordRPC::LevelInitPreEntity()
{
	Update( gpGlobals->frametime );
}

/**
 * @brief
 */
void CTFDiscordRPC::LevelInitPostEntity()
{
	Update( gpGlobals->frametime );
}

/**
 * @brief
 */
void CTFDiscordRPC::LevelShutdownPreEntity()
{
}

/**
 * @brief
 */
void CTFDiscordRPC::LevelShutdownPostEntity()
{
	Reset();
}

/**
 * @brief Shut down function.
 */
void CTFDiscordRPC::Shutdown()
{
	Discord_Shutdown();
}

/**
 * @brief Update function (Discord Rich Precense configs)
 */
void CTFDiscordRPC::UpdateRPC()
{
	//The elapsed timer function using <ctime>
	//this is for setting up the time when the player joins a server
	//-Nbc66
	time_t iSysTime;
	time( &iSysTime );
	struct tm *tStartTime = NULL;
	tStartTime = localtime( &iSysTime );
	tStartTime->tm_sec += 0 - gpGlobals->curtime;

	m_flLastUpdatedTime = gpGlobals->realtime;

	if ( engine->IsConnected() )
	{
		UpdatePlayerInfo();
		UpdateNetworkInfo();
		//starts the elapsed timer for discord rpc
		//-Nbc66
		m_RPC.startTimestamp = mktime( tStartTime );
		//sets the map name
		m_RPC.details = "Currently in-game";
	}

	//checks if the loading bar is being drawn
	//and sets the discord status to "Currently is loading..."
	//-Nbc66
	if ( engine->IsDrawingLoadingImage() == true )
	{
		m_RPC.state = "";
		m_RPC.details = "Currently loading...";
	}

	//SetLogo();

	Discord_UpdatePresence( &m_RPC );
}

/**
 * @brief Resets the current rich precense to it's defaults
 */
void CTFDiscordRPC::Reset()
{
	Q_memset( &m_RPC, 0, sizeof( m_RPC ) );
	m_RPC.details = "Main Menu";
	m_RPC.state = "";
	m_RPC.endTimestamp;

	Discord_UpdatePresence( &m_RPC );
}

/**
 * @brief Update the current server's party info.
 */
void CTFDiscordRPC::UpdatePlayerInfo()
{
	C_TF_PlayerResource *pResource = g_TF_PR;
	if ( !pResource )
		return;

	int maxPlayers = gpGlobals->maxClients;
	int curPlayers = 0;

	const char *pzePlayerName = NULL;

	for ( int i = 1; i < maxPlayers; i++ )
	{
		if ( pResource->IsConnected( i ) )
		{
			curPlayers++;
			if ( pResource->IsLocalPlayer( i ) )
			{
				pzePlayerName = pResource->GetPlayerName( i );
			}
		}
	}

	m_RPC.partySize = curPlayers;
	m_RPC.partyMax = maxPlayers;
}

/**
 * @brief Update the current server's party network info.
 */
void CTFDiscordRPC::UpdateNetworkInfo()
{
	char partyId[ 128 ];
	INetChannelInfo *ni = engine->GetNetChannelInfo();
	Q_snprintf( partyId, sizeof( partyId ), "%s-party", ni->GetAddress() ); // adding -party here because secrets cannot match the party id

	m_RPC.partyId = partyId;

	m_RPC.joinSecret = ni->GetAddress();
	//dosent work untill i can figgure out how to get the source tv ip
	//m_RPC.spectateSecret = "Spectate";
}