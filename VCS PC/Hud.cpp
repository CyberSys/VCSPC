#include "StdAfx.h"

/*#pragma message("Using temp global vars for CHud...")
const float fMyClockWidth = 0.43;
const float fMy12HClockWidth = 0.42;
const float fMyClockHeight = 1.0;
const float fMyClockPosX = 87.0;
const float fMyClockPosY = 25.0;
const float fHealthbarPosX = 138.0;
const float fHealthbarPosY = 67.0;
const float fHealthbarWidth = 48.0;
const float fHealthbarHeight = 15.0;
const float fHealthbar2PPosY = 194.0;
const float fArmourbarPosY = 47.0;
const float fArmourbar2PPosY = 176.0;

const float fMoneyPosX = 37.0;
const float fMoneyPosY = 85.0;
const float fMoneyWidth = 0.46;

const float fWeaponIconPosX = 85.0;
const float fWeaponIconPosY = 26.0;
const float fWeaponIcon2PPosY = 138.0;

const float	fWeaponAmmoWidth = 0.24;
const float	fWeaponAmmoHeight = 0.58;
const float	fWeaponAmmoPosX = 54.0;
const float	fWeaponAmmoPosY = 71.0;
const float	fWeaponAmmo2PPosY = 26.0;*/

// NOT TEMP
const float fWeaponIconWidth = 66.0f;
const float fWeaponIconHeight = 72.0f;
const float	fWLStarPosX	= 34.0f;
const float fWLStarPosY = 100.5f;
const float fWLStarHeight = 1.4f;
const float fWLStarWidth = 0.75f;
const float fWLStarDistance = 18.0f;

int&		CHud::m_HelpMessageState = *(int*)0xBAA474;
char		CHud::m_PagerMessage[16];

static unsigned char	PagerOn;
static bool				PagerSoundPlayed;
static float			PagerXOffset;

static bool				CounterOnLastFrame[NUM_ONSCREEN_COUNTERS], PlaceOnLastFrame;
static unsigned short	CounterFlashTimer[NUM_ONSCREEN_COUNTERS], PlaceFlashTimer;

//#define COMPILE_SLANTED_TEXT

// Wrappers
WRAPPER void CHud::DrawBarChart(float fX, float fY, WORD wWidth, WORD wHeight, float fPercentage, BYTE drawBlueLine, BYTE drawPercentage, BYTE drawBorder, CRGBA dwColor, CRGBA dwForeColor)
{	WRAPARG(fX); WRAPARG(fY); WRAPARG(wWidth); WRAPARG(wHeight); WRAPARG(fPercentage); WRAPARG(drawBlueLine); WRAPARG(drawPercentage); WRAPARG(drawBorder); WRAPARG(dwColor); WRAPARG(dwForeColor);
	EAXJMP(0x728640); }
WRAPPER void CHud::DrawWeaponIcon(CPed* pPed, int iX, int iY, float fAlpha = 255.0f) { WRAPARG(pPed); WRAPARG(iX); WRAPARG(iY); WRAPARG(fAlpha); EAXJMP(0x58D7D0); }


void CHud::GetRidOfAllCustomHUDMessages()
{
	hud.pCarName = NULL;
	hud.pCarNameToDisplay = NULL;
	hud.CarNameShowingTime = 0;
	hud.CarNameFadingTime = 0;
	hud.CarNameStage = 0;

	hud.m_ZoneNameTimer = 0;
	hud.m_pZoneName = NULL;
	hud.m_ZoneState = 0;

	CUserDisplay::Pager.ClearMessages();
}

void CHud::Initialise()
{
#if !DEBUG
	hud.bShouldFPSBeDisplayed = false;
#else
	hud.bShouldFPSBeDisplayed = true;
#endif
	hud.bLCSPS2Style = false;
	hud.bShouldGridrefBeDisplayed = false;
//	hud.bDrawDevLogos = false;

	hud.CarNameStage = 0;
	hud.pLastCarName = NULL;
	hud.pCarName = NULL;
	hud.CarNameShowingTime = 0;
	hud.CarNameFadingTime = 0;
	
	hud.m_ZoneState = 0;
	hud.m_ZoneNameTimer = 0;
	hud.m_pZoneName = NULL;
	hud.m_pLastZoneName = NULL;

	PagerOn = 0;
	PagerSoundPlayed = false;
	PagerXOffset = 200.0;

	for ( int i = 0; i < NUM_ONSCREEN_COUNTERS; ++i )
	{
		CounterOnLastFrame[i] = false;
		CounterFlashTimer[i] = 0;
	}

	PlaceOnLastFrame = false;
	PlaceFlashTimer = 0;
}

void CHud::SetVehicleName(char* carName)
{
	hud.pCarName = carName;
}

void CHud::SetZoneName(char* zoneName, bool bIgnoreIntID)
{
	if ( (bIgnoreIntID || !*activeInterior && !hud.m_ZoneState) && (!zoneName || strncmp(zoneName, "DUMMY", 5)) )
		hud.m_pZoneName = zoneName;
}

void CHud::SetPagerMessage(char* pMsg)
{
	unsigned short	wLoopCounter = 0;
	while ( wLoopCounter < sizeof(m_PagerMessage) )
	{
		if ( !pMsg[wLoopCounter] )
			break;
		m_PagerMessage[wLoopCounter] = pMsg[wLoopCounter];
		++wLoopCounter;
	}

	m_PagerMessage[wLoopCounter] = '\0';
}

void CHud::DrawHUD()
{	
	CRGBA	BorderColour(0, 0, 0, 255);
	char	string[50];
	CPed*	playerPed = players[*bCurrentPlayer].GetPed();
	CPed*	secondPlayerPed = players[1].GetPed();

	if ( !CDLCManager::GetDLC(DLC_HALLOWEEN)->IsEnabled() )
	{
		// Clock
		BYTE	bHour = clock_struct->GetHour();
		float	fClockWidth;

		CFont::SetTextBackground(0, 0);
		CFont::SetTextUseProportionalValues(false);
		CFont::SetFontStyle(FONT_Hud);
		CFont::SetTextAlignment(ALIGN_Right);
		CFont::SetTextWrapX(0.0);
		CFont::SetTextOutline(0);
		CFont::SetTextBorderRGBA(CRGBA(0, 0, 0, 255));
		if ( menu->GetHudMode() == 2 )
		{
			fClockWidth = 0.42f;
			if ( CClock::Convert24To12(bHour) )
				sprintf(string, "%d:%02d]", bHour, clock_struct->GetMinute());
			else
				sprintf(string, "%d:%02d[", bHour, clock_struct->GetMinute());
		}
		else
		{
			fClockWidth = 0.43f;
			sprintf(string, "%02d:%02d", bHour, clock_struct->GetMinute());
		}
		CFont::SetTextLetterSize(_width(fClockWidth), _height(1.0));
		CFont::SetTextColour(CRGBA(BaseColors[14]));
		CFont::PrintString(_x(87.0), _y(25.0), string);
		//CFont::SetTextOutline(0);
	}

	// 1st player healthbar
	PrintHealthForPlayer(*bCurrentPlayer, _x(138.0), _y(67.0));

	// 2nd player healthbar
	if ( secondPlayerPed )
		PrintHealthForPlayer(1, _x(138.0), _y(194.0));

	BYTE		bDraw1stPlayerOxygenBar = 0;
	BYTE		bDraw2ndPlayerOxygenBar = 0;
	CVehicle*	pVehicle = 0;
	CVehicle*	p2ndVehicle = 0;

	if( playerPed->GetPedFlags().bIsStanding && playerPed->GetPedFlags().bInVehicle )
		pVehicle = playerPed->GetVehiclePtr();

	if( secondPlayerPed && secondPlayerPed->GetPedFlags().bIsStanding && secondPlayerPed->GetPedFlags().bInVehicle )
		p2ndVehicle = secondPlayerPed->GetVehiclePtr();

	if (	playerPed->GetPedIntelligencePtr()->GetSwimTask() ||
			pVehicle &&
			pVehicle->GetVehicleFlags().bIsDrowning ||
			CStats::CalcPlayerStat(8) > players[*bCurrentPlayer].GetPlayerData().m_fBreath && *unkLastBreathCheck + 500 > CTimer::m_snTimeInMilliseconds )
	{
		bDraw1stPlayerOxygenBar = 1;
		*unkLastBreathCheck = CTimer::m_snTimeInMilliseconds;
	}

	if ( secondPlayerPed )
	{
		if (	secondPlayerPed->GetPedIntelligencePtr()->GetSwimTask() ||
				p2ndVehicle &&
				p2ndVehicle->GetVehicleFlags().bIsDrowning ||
				CStats::CalcPlayerStat(8) > players[1].GetPlayerData().m_fBreath && *unkLastBreathCheck + 500 > CTimer::m_snTimeInMilliseconds )
		{
			bDraw2ndPlayerOxygenBar = 1;
			*unkLastBreathCheck = CTimer::m_snTimeInMilliseconds;
		}
	}
	
	// 1st player armourbar
	if ( !bDraw1stPlayerOxygenBar )
		PrintArmourForPlayer(*bCurrentPlayer, _x(138.0), _y(47.0));
	else
		PrintBreathForPlayer(*bCurrentPlayer, _x(138.0), _y(47.0));

	// 2nd player armourbar
	if ( secondPlayerPed )
	{
		if ( !bDraw2ndPlayerOxygenBar )
			PrintArmourForPlayer(1, _x(138.0), _y(176.0));
		else
			PrintBreathForPlayer(1, _x(138.0), _y(176.0));
	}

	// Money
	int		displayedScore = players[*bCurrentPlayer].GetDisplayedScore();
	char*	moneyText;

	if ( displayedScore < 0 )
	{
		CFont::SetTextColour(BaseColors[0]);
		displayedScore = -displayedScore;
		moneyText = "-$%07d";
	}
	else
	{	
		CFont::SetTextColour(BaseColors[1]);
		moneyText = "$%08d";
	}
	sprintf(string, moneyText, displayedScore);
	CFont::SetTextUseProportionalValues(false);
	CFont::SetTextBackground(0, 0);
	CFont::SetTextLetterSize(_width(0.46), _height(1.0));
	CFont::SetTextAlignment(ALIGN_Right);
	CFont::SetTextWrapX(0.0);
	CFont::SetFontStyle(FONT_Hud);
	CFont::SetTextOutline(0);
	CFont::SetTextBorderRGBA(BorderColour);

	CFont::PrintString(_x(37.0), _y(85.0), /*ConvertHUDNumbers(string)*/string);
	//CFont::SetTextOutline(0);

	// 1st player weapon icon
	DrawWeaponIcon(playerPed, _x(85.0), _y(26.0));

	// 1st player weapon ammo
	DrawWeaponAmmo(playerPed, _x(54.0), _y(71.0));

	// 2nd player weapon icon
	if ( secondPlayerPed )
	{
		DrawWeaponIcon(secondPlayerPed, _x(85.0), _y(138.0));

		// 2nd player weapon ammo
		DrawWeaponAmmo(secondPlayerPed, _x(54.0), _y(26.0));
	}

	// Pager
	if ( !m_PagerMessage[0] )
	{
		if ( PagerOn == 1 )
		{
			PagerSoundPlayed = false;
			PagerOn = 2;
		}
	}
	if ( m_PagerMessage[0] || PagerOn == 2 )
	{
		if ( !PagerOn )
		{
			PagerOn = 1;
			PagerXOffset = 200.0;
		}
		if ( PagerOn == 1 )
		{
			if ( PagerXOffset > 0.0 )
			{
				float	fStep = PagerXOffset * 0.1;
				if ( fStep > 10.0 )
					fStep = 10.0;
				PagerXOffset -= fStep;
			}
			if ( !PagerSoundPlayed )
			{
				MusicManager->ReportFrontendAudioEvent(46, 0.0, 1.0);
				PagerSoundPlayed = true;
			}
		}
		else
		{
			if ( PagerOn == 2 )
			{
				float	fStep = PagerXOffset * 0.1;
				if ( fStep < 2.0 )
					fStep = 2.0;
				PagerXOffset += fStep;
				if ( PagerXOffset > 200.0 )
				{
					PagerOn = 0;
					PagerXOffset = 200.0;
				}
			}
		}

		hudTextures[HUD_Pager].DrawTexturedRect(&CRect(_xleft(32.5 - PagerXOffset), _y(117.5), _xleft(202.5 - PagerXOffset), _y(32.5)), &CRGBA(255, 255, 255, 255));

		CFont::SetTextUseProportionalValues(false);
		CFont::SetTextLetterSize(_width(0.5), _height(1.4));
		CFont::SetTextAlignment(ALIGN_Left);
		CFont::SetTextColour(CRGBA(0, 0, 0, 255));
		CFont::SetFontStyle(FONT_PagerFont);
		CFont::PrintString(_xleft(50.0 - PagerXOffset), _y(47.5), m_PagerMessage);
	}
}

void CHud::DrawOnscreenTimer()
{
	if ( CUserDisplay::OnscnTimer.IsEnabled() && !StyledText_5[0] && !Garage_MessageIDString[0] ) 
	{
		CFont::SetTextBackground(0, 0);
		CFont::SetTextLetterSize(_width(0.5f), _height(1.0f));
		CFont::SetTextAlignment(ALIGN_Right);
		CFont::SetFontStyle(FONT_Eurostile);
		CFont::SetTextShadow(1);

		static DWORD			dwTicksCounter = 0;
		static int				nCounterTimer = 0;
		bool					bRecalculateStuffThisTick;

		if ( CTimer::m_snTimeInMillisecondsPauseMode - nCounterTimer > 40 )	// Cap it at 25 FPS
		{
			bRecalculateStuffThisTick = true;
			++dwTicksCounter;
			nCounterTimer = CTimer::m_snTimeInMillisecondsPauseMode;
		}
		else
			bRecalculateStuffThisTick = false;

		COnscreenCounterEntry*	pCounters = CUserDisplay::OnscnTimer.GetCounter();
		COnscreenTimerEntry*	pTimer = CUserDisplay::OnscnTimer.GetTimer();
		unsigned char			bChildrenEntries = 0;

		for ( int i = 0; i < NUM_ONSCREEN_COUNTERS; ++i )
		{
			if ( !pCounters[i].m_bShown || pCounters[i].m_bForceRecheck )
			{
				pCounters[i].m_bForceRecheck = false;
				CounterOnLastFrame[i] = false;
			}

			if ( pCounters[i].m_bShown )
			{
				if ( pCounters[i].m_wOwnsWhat != 0xFFFF )
					bChildrenEntries += 3;
			}
		}

		if ( pTimer->m_bShown )
		{
			int		nRemainder = *pTimer->m_pVariable % 1000;
			if ( !pTimer->m_bFlashing || (*pTimer->m_pVariable < 750 || ( nRemainder < 750 && nRemainder > 250 )) )
			{
				CFont::SetTextUseProportionalValues(false);
				CFont::SetTextBorderRGBA(pTimer->m_shadowColour);
				CFont::SetTextColour(pTimer->m_colour);			

				CFont::PrintString(_x(32.0f), _y(bChildrenEntries * 20.0f + 148.0f), pTimer->m_cDisplayedText);
				if ( pTimer->m_cGXTentry[0] )
				{
					float		fPosXOffset;
					int			nTextLength = strlen(pTimer->m_cDisplayedText);
					if ( nTextLength > 4 )
						fPosXOffset = 12.5f * (nTextLength - 4);
					else
						fPosXOffset = 0.0f;

					CFont::SetTextUseProportionalValues(true);
					CFont::PrintString(_x(87.5f + fPosXOffset), _y(bChildrenEntries * 20.0f + 148.0f), gxt->GetText(pTimer->m_cGXTentry));
				}
			}
		}

		for ( int i = 0; i < NUM_ONSCREEN_COUNTERS; ++i )
		{
			if ( pCounters[i].m_bShown )
			{
				if ( !CounterOnLastFrame[i] && pCounters[i].m_bFlashing )
					CounterFlashTimer[i] = 1;

				CounterOnLastFrame[i] = true;

				/*if ( CounterFlashTimer[i] )
				{
					if ( ++CounterFlashTimer[i] > 50 )
						CounterFlashTimer[i] = 0;
				}*/

				if ( CounterFlashTimer[i] )
				{
					if ( bRecalculateStuffThisTick )
					{
						if ( ++CounterFlashTimer[i] > 50 )
							CounterFlashTimer[i] = 0;
					}
				}
				
				if ( dwTicksCounter & 4 || !CounterFlashTimer[i] )
				{
					CFont::SetTextBorderRGBA(pCounters[i].m_shadowColour);
					CFont::SetTextColour(pCounters[i].m_colour);

					if ( pCounters[i].m_wType != 3 )
					{
						if ( pCounters[i].m_wType == 1 )
							DrawBarChart(	_x(93.0f), _y(i * 20.0f + 173.0f), _width(61.0f),
											_height(18.0), pCounters[i].m_counterData.nBarFill,
											0, 0, 1, pCounters[i].m_colour, 
											CRGBA(0, 0, 0, 0));
						else
						{
							CFont::SetTextUseProportionalValues(false);
							CFont::PrintString(_x(32.0f), _y(i * 20.0f + 173.0f), pCounters[i].m_counterData.cDisplayedText);
						}
					}

					if ( pCounters[i].m_cGXTentry[0] )
					{
						float		fPosXOffset;
						int			nTextLength = pCounters[i].m_wType == 1 ? 5 : strlen(pCounters[i].m_counterData.cDisplayedText);
						if ( nTextLength > 4 )
							fPosXOffset = 12.5f * (nTextLength - 4);
						else
							fPosXOffset = 0.0f;

						CFont::SetTextUseProportionalValues(true);
						if ( pCounters[i].m_wType != 3 )
							CFont::PrintString(_x(87.5f + fPosXOffset), _y(i * 20.0f + 173.0f), gxt->GetText(pCounters[i].m_cGXTentry));
						else
							CFont::PrintString(_x(32.0f), _y(i * 20.0f + 160.0f), gxt->GetText(pCounters[i].m_cGXTentry));
					}
				}
			}
		}
	}
}

void CHud::DrawPermanentTexts()
{
#ifndef _NDEBUG
#if !defined COMPILE_RC && !defined MAKE_ZZCOOL_MOVIE_DEMO
	if ( hud.ShouldDisplayFPS() && !InAmazingScreenshotMode )
	{
		char		debugText[64];
		if ( !CUpdateManager::AnyTextDisplaysNow() )
		{	
			sprintf(debugText, "%d FPS", static_cast<int>(*currentFPS));
			CFont::SetTextBackground(0, 0);
			CFont::SetTextUseProportionalValues(false);
			CFont::SetFontStyle(FONT_Eurostile);
			CFont::SetTextAlignment(ALIGN_Right);
			CFont::SetTextOutline(1);
			CFont::SetTextBorderRGBA(CRGBA(0, 0, 0, 255));
			CFont::SetTextLetterSize(_width(0.35), _height(0.65));
			CFont::SetTextColour(CRGBA(BaseColors[*currentFPS > 10.0]));
			CFont::PrintString(_x(15.0), _y(7.5), debugText);
		}

#ifdef INCLUDE_STREAMING_TEXT
		long double	percentUsage = ((long double)(*memoryUsed) / * memoryAvailable) * 100.0;

		sprintf(debugText, "STREAMING %dKB/%dKB (%d%% USED)", *memoryUsed / 1024, *memoryAvailable / 1024, (int)percentUsage);
		CFont::SetTextUseProportionalValues(true);
		CFont::SetTextAlignment(ALIGN_Left);
		CFont::SetTextColour(CRGBA(DEBUG_ORANGE_R, DEBUG_ORANGE_G, DEBUG_ORANGE_B));
		CFont::PrintString(_width(5.0), _y(7.5), debugText);
#endif

		if ( CPed* pPlayerPed = players[0].GetPed() )
		{
			CVector*	coords = pPlayerPed->GetCoords();

			sprintf(debugText, "%.3f %.3f %.3f", coords->x, coords->y, coords->z);
			CFont::SetTextUseProportionalValues(true);
			CFont::SetTextAlignment(ALIGN_Right);
			CFont::SetTextColour(CRGBA(0x0A, 0x57, 0x82));
			CFont::PrintString(_x(5.0), _ydown(11.0), debugText);
		}
		CFont::SetTextOutline(0);
	}
#else
	if ( !CUpdateManager::AnyTextDisplaysNow() )
	{
		CFont::SetTextBackground(0, 0);
		CFont::SetTextUseProportionalValues(true);
		CFont::SetFontStyle(FONT_Eurostile);
		CFont::SetTextAlignment(ALIGN_Right);
		CFont::SetTextOutline(1);
		CFont::SetTextBorderRGBA(CRGBA(0, 0, 0, 255));
		CFont::SetTextLetterSize(_width(0.75), _height(1.15));
		CFont::SetTextColour(CRGBA(255, 255, 255));
		CFont::PrintString(_x(25.0), _y(3.0), "PRE-BETA3 BUILD");
	}
#endif
#endif
	CUpdateManager::Display();
}

void CHud::PrintHealthForPlayer(int playerID, float posX, float posY)
{
	if ( *wFlashingComponentID != FLASH_Healthbar || CTimer::m_FrameCounter & 8 )
	{
		if ( players[playerID].GetLastTimeArmourLost() == players[playerID].GetLastTimeEnergyLost() || players[playerID].GetLastTimeEnergyLost() + BAR_ENERGY_LOSS_FLASH_DURATION < CTimer::m_snTimeInMilliseconds || CTimer::m_FrameCounter & 4 )
		{
			if ( players[playerID].GetPed()->GetHealth() >= 10 || CTimer::m_FrameCounter & 8 )
			{
				if ( hud.IsStyleLCS() )
					DrawBarChart(	posX, posY, _width(48.0), _height(15.0), 
									players[playerID].GetPed()->GetHealth() / players[playerID].GetMaxHealth() * 100.0,
									0, 0, 1, CRGBA(BaseColors[9]), CRGBA(0, 0, 0, 0) );
				else
					DrawBarChartWithRoundBorder(	posX, posY, _width(48.0), _height(15.0), 
									players[playerID].GetPed()->GetHealth() / players[playerID].GetMaxHealth() * 100.0,
									0, 0, 1, CRGBA(BaseColors[9]), CRGBA(0, 0, 0, 0) );
			}
		}
	}
}

void CHud::PrintArmourForPlayer(int playerID, float posX, float posY)
{
	if ( *wFlashingComponentID != FLASH_Armourbar || CTimer::m_FrameCounter & 8 )
	{
		if ( players[playerID].GetLastTimeArmourLost() == 0 || players[playerID].GetLastTimeArmourLost() + BAR_ENERGY_LOSS_FLASH_DURATION < CTimer::m_snTimeInMilliseconds || CTimer::m_FrameCounter & 4 )
		{
			if ( players[playerID].GetPed()->GetArmour() >= 1.0 )
			{
				if ( hud.IsStyleLCS() )
					DrawBarChart(	posX, posY, _width(48.0), _height(15.0), 
									players[playerID].GetPed()->GetArmour() / players[playerID].GetMaxArmour() * 100.0,
									0, 0, 1, CRGBA(0x09, 0xFF, 0xFF, 0xFF), CRGBA(0, 0, 0, 0) );
				else
					DrawBarChartWithRoundBorder(	posX, posY, _width(48.0), _height(15.0), 
									players[playerID].GetPed()->GetArmour() / players[playerID].GetMaxArmour() * 100.0,
									0, 0, 1, CRGBA(0x09, 0xFF, 0xFF, 0xFF), CRGBA(0, 0, 0, 0) );
			}
		}
	}
}

void CHud::PrintBreathForPlayer(int playerID, float posX, float posY)
{
	if ( *wFlashingComponentID != FLASH_Breathbar || CTimer::m_FrameCounter & 8 )
	{
		if ( hud.IsStyleLCS() )
			DrawBarChart(	posX, posY, _width(48.0), _height(15.0),
								players[playerID].GetPed()->GetPlayerData()->m_fBreath / CStats::CalcPlayerStat(8) * 100.0,
								0, 0, 1, CRGBA(BaseColors[13]), CRGBA(0, 0, 0, 0) );
		else
			DrawBarChartWithRoundBorder(	posX, posY, _width(48.0), _height(15.0),
								players[playerID].GetPed()->GetPlayerData()->m_fBreath / CStats::CalcPlayerStat(8) * 100.0,
								0, 0, 1, CRGBA(BaseColors[13]), CRGBA(0, 0, 0, 0) );
	}
}

void CHud::DrawWeaponAmmo(CPed* ped, float fX, float fY)
{
	char	AmmoText[16];
	
	int		weapAmmo = ped->GetWeaponSlots()[ped->GetActiveWeapon()].dwAmmoRemaining;
	eWeaponType		weapType = ped->GetWeaponSlots()[ped->GetActiveWeapon()].dwType;
	int		weapAmmoInClip = ped->GetWeaponSlots()[ped->GetActiveWeapon()].dwAmmoInClip;

	BYTE	somethingUnknown = ped->func_5E6580();

	short	clipSize = CWeaponInfo::GetWeaponInfo(weapType, somethingUnknown)->GetClipSize();
	
	if ( clipSize <= 1 || clipSize >= 1000 )
		sprintf(AmmoText, "%d", weapAmmo);
	else
	{
		int		ammoClipToShow;
		int		ammoRestToShow;

		if ( weapType == 37 )
		{
			int tempAmmoValue = 9999;
			if ( ( weapAmmo - weapAmmoInClip ) / 10 <= 9999 )
				tempAmmoValue = ( weapAmmo - weapAmmoInClip ) / 10;
		
			ammoClipToShow = weapAmmoInClip / 10;
			ammoRestToShow = tempAmmoValue;
		}
		else
		{
			int tempAmmoValue = weapAmmo - weapAmmoInClip;
			if ( weapAmmo - weapAmmoInClip > 9999 )
				tempAmmoValue = 9999;

			ammoClipToShow = weapAmmoInClip;
			ammoRestToShow = tempAmmoValue;
		}
		sprintf(AmmoText, "%d-%d", ammoRestToShow, ammoClipToShow);
	}
	CFont::SetTextBackground(0, 0);
	CFont::SetTextLetterSize(_width(0.24), _height(0.58));
	CFont::SetTextAlignment(ALIGN_Center);
	CFont::func_7194E0(_width(640.0));
	CFont::SetTextUseProportionalValues(false);
	CFont::SetTextOutline(0);
	CFont::SetTextBorderRGBA(CRGBA(0, 0, 0, 255));
	if ( hud.IsStyleLCS() )
		CFont::SetFontStyle(FONT_Eurostile);
	else
		CFont::SetFontStyle(FONT_PagerFont);

	if (	weapAmmo - weapAmmoInClip < 9999
		&& !CDarkel::FrenzyOnGoing()
		&& weapType != 40
		&& ( weapType < 10 || weapType >= 16 )
		&& weapType != 46
		&& CWeaponInfo::GetWeaponInfo(weapType, 1)->GetWeaponType() != 5
		&& CWeaponInfo::GetWeaponInfo(weapType, 1)->GetWeaponSlot() > 1 )
	{
		CFont::SetTextColour(CRGBA(BaseColors[4]));
		CFont::PrintString(fX, fY, AmmoText);
	}
}

void CHud::DrawRadioName(void* object, const char* radioName)
{
	CFont::SetFontStyle(FONT_RageItalic);
	CFont::SetTextJustify(false);
	CFont::SetTextBackground(0, 0);
	CFont::SetTextLetterSize(_width(0.6), _height(1.0));
	CFont::SetTextUseProportionalValues(true);
	CFont::SetTextAlignment(ALIGN_Center);
	CFont::SetTextWrapX(0.0);
	CFont::SetTextShadow(1);
	CFont::SetTextBorderRGBA(CRGBA(0, 0, 0, 255));

	CRGBA	radioColour;
	if ( *((DWORD*)object + 27) || *((DWORD*)object + 28) )
		radioColour = CRGBA(BaseColors[12]);
	else
		radioColour = CRGBA(BaseColors[2]);
	CFont::SetTextColour(radioColour);
	CFont::PrintString(RsGlobal.MaximumWidth / 2, _y(15.0), radioName);
	CFont::func_71A210();
}

void CHud::DrawVehicleName()
{
	if ( hud.pCarName )
	{
		int tmpStage = hud.CarNameStage;
		char* tmpCarName = hud.pLastCarName;
		float alpha = 0.0;

		if ( hud.pCarName != hud.pLastCarName )
		{
			if ( tmpStage )
			{
				if ( tmpStage <= 4 )
				{
					tmpStage = 4;
					hud.CarNameStage = 4;
					hud.CarNameShowingTime = 0;
				}
			}
			else
			{
				tmpStage = 1;
				hud.CarNameStage = 1;
				hud.CarNameShowingTime = 0;
				hud.pCarNameToDisplay = hud.pCarName;
			}
			tmpCarName = hud.pCarName;
			hud.pLastCarName = hud.pCarName;
		}

		if ( tmpStage )
		{
			switch ( tmpStage )
			{
				case 2:
					hud.CarNameFadingTime += CTimer::ms_fTimeStep * 0.02 * 1000.0;
					if ( hud.CarNameFadingTime > 1000 )
					{
						hud.CarNameFadingTime = 1000;
						hud.CarNameStage = 1;
					}
					alpha = hud.CarNameFadingTime * 0.001 * 255.0;
					break;

				case 3:
					hud.CarNameFadingTime += CTimer::ms_fTimeStep * 0.02 * -1000.0;
					if ( hud.CarNameFadingTime < 0 )
					{
						hud.CarNameFadingTime = 0;
						hud.CarNameStage = 0;
					}
					alpha = hud.CarNameFadingTime * 0.001 * 255.0;
					break;

				case 4:
					hud.CarNameFadingTime += CTimer::ms_fTimeStep * 0.02 * -1000.0;
					if ( hud.CarNameFadingTime < 0 )
					{
						hud.CarNameShowingTime = 0;
						hud.CarNameStage = 2;
						hud.pCarNameToDisplay = tmpCarName;
						hud.CarNameFadingTime = 0;
					}
					alpha = hud.CarNameFadingTime * 0.001 * 255.0;
					break;

				case 1:
					if ( hud.CarNameShowingTime > 10000 )
					{
						hud.CarNameStage = 3;
						hud.CarNameFadingTime = 1000;
					}
					alpha = 255.0;
					break;

				default: 
					break;
			}
		}
		if ( !PriorityText[0] )
		{
			hud.CarNameShowingTime += CTimer::ms_fTimeStep * 0.02 * 1000.0;
			
			CFont::SetTextUseProportionalValues(true);
			CFont::SetTextAlignment(ALIGN_Right);
			CFont::SetFontStyle(FONT_RageItalic);
			CFont::SetTextLetterSizeWithLanguageScaling(_width(1.0), _height(1.5));
			CFont::SetTextShadow(2);
#ifdef COMPILE_SLANTED_TEXT
#pragma message ("INFO: Compiling slanted text...")
			CFont::ResetSlantedTextPos(RsGlobal.MaximumWidth, _y(340.0));
			CFont::SetTextSlanted(0.15);
#endif
			CFont::SetTextBorderRGBA(CRGBA(0, 0, 0, (BYTE)alpha));
			CFont::SetTextColour(CRGBA(BaseColors[4], (BYTE)alpha));
			if( *bWants_To_Draw_Hud && alpha )
				CFont::PrintString(_x(20.0), _ydown(78.0), hud.pCarNameToDisplay);
#ifdef COMPILE_SLANTED_TEXT
			CFont::SetTextSlanted(0.0);
#endif
		}
	}
	else
	{
		hud.CarNameStage = 0;
		hud.CarNameShowingTime = 0;
		hud.CarNameFadingTime = 0;
		hud.pLastCarName = 0;
	}
}

void CHud::DrawAreaText()
{
	if ( hud.m_pZoneName )
	{
		int tmpStage = hud.m_ZoneState;
		float alpha = 0.0;

		if ( hud.m_pZoneName != hud.m_pLastZoneName )
		{
			switch ( hud.m_ZoneState )
			{
				case 0:
					if( !*radarGrey && *bWants_To_Draw_Hud || *PlayerEnexEntryStage == 1 || *PlayerEnexEntryStage == 2)
					{
						tmpStage = 2;
						hud.m_ZoneState = 2;
						hud.m_ZoneNameTimer = 0;
						hud.m_ZoneFadeTimer = 0;
						hud.m_ZoneToPrint = hud.m_pZoneName;
					}
					break;
				case 1:
				case 2:
				case 3:
					tmpStage = 4;
					hud.m_ZoneState = 4;
				case 4:
//zone_zero_timer:
					hud.m_ZoneNameTimer = 0;
					break;
				default:
					break;
			}
			hud.m_pLastZoneName = hud.m_pZoneName;
		}

		alpha = 255.0;

		if ( tmpStage )
		{
			switch ( tmpStage )
			{
				case 1:
					alpha = 255.0;
					hud.m_ZoneFadeTimer = 1000;

					if( hud.m_ZoneNameTimer > 10000 )
					{
						hud.m_ZoneState = 3;
						hud.m_ZoneFadeTimer = 1000;
					}
					break;
				case 2:
					if ( !camera->GetFading() )
					{
						if ( camera->GetFadeStage() != 2 )
							hud.m_ZoneFadeTimer += CTimer::ms_fTimeStep * 0.02 * 1000.0;
					}
					if ( hud.m_ZoneFadeTimer > 1000 )
					{
						hud.m_ZoneFadeTimer = 1000;
						hud.m_ZoneState = 1;
					}
					if ( camera->GetFadeStage() != 2 )
						alpha = hud.m_ZoneFadeTimer * 0.001 * 255.0;
					else
					{
						alpha = 255.0;
						hud.m_ZoneState = 3;
						hud.m_ZoneFadeTimer = 1000;
					}
					break;
				case 3:
					if ( !camera->GetFading() )
					{
						if ( camera->GetFadeStage() != 2 )
							hud.m_ZoneFadeTimer += CTimer::ms_fTimeStep * 0.02 * -1000.0;
					}
					if ( hud.m_ZoneFadeTimer < 0 )
					{
						hud.m_ZoneFadeTimer = 0;
						hud.m_ZoneState = 0;
					}
					if ( camera->GetFadeStage() != 2 )
						alpha = hud.m_ZoneFadeTimer * 0.001 * 255.0;
					else
					{
						hud.m_ZoneFadeTimer = 1000;
						alpha = 255.0;
					}
					break;
				case 4:
					hud.m_ZoneFadeTimer += CTimer::ms_fTimeStep * 0.02 * -1000.0;
					
					if( hud.m_ZoneFadeTimer < 0 )
					{
						hud.m_ZoneFadeTimer = 0;
						hud.m_ZoneState = 2;
						hud.m_ZoneToPrint = hud.m_pZoneName;
					}
					
					alpha = hud.m_ZoneFadeTimer * 0.001 * 255.0;
					break;
				default: 
					break;
			}
			if( PriorityText[0] || *(float*)0xBAA3E0 != 0.0 || *(float*)0xBAA3E4 != 0.0 )
			{
				hud.m_ZoneState = 3;
			}
			else
			{
				hud.m_ZoneNameTimer += CTimer::ms_fTimeStep * 0.02 * 1000.0;
				CFont::SetTextUseProportionalValues(true);
				CFont::SetTextAlignment(ALIGN_Right);
				CFont::SetFontStyle(FONT_RageItalic);
				CFont::SetTextShadow(2);
#ifdef COMPILE_SLANTED_TEXT
				CFont::ResetSlantedTextPos(RsGlobal.MaximumWidth, _y(360.0));
				CFont::SetTextSlanted(0.15);
#endif
				CFont::SetTextBorderRGBA(CRGBA(0, 0, 0, alpha));
				CFont::SetTextLetterSize(_width(1.2), _height(1.7));
				CFont::SetTextColour(CRGBA(BaseColors[4], alpha));
				CFont::PrintString(_x(20.0), _ydown(58.0), hud.m_ZoneToPrint);
				CFont::SetTextShadow(0);
#ifdef COMPILE_SLANTED_TEXT
				CFont::SetTextSlanted(0.0);
#endif
			}
		}
	}
}

void CHud::DrawBigMessage(bool bHideOnFade)
{
	if ( bHideOnFade == *bHideStyledTextWhileFading )
	{
		// Big message 5
		if ( !StyledText_2[0] && StyledText_5[0] )
		{
		}
	}

	if ( bHideOnFade )
	{
		// Styled text 7
		if ( StyledText_7[0] )
		{
			CFont::SetTextBackground(0, 0);
			CFont::SetTextJustify(false);
			CFont::SetTextLetterSizeWithLanguageScaling(_width(0.6), _height(1.1));
			CFont::SetTextAlignment(ALIGN_Center);
			CFont::SetTextUseProportionalValues(true);
			CFont::func_7194E0(_width(500.0));
			CFont::SetFontStyle(FONT_Eurostile);
			CFont::SetTextOutline(1);
			CFont::SetTextBorderRGBA(CRGBA(0, 0, 0, 255));
			CFont::SetTextColour(CRGBA(BaseColors[9]));
			CFont::PrintString(RsGlobal.MaximumWidth / 2, _ymiddle(26.0), StyledText_7);
		}

		// Styled text 4
		if ( StyledText_4[0] )
		{
			CFont::SetTextBackground(0, 0);
			CFont::SetTextJustify(false);
			CFont::SetTextLetterSizeWithLanguageScaling(_width(0.6), _height(1.35));
			CFont::SetTextAlignment(ALIGN_Center);
			CFont::SetTextUseProportionalValues(true);
			CFont::func_7194E0(_width(500.0));
			CFont::SetFontStyle(FONT_Eurostile);
			CFont::SetTextOutline(1);
			CFont::SetTextBorderRGBA(CRGBA(0, 0, 0, 255));
			CFont::SetTextColour(BaseColors[6]);
			CFont::PrintString(RsGlobal.MaximumWidth / 2, _ydown(22.5f + WidescreenSupport::GetTextPosition())/*_ydown(103.0)*/, StyledText_4);
		}
		BigMessageRestOfFunc();

	}
}

void CHud::DrawBigMessage1()
{
	CFont::SetTextBackground(0, 0);
	CFont::SetTextJustify(false);
	CFont::SetTextLetterSizeWithLanguageScaling(_width(1.3), _height(1.8));
	CFont::SetTextAlignment(ALIGN_Center);
	CFont::SetTextUseProportionalValues(true);
	CFont::func_7194E0(_width(590.0));
	CFont::SetFontStyle(FONT_Pricedown);
	CFont::SetTextShadow(3);
}

void CHud::DrawBigMessage2()
{
	CFont::SetTextBackground(0, 0);
	CFont::SetTextUseProportionalValues(true);
	CFont::SetTextWrapX(0.0);
	CFont::SetTextAlignment(ALIGN_Right);
	CFont::SetFontStyle(FONT_RageItalic);
	CFont::SetTextLetterSize(_width(1.0), _height(1.2));
	CFont::SetTextShadow(2);
	CFont::SetTextBorderRGBA(CRGBA(0, 0, 0, hud.BigMessage2Alpha));
	CFont::SetTextColour(CRGBA(0xDA, 0xA8, 0x02, hud.BigMessage2Alpha));
	//CFont::SetTextColour(RwRGBA(&BaseColors[6], hud.BigMessage2Alpha));
	CFont::UnkPrintString(_x(20.0), _ydown(38.0), StyledText_2);
}


void CHud::DrawBigMessage3()
{
	CFont::SetTextBackground(0, 0);
	CFont::SetTextLetterSize(_width(1.3), _height(1.8));
	CFont::SetTextUseProportionalValues(true);
	CFont::SetTextJustify(false);
	CFont::SetTextAlignment(ALIGN_Center);
	CFont::SetFontStyle(FONT_Pricedown);
	CFont::SetTextShadow(3);
	CFont::SetTextBorderRGBA(CRGBA(0, 0, 0, hud.BigMessage3Alpha));
	CFont::SetTextColour(CRGBA(BaseColors[10], hud.BigMessage3Alpha));
	CFont::UnkPrintString(RsGlobal.MaximumWidth / 2, _ymiddle(-30.0), StyledText_3);
}

/*void CHud::DrawDevLogos()
{
#if DEBUG
	if ( !hud.bDrawDevLogos )
		return;

	DevLogos[0].DrawTexturedRect(&CRect( 
											_x(170.0), _y(460.0),
											_x(170.0) + _width(175.0), 
											_y(460.0) - _height(175.0)), 
										&CRGBA(255, 255, 255, 255));

	/*DevLogos[1].DrawTexturedRect(&CRect( 
											_x(637.5), _y(450.0),
											_x(637.5) + _width(45.0), 
											_y(450.0) - _height(45.0)), 
										&RwRGBA(255, 255, 255, 255));*/

/*#endif
}*/
							

void CHud::DrawBarChartWithRoundBorder(float fX, float fY, WORD wWidth, WORD wHeight, float fPercentage, BYTE drawBlueLine, BYTE drawPercentage, BYTE drawBorder, CRGBA dwColor, CRGBA dwForeColor)
{
	DrawBarChart(fX, fY, wWidth, wHeight, fPercentage, drawBlueLine, drawPercentage, 0, dwColor, dwForeColor);

	RwRenderStateSet(rwRENDERSTATEZTESTENABLE, FALSE);
	CRGBA	BarRGBA(0, 0, 0, 255);
	CRect	BarSize;

	BarSize.x1 = fX - _width(2.0);
	BarSize.y1 = fY - _height(1.5);
	BarSize.x2 = fX + wWidth + _width(2.0);
	BarSize.y2 = fY + wHeight+ _height(1.5);
	hudTextures[HUD_BarOutline].DrawTexturedRect(&BarSize, &BarRGBA);
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, FALSE);
}

/*void CHud::DrawSquareBar(float fX, float fY, WORD wWidth, WORD wHeight, float fPercentage, BYTE drawBlueLine, BYTE drawShadow, BYTE drawBorder, CRGBA dwColour, CRGBA dwForeColor)
{
	CRect coords;

	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, NULL);
	RwRenderStateSet(rwRENDERSTATESHADEMODE, (void*)rwSHADEMODEFLAT);
	
	wHeight /= 2.0;
	fY += wHeight;
	if (fPercentage < 0.0)
		fPercentage = 0.0;
	if ( drawBorder )
	{
		/*RwRGBA backColour(0, 0, 0, 255);

		coords.x1 = fX - (_x((640.0 - 1.0 * drawBorder)));
		coords.y1 = fY - (_y(1.0 * drawBorder));
		coords.x2 = fX + wWidth + (_width(1.0 * drawBorder));
		coords.y2 = fY + wHeight + (_height(1.0 * drawBorder));
		CDraw::Rect(&coords, &backColour);*/

/*		CDraw::Rect(&CRect(	fX - (_x((640.0 - 1.0 * drawBorder))), 
								fY - (_y(1.0 * drawBorder)), 
								fX + wWidth + (_width(1.0 * drawBorder)),
								fY + wHeight + (_height(1.0 * drawBorder))), 
					&CRGBA(0, 0, 0, 255));
	}
	if ( drawShadow )
	{
		CRGBA shadowColour(0, 0, 0, 200);

		coords.x1 = fX + (_x((640.0 - 2.0 * drawShadow)));
		coords.y1 = fY + (_y(2.0 * drawShadow));
		coords.x2 = fX + wWidth + (_width(2.0 * drawShadow));
		coords.y2 = fY + wHeight + (_height(2.0 * drawShadow));
		CDraw::Rect(&coords, &shadowColour);
	}
	CRGBA whiteColour(255, 255, 255, 255);

	coords.x1 = fX;
	coords.y1 = fY;
	coords.x2 = fX + wWidth;
	coords.y2 = fY + wHeight;
	CDraw::Rect(&coords, &whiteColour);
	coords.x2 = fX + wWidth * ((fPercentage + 1.0) / 100.0);
	CDraw::Rect(&coords, &dwColour);
}*/

// This is just ugh
void CHud::DrawSquareBar(float fX, float fY, WORD wWidth, WORD wHeight, float fPercentage, BYTE drawBlueLine, BYTE drawShadow, BYTE drawBorder, CRGBA dwColour, CRGBA dwForeColor)
{
	CRect coords;

	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, nullptr);
	RwRenderStateSet(rwRENDERSTATESHADEMODE, (void*)rwSHADEMODEFLAT);

	wWidth *= *ScreenAspectRatio * (3.0/4.0);
	
	wHeight /= 2.0;
	fY += wHeight;
	if (fPercentage < 0.0)
		fPercentage = 0.0;
	if ( drawBorder )
	{
		/*RwRGBA backColour(0, 0, 0, 255);

		coords.x1 = fX - (_x((640.0 - 1.0 * drawBorder)));
		coords.y1 = fY - (_y(1.0 * drawBorder));
		coords.x2 = fX + wWidth + (_width(1.0 * drawBorder));
		coords.y2 = fY + wHeight + (_height(1.0 * drawBorder));
		CDraw::Rect(&coords, &backColour);*/

		CSprite2d::DrawRect(CRect(	fX - (_xleft((1.0 * drawBorder))), 
								fY - (_y(1.0 * drawBorder)), 
								fX + wWidth + (_width(1.0 * drawBorder)),
								fY + wHeight + (_height(1.0 * drawBorder))), 
					CRGBA(0, 0, 0, 255));
	}
	if ( drawShadow )
	{
		CRGBA shadowColour(0, 0, 0, 200);

		coords.x1 = fX + (_xleft((2.0 * drawShadow)));
		coords.y1 = fY + (_y(2.0 * drawShadow));
		coords.x2 = fX + wWidth + (_width(2.0 * drawShadow));
		coords.y2 = fY + wHeight + (_height(2.0 * drawShadow));
		CSprite2d::DrawRect(coords, shadowColour);
	}
	CRGBA whiteColour(255, 255, 255, 255);

	coords.x1 = fX;
	coords.y1 = fY;
	coords.x2 = fX + wWidth;
	coords.y2 = fY + wHeight;
	CSprite2d::DrawRect(coords, whiteColour);
	coords.x2 = fX + wWidth * ((fPercentage + 1.0) / 100.0);
	CSprite2d::DrawRect(coords, dwColour);
}

float CHud::GetScreenCoordsForPlayerItem(BYTE plrID, float position, BYTE offset)
{
	return position;
}

bool CHud::HelpMessageShown()
{
	return m_HelpMessageState != 0 || m_PagerMessage[0];
}