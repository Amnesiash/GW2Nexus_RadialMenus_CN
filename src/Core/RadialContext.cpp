///----------------------------------------------------------------------------------------------------
/// Copyright (c) Raidcore.GG - All rights reserved.
///
/// Name         :  RadialContext.h
/// Description  :  Primary interface for radial menus.
/// Authors      :  K. Bieniek
///----------------------------------------------------------------------------------------------------

#include "RadialContext.h"

#include <map>
#include <fstream>
#include <thread>

#include "imgui/imgui_internal.h"
#include "imgui_extensions.h"
#include "nlohmann/json.hpp"
using json = nlohmann::json;

#include "Addon.h"
#include "Language.h"
#include "Shared.h"
#include "Util.h"
#include "resource.h"
#include "StateObserver.h"

#define NORMAL_ITEM_COLOR ImColor(255, 255, 255, 255)
#define NORMAL_ITEM_COLOR_HOVER ImColor(245, 192, 67, 255)
#define SMALL_ITEM_COLOR ImColor(255, 255, 255, 0)
#define SMALL_ITEM_COLOR_HOVER ImColor(255, 255, 255, 255)

/* helpers start */
static bool HasChanges = false;

static std::mutex UnboundKeysMutex;
static std::map<EGameBinds, std::vector<std::string>> UnboundKeys;

void GameBindSelectable(ActionBase* aAction, const char* aLabel, EGameBinds aGameBind)
{
	bool isBound = APIDefs->GameBinds.IsBound(aGameBind);

	if (!isBound)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)ImColor(255, 255, 0, 255));
	}

	if (ImGui::Selectable((APIDefs->Localization.Translate(aLabel) + ("##" + std::to_string(aGameBind))).c_str()))
	{
		((ActionGameInputBind*)aAction)->Identifier = aGameBind;
		HasChanges = true;
	}

	if (!isBound)
	{
		ImGui::PopStyleColor();
		ImGui::TooltipGeneric(Lang::T("RM_TIP_UNBOUND_GAMEBIND"));
	}
}

std::string GameBindToString(EGameBinds aGameBind)
{
	static std::map<EGameBinds, std::string> LookupTable =
	{
		// Movement
		{ EGameBinds_MoveForward, "((MoveForward))" },
		{ EGameBinds_MoveBackward, "((MoveBackward))" },
		{ EGameBinds_MoveLeft, "((MoveLeft))" },
		{ EGameBinds_MoveRight, "((MoveRight))" },
		{ EGameBinds_MoveTurnLeft, "((MoveTurnLeft))" },
		{ EGameBinds_MoveTurnRight, "((MoveTurnRight))" },
		{ EGameBinds_MoveDodge, "((MoveDodge))" },
		{ EGameBinds_MoveAutoRun, "((MoveAutoRun))" },
		{ EGameBinds_MoveWalk, "((MoveWalk))" },
		{ EGameBinds_MoveJump_SwimUp_FlyUp, "((MoveJump))"},
		{ EGameBinds_MoveSwimDown_FlyDown, "((MoveSwimDown))" },
		{ EGameBinds_MoveAboutFace, "((MoveAboutFace))" },

		// Skills
		{ EGameBinds_SkillWeaponSwap, "((SkillWeaponSwap))" },
		{ EGameBinds_SkillWeapon1, "((SkillWeapon1))" },
		{ EGameBinds_SkillWeapon2, "((SkillWeapon2))" },
		{ EGameBinds_SkillWeapon3, "((SkillWeapon3))" },
		{ EGameBinds_SkillWeapon4, "((SkillWeapon4))" },
		{ EGameBinds_SkillWeapon5, "((SkillWeapon5))" },
		{ EGameBinds_SkillHeal, "((SkillHeal))" },
		{ EGameBinds_SkillUtility1, "((SkillUtility1))" },
		{ EGameBinds_SkillUtility2, "((SkillUtility2))" },
		{ EGameBinds_SkillUtility3, "((SkillUtility3))" },
		{ EGameBinds_SkillElite, "((SkillElite))" },
		{ EGameBinds_SkillProfession1, "((SkillProfession1))" },
		{ EGameBinds_SkillProfession2, "((SkillProfession2))" },
		{ EGameBinds_SkillProfession3, "((SkillProfession3))" },
		{ EGameBinds_SkillProfession4, "((SkillProfession4))" },
		{ EGameBinds_SkillProfession5, "((SkillProfession5))" },
		{ EGameBinds_SkillProfession6, "((SkillProfession6))" },
		{ EGameBinds_SkillProfession7, "((SkillProfession7))" },
		{ EGameBinds_SkillSpecialAction, "((SkillSpecialAction))" },

		// Targeting
		{ EGameBinds_TargetAlert, "((TargetAlert))" },
		{ EGameBinds_TargetCall, "((TargetCall))" },
		{ EGameBinds_TargetTake, "((TargetTake))" },
		{ EGameBinds_TargetCallLocal, "((TargetCallLocal))" },
		{ EGameBinds_TargetTakeLocal, "((TargetTakeLocal))" },
		{ EGameBinds_TargetEnemyNearest, "((TargetEnemyNearest))" },
		{ EGameBinds_TargetEnemyNext, "((TargetEnemyNext))" },
		{ EGameBinds_TargetEnemyPrev, "((TargetEnemyPrev))" },
		{ EGameBinds_TargetAllyNearest, "((TargetAllyNearest))" },
		{ EGameBinds_TargetAllyNext, "((TargetAllyNext))" },
		{ EGameBinds_TargetAllyPrev, "((TargetAllyPrev))" },
		{ EGameBinds_TargetLock, "((TargetLock))" },
		{ EGameBinds_TargetSnapGroundTarget, "((TargetSnapGroundTarget))" },
		{ EGameBinds_TargetSnapGroundTargetToggle, "((TargetSnapGroundTargetToggle))" },
		{ EGameBinds_TargetAutoTargetingDisable, "((TargetAutoTargetingDisable))" },
		{ EGameBinds_TargetAutoTargetingToggle, "((TargetAutoTargetingToggle))" },
		{ EGameBinds_TargetAllyTargetingMode, "((TargetAllyTargetingMode))" },
		{ EGameBinds_TargetAllyTargetingModeToggle, "((TargetAllyTargetingModeToggle))" },

		// UI Binds
		{ EGameBinds_UiCommerce, "((UiCommerce))" }, // TradingPost
		{ EGameBinds_UiContacts, "((UiContacts))" },
		{ EGameBinds_UiGuild, "((UiGuild))" },
		{ EGameBinds_UiHero, "((UiHero))" },
		{ EGameBinds_UiInventory, "((UiInventory))" },
		{ EGameBinds_UiKennel, "((UiKennel))" }, // Pets
		{ EGameBinds_UiLogout, "((UiLogout))" },
		{ EGameBinds_UiMail, "((UiMail))" },
		{ EGameBinds_UiOptions, "((UiOptions))" },
		{ EGameBinds_UiParty, "((UiParty))" },
		{ EGameBinds_UiPvp, "((UiPvp))" },
		{ EGameBinds_UiPvpBuild, "((UiPvpBuild))" },
		{ EGameBinds_UiScoreboard, "((UiScoreboard))" },
		{ EGameBinds_UiSeasonalObjectivesShop, "((UiSeasonalObjectivesShop))" }, // Wizard's Vault
		{ EGameBinds_UiInformation, "((UiInformation))" },
		{ EGameBinds_UiChatToggle, "((UiChatToggle))" },
		{ EGameBinds_UiChatCommand, "((UiChatCommand))" },
		{ EGameBinds_UiChatFocus, "((UiChatFocus))" },
		{ EGameBinds_UiChatReply, "((UiChatReply))" },
		{ EGameBinds_UiToggle, "((UiToggle))" },
		{ EGameBinds_UiSquadBroadcastChatToggle, "((UiSquadBroadcastChatToggle))" },
		{ EGameBinds_UiSquadBroadcastChatCommand, "((UiSquadBroadcastChatCommand))" },
		{ EGameBinds_UiSquadBroadcastChatFocus, "((UiSquadBroadcastChatFocus))" },

		// Camera
		{ EGameBinds_CameraFree, "((CameraFree))" },
		{ EGameBinds_CameraZoomIn, "((CameraZoomIn))" },
		{ EGameBinds_CameraZoomOut, "((CameraZoomOut))" },
		{ EGameBinds_CameraReverse, "((CameraReverse))" },
		{ EGameBinds_CameraActionMode, "((CameraActionMode))" },
		{ EGameBinds_CameraActionModeDisable, "((CameraActionModeDisable))" },

		// Screenshots
		{ EGameBinds_ScreenshotNormal, "((ScreenshotNormal))" },
		{ EGameBinds_ScreenshotStereoscopic, "((ScreenshotStereoscopic))" },

		// Map
		{ EGameBinds_MapToggle, "((MapToggle))" },
		{ EGameBinds_MapFocusPlayer, "((MapFocusPlayer))" },
		{ EGameBinds_MapFloorDown, "((MapFloorDown))" },
		{ EGameBinds_MapFloorUp, "((MapFloorUp))" },
		{ EGameBinds_MapZoomIn, "((MapZoomIn))" },
		{ EGameBinds_MapZoomOut, "((MapZoomOut))" },

		// Mounts
		{ EGameBinds_SpumoniToggle, "((SpumoniToggle))" },
		{ EGameBinds_SpumoniMovement, "((SpumoniMovement))" },
		{ EGameBinds_SpumoniSecondaryMovement, "((SpumoniSecondaryMovement))" },
		{ EGameBinds_SpumoniMAM01, "((SpumoniMAM01))" }, // Raptor
		{ EGameBinds_SpumoniMAM02, "((SpumoniMAM02))" }, // Springer
		{ EGameBinds_SpumoniMAM03, "((SpumoniMAM03))" }, // Skimmer
		{ EGameBinds_SpumoniMAM04, "((SpumoniMAM04))" }, // Jackal
		{ EGameBinds_SpumoniMAM05, "((SpumoniMAM05))" }, // Griffon
		{ EGameBinds_SpumoniMAM06, "((SpumoniMAM06))" }, // RollerBeetle
		{ EGameBinds_SpumoniMAM07, "((SpumoniMAM07))" }, // Warclaw
		{ EGameBinds_SpumoniMAM08, "((SpumoniMAM08))" }, // Skyscale
		{ EGameBinds_SpumoniMAM09, "((SpumoniMAM09))" }, // SiegeTurtle

		// Spectator Binds
		{ EGameBinds_SpectatorNearestFixed, "((SpectatorNearestFixed))" },
		{ EGameBinds_SpectatorNearestPlayer, "((SpectatorNearestPlayer))" },
		{ EGameBinds_SpectatorPlayerRed1, "((SpectatorPlayerRed1))" },
		{ EGameBinds_SpectatorPlayerRed2, "((SpectatorPlayerRed2))" },
		{ EGameBinds_SpectatorPlayerRed3, "((SpectatorPlayerRed3))" },
		{ EGameBinds_SpectatorPlayerRed4, "((SpectatorPlayerRed4))" },
		{ EGameBinds_SpectatorPlayerRed5, "((SpectatorPlayerRed5))" },
		{ EGameBinds_SpectatorPlayerBlue1, "((SpectatorPlayerBlue1))" },
		{ EGameBinds_SpectatorPlayerBlue2, "((SpectatorPlayerBlue2))" },
		{ EGameBinds_SpectatorPlayerBlue3, "((SpectatorPlayerBlue3))" },
		{ EGameBinds_SpectatorPlayerBlue4, "((SpectatorPlayerBlue4))" },
		{ EGameBinds_SpectatorPlayerBlue5, "((SpectatorPlayerBlue5))" },
		{ EGameBinds_SpectatorFreeCamera, "((SpectatorFreeCamera))" },
		{ EGameBinds_SpectatorFreeCameraMode, "((SpectatorFreeCameraMode))" },
		{ EGameBinds_SpectatorFreeMoveForward, "((SpectatorFreeMoveForward))" },
		{ EGameBinds_SpectatorFreeMoveBackward, "((SpectatorFreeMoveBackward))" },
		{ EGameBinds_SpectatorFreeMoveLeft, "((SpectatorFreeMoveLeft))" },
		{ EGameBinds_SpectatorFreeMoveRight, "((SpectatorFreeMoveRight))" },
		{ EGameBinds_SpectatorFreeMoveUp, "((SpectatorFreeMoveUp))" },
		{ EGameBinds_SpectatorFreeMoveDown, "((SpectatorFreeMoveDown))" },

		// Squad Markers
		{ EGameBinds_SquadMarkerPlaceWorld1, "((SquadMarkerPlaceWorld1))" }, // Arrow
		{ EGameBinds_SquadMarkerPlaceWorld2, "((SquadMarkerPlaceWorld2))" }, // Circle
		{ EGameBinds_SquadMarkerPlaceWorld3, "((SquadMarkerPlaceWorld3))" }, // Heart
		{ EGameBinds_SquadMarkerPlaceWorld4, "((SquadMarkerPlaceWorld4))" }, // Square
		{ EGameBinds_SquadMarkerPlaceWorld5, "((SquadMarkerPlaceWorld5))" }, // Star
		{ EGameBinds_SquadMarkerPlaceWorld6, "((SquadMarkerPlaceWorld6))" }, // Swirl
		{ EGameBinds_SquadMarkerPlaceWorld7, "((SquadMarkerPlaceWorld7))" }, // Triangle
		{ EGameBinds_SquadMarkerPlaceWorld8, "((SquadMarkerPlaceWorld8))" }, // Cross
		{ EGameBinds_SquadMarkerClearAllWorld, "((SquadMarkerClearAllWorld))" },
		{ EGameBinds_SquadMarkerSetAgent1, "((SquadMarkerSetAgent1))" }, // Arrow
		{ EGameBinds_SquadMarkerSetAgent2, "((SquadMarkerSetAgent2))" }, // Circle
		{ EGameBinds_SquadMarkerSetAgent3, "((SquadMarkerSetAgent3))" }, // Heart
		{ EGameBinds_SquadMarkerSetAgent4, "((SquadMarkerSetAgent4))" }, // Square
		{ EGameBinds_SquadMarkerSetAgent5, "((SquadMarkerSetAgent5))" }, // Star
		{ EGameBinds_SquadMarkerSetAgent6, "((SquadMarkerSetAgent6))" }, // Swirl
		{ EGameBinds_SquadMarkerSetAgent7, "((SquadMarkerSetAgent7))" }, // Triangle
		{ EGameBinds_SquadMarkerSetAgent8, "((SquadMarkerSetAgent8))" }, // Cross
		{ EGameBinds_SquadMarkerClearAllAgent, "((SquadMarkerClearAllAgent))" },

		// Mastery Skills
		{ EGameBinds_MasteryAccess, "((MasteryAccess))" },
		{ EGameBinds_MasteryAccess01, "((MasteryAccess01))" }, // Fishing
		{ EGameBinds_MasteryAccess02, "((MasteryAccess02))" }, // Skiff
		{ EGameBinds_MasteryAccess03, "((MasteryAccess03))" }, // Jade Bot Waypoint
		{ EGameBinds_MasteryAccess04, "((MasteryAccess04))" }, // Rift Scan
		{ EGameBinds_MasteryAccess05, "((MasteryAccess05))" }, // Skyscale
		{ EGameBinds_MasteryAccess06, "((MasteryAccess06))" }, // Homestead Doorway

		// Miscellaneous Binds
		{ EGameBinds_MiscAoELoot, "((MiscAoELoot))" },
		{ EGameBinds_MiscInteract, "((MiscInteract))" },
		{ EGameBinds_MiscShowEnemies, "((MiscShowEnemies))" },
		{ EGameBinds_MiscShowAllies, "((MiscShowAllies))" },
		{ EGameBinds_MiscCombatStance, "((MiscCombatStance))" }, // Stow/Draw
		{ EGameBinds_MiscToggleLanguage, "((MiscToggleLanguage))" },
		{ EGameBinds_MiscTogglePetCombat, "((MiscTogglePetCombat))" },
		{ EGameBinds_MiscToggleFullScreen, "((MiscToggleFullScreen))" },
		{ EGameBinds_MiscToggleDecorationMode, "((MiscToggleDecorationMode))" }, // Decoration Mode

		// Toys/Novelties
		{ EGameBinds_ToyUseDefault, "((ToyUseDefault))" },
		{ EGameBinds_ToyUseSlot1, "((ToyUseSlot1))" }, // Chair
		{ EGameBinds_ToyUseSlot2, "((ToyUseSlot2))" }, // Instrument
		{ EGameBinds_ToyUseSlot3, "((ToyUseSlot3))" }, // Held Item
		{ EGameBinds_ToyUseSlot4, "((ToyUseSlot4))" }, // Toy
		{ EGameBinds_ToyUseSlot5, "((ToyUseSlot5))" }, // Tonic
		//ToyUseSlot6 unused

		// Build Templates
		{ EGameBinds_Loadout1, "((Loadout1))" },
		{ EGameBinds_Loadout2, "((Loadout2))" },
		{ EGameBinds_Loadout3, "((Loadout3))" },
		{ EGameBinds_Loadout4, "((Loadout4))" },
		{ EGameBinds_Loadout5, "((Loadout5))" },
		{ EGameBinds_Loadout6, "((Loadout6))" },
		{ EGameBinds_Loadout7, "((Loadout7))" },
		{ EGameBinds_Loadout8, "((Loadout8))" },
		{ EGameBinds_Loadout9, "((Loadout9))" },

		// Equipment Templates
		{ EGameBinds_GearLoadout1, "((GearLoadout1))" },
		{ EGameBinds_GearLoadout2, "((GearLoadout2))" },
		{ EGameBinds_GearLoadout3, "((GearLoadout3))" },
		{ EGameBinds_GearLoadout4, "((GearLoadout4))" },
		{ EGameBinds_GearLoadout5, "((GearLoadout5))" },
		{ EGameBinds_GearLoadout6, "((GearLoadout6))" },
		{ EGameBinds_GearLoadout7, "((GearLoadout7))" },
		{ EGameBinds_GearLoadout8, "((GearLoadout8))" },
		{ EGameBinds_GearLoadout9, "((GearLoadout9))" }
	};

	return LookupTable[aGameBind];
}

std::string ConditionsToString(Conditions* aConditions, bool* aLinkWithPrevious = nullptr)
{
	std::string str;
	if (aConditions->IsCombat         != EObserveBoolean::Either) { str.append(aConditions->IsCombat == EObserveBoolean::True ? Lang::T("RM_COND_LINE_IN_COMBAT") : Lang::T("RM_COND_LINE_OUT_COMBAT")); }
	if (aConditions->IsMounted == EObserveMount::Any)
	{
		str.append(Lang::T("RM_COND_LINE_MOUNTED"));
	}
	else if (aConditions->IsMounted == EObserveMount::Either)
	{
		/* nop */
	}
	else if (aConditions->IsMounted == EObserveMount::NotMounted)
	{
		str.append(Lang::T("RM_COND_LINE_NOT_MOUNTED"));
	}
	else
	{
		str.append(std::string(Lang::T("RM_COND_LINE_ON_MOUNT")) + StateObserver::StateToString(aConditions->IsMounted) + "\n");
	}
	if (aConditions->IsCommander      != EObserveBoolean::Either) { str.append(aConditions->IsCommander == EObserveBoolean::True ? Lang::T("RM_COND_LINE_CMD") : Lang::T("RM_COND_LINE_NO_CMD")); }
	if (aConditions->IsCompetitive    != EObserveBoolean::Either) { str.append(aConditions->IsCompetitive == EObserveBoolean::True ? Lang::T("RM_COND_LINE_COMP") : Lang::T("RM_COND_LINE_PVE")); }
	if (aConditions->IsMapOpen        != EObserveBoolean::Either) { str.append(aConditions->IsMapOpen == EObserveBoolean::True ? Lang::T("RM_COND_LINE_MAP_OPEN") : Lang::T("RM_COND_LINE_MAP_CLOSED")); }
	if (aConditions->IsTextboxActive  != EObserveBoolean::Either) { str.append(aConditions->IsTextboxActive == EObserveBoolean::True ? Lang::T("RM_COND_LINE_TEXTBOX") : Lang::T("RM_COND_LINE_NO_TEXTBOX")); }
	if (aConditions->IsInstance       != EObserveBoolean::Either) { str.append(aConditions->IsInstance == EObserveBoolean::True ? Lang::T("RM_COND_LINE_INSTANCE") : Lang::T("RM_COND_LINE_NO_INSTANCE")); }

	/* derived game states */
	if (aConditions->IsGameplay       != EObserveBoolean::Either) { str.append(aConditions->IsGameplay == EObserveBoolean::True ? Lang::T("RM_COND_LINE_GAMEPLAY") : Lang::T("RM_COND_LINE_NOT_GAMEPLAY")); }

	/* derived positional states */
	if (aConditions->IsUnderwater     != EObserveBoolean::Either) { str.append(aConditions->IsUnderwater == EObserveBoolean::True ? Lang::T("RM_COND_LINE_UNDERWATER") : Lang::T("RM_COND_LINE_TERRESTRIAL")); }
	if (aConditions->IsOnWaterSurface != EObserveBoolean::Either) { str.append(aConditions->IsOnWaterSurface == EObserveBoolean::True ? Lang::T("RM_COND_LINE_ON_WATER") : Lang::T("RM_COND_LINE_NOT_ON_WATER")); }
	if (aConditions->IsAirborne       != EObserveBoolean::Either) { str.append(aConditions->IsAirborne == EObserveBoolean::True ? Lang::T("RM_COND_LINE_AIRBORNE") : Lang::T("RM_COND_LINE_NOT_AIRBORNE")); }

	if (!str.empty())
	{
		str = Lang::T("RM_COND_HEADER") + str;
	}

	if (aLinkWithPrevious != nullptr && *aLinkWithPrevious)
	{
		if (!str.empty())
		{
			str.append("\n");
		}
		str.append(Lang::T("RM_COND_LINKED"));
	}
	return str;
}
/* helpers end */

bool ConditionSelectable(std::string aName, EObserveBoolean* aState, const char* aHelpTooltip = nullptr)
{
	ImGui::TableNextRow();
	ImGui::TableSetColumnIndex(0);
	float width = ImGui::GetColumnWidth();
	ImGui::Text(aName.c_str());
	if (aHelpTooltip)
	{
		ImGui::HelpMarker(aHelpTooltip);
	}

	ImGui::TableSetColumnIndex(1);
	std::string state;
	switch (*aState)
	{
		case EObserveBoolean::Either:
			state = Lang::T("RM_STATE_EITHER");
			break;
		case EObserveBoolean::False:
			state = Lang::T("RM_STATE_MUST_FALSE");
			break;
		case EObserveBoolean::True:
			state = Lang::T("RM_STATE_MUST_TRUE");
			break;
	}
	ImGui::PushItemWidth(width);
	if (ImGui::BeginCombo(("##observestate" + aName).c_str(), state.c_str()))
	{
		if (ImGui::Selectable(Lang::T("RM_STATE_EITHER")))
		{
			*aState = EObserveBoolean::Either;
		}
		if (ImGui::Selectable(Lang::T("RM_STATE_MUST_FALSE")))
		{
			*aState = EObserveBoolean::False;
		}
		if (ImGui::Selectable(Lang::T("RM_STATE_MUST_TRUE")))
		{
			*aState = EObserveBoolean::True;
		}
		ImGui::EndCombo();
	}

	ImGui::TableSetColumnIndex(2);
	if (ImGui::Button((std::string(Lang::T("RM_BTN_APPLY_TO_ALL")) + "##applyallcondsel" + aName).c_str()))
	{
		return true;
	}

	return false;
}

bool ConditionSelectable(std::string aName, EObserveMount* aState, const char* aHelpTooltip = nullptr)
{
	ImGui::TableNextRow();
	ImGui::TableSetColumnIndex(0);
	float width = ImGui::GetColumnWidth();
	ImGui::Text(aName.c_str());
	if (aHelpTooltip)
	{
		ImGui::HelpMarker(aHelpTooltip);
	}

	ImGui::TableSetColumnIndex(1);
	std::string state;
	switch (*aState)
	{
		default:
		case EObserveMount::Any:
			state = Lang::T("RM_MOUNT_ANY");
			break;
		case EObserveMount::Either:
			state = Lang::T("RM_STATE_EITHER");
			break;
		case EObserveMount::NotMounted:
			state = Lang::T("RM_MOUNT_NOT");
			break;
		case EObserveMount::Jackal:
			state = Lang::T("RM_MOUNT_JACKAL");
			break;
		case EObserveMount::Griffon:
			state = Lang::T("RM_MOUNT_GRIFFON");
			break;
		case EObserveMount::Springer:
			state = Lang::T("RM_MOUNT_SPRINGER");
			break;
		case EObserveMount::Skimmer:
			state = Lang::T("RM_MOUNT_SKIMMER");
			break;
		case EObserveMount::Raptor:
			state = Lang::T("RM_MOUNT_RAPTOR");
			break;
		case EObserveMount::RollerBeetle:
			state = Lang::T("RM_MOUNT_ROLLERBEETLE");
			break;
		case EObserveMount::Warclaw:
			state = Lang::T("RM_MOUNT_WARCLAW");
			break;
		case EObserveMount::Skyscale:
			state = Lang::T("RM_MOUNT_SKYSCALE");
			break;
		case EObserveMount::Skiff:
			state = Lang::T("RM_MOUNT_SKIFF");
			break;
		case EObserveMount::SiegeTurtle:
			state = Lang::T("RM_MOUNT_SIEGETURTLE");
			break;
	}
	ImGui::PushItemWidth(width);
	if (ImGui::BeginCombo(("##observestate" + aName).c_str(), state.c_str()))
	{
		if (ImGui::Selectable(Lang::T("RM_STATE_EITHER")))
		{
			*aState = EObserveMount::Either;
		}

		if (ImGui::Selectable(Lang::T("RM_MOUNT_ANY")))
		{
			*aState = EObserveMount::Any;
		}

		if (ImGui::Selectable(Lang::T("RM_MOUNT_NOT")))
		{
			*aState = EObserveMount::NotMounted;
		}
		
		if (ImGui::Selectable(Lang::T("RM_MOUNT_RAPTOR")))
		{
			*aState = EObserveMount::Raptor;
		}
		if (ImGui::Selectable(Lang::T("RM_MOUNT_SPRINGER")))
		{
			*aState = EObserveMount::Springer;
		}
		if (ImGui::Selectable(Lang::T("RM_MOUNT_SKIMMER")))
		{
			*aState = EObserveMount::Skimmer;
		}
		if (ImGui::Selectable(Lang::T("RM_MOUNT_JACKAL")))
		{
			*aState = EObserveMount::Jackal;
		}
		if (ImGui::Selectable(Lang::T("RM_MOUNT_GRIFFON")))
		{
			*aState = EObserveMount::Griffon;
		}
		if (ImGui::Selectable(Lang::T("RM_MOUNT_ROLLERBEETLE")))
		{
			*aState = EObserveMount::RollerBeetle;
		}
		if (ImGui::Selectable(Lang::T("RM_MOUNT_WARCLAW")))
		{
			*aState = EObserveMount::Warclaw;
		}
		if (ImGui::Selectable(Lang::T("RM_MOUNT_SKYSCALE")))
		{
			*aState = EObserveMount::Skyscale;
		}
		if (ImGui::Selectable(Lang::T("RM_MOUNT_SKIFF")))
		{
			*aState = EObserveMount::Skiff;
		}
		if (ImGui::Selectable(Lang::T("RM_MOUNT_SIEGETURTLE")))
		{
			*aState = EObserveMount::SiegeTurtle;
		}
		ImGui::EndCombo();
	}

	ImGui::TableSetColumnIndex(2);
	if (ImGui::Button((std::string(Lang::T("RM_BTN_APPLY_TO_ALL")) + "##applyallcondsel" + aName).c_str()))
	{
		return true;
	}

	return false;
}

int ConditionEditor(std::string aName, Conditions* aConditions, std::string aApplyAllInfo, bool* aPreviousReq = nullptr)
{
	std::string popupName = "ConditionEditor##" + aName;

	if (ImGui::Button(aName.c_str()))
	{
		ImGui::OpenPopup(popupName.c_str());
	}

	int stateIndex = -1;

	if (ImGui::BeginPopupContextItem(popupName.c_str()))
	{
		if (ImGui::BeginTable("##conditioneditortable", 3))
		{
			/* direct states */
			if (ConditionSelectable(Lang::T("RM_COND_IS_IN_COMBAT"), &aConditions->IsCombat)) { stateIndex = 0; };
			if (ConditionSelectable(Lang::T("RM_COND_IS_MOUNTED"), &aConditions->IsMounted)) { stateIndex = 1; };
			if (ConditionSelectable(Lang::T("RM_COND_IS_COMMANDER"), &aConditions->IsCommander)) { stateIndex = 2; };
			if (ConditionSelectable(Lang::T("RM_COND_IS_PVPWVW"), &aConditions->IsCompetitive)) { stateIndex = 3; };
			if (ConditionSelectable(Lang::T("RM_COND_IS_MAP_OPEN"), &aConditions->IsMapOpen)) { stateIndex = 4; };
			if (ConditionSelectable(Lang::T("RM_COND_IS_TEXTBOX_ACTIVE"), &aConditions->IsTextboxActive)) { stateIndex = 5; };
			if (ConditionSelectable(Lang::T("RM_COND_IS_INSTANCE"), &aConditions->IsInstance)) { stateIndex = 6; };

			/* derived game states */
			if (ConditionSelectable(Lang::T("RM_COND_IS_GAMEPLAY"), &aConditions->IsGameplay)) { stateIndex = 7; };

			/* derived positional states */
			if (ConditionSelectable(Lang::T("RM_COND_IS_UNDERWATER"), &aConditions->IsUnderwater)) { stateIndex = 8; };
			if (ConditionSelectable(Lang::T("RM_COND_IS_ON_WATER_SURFACE"), &aConditions->IsOnWaterSurface)) { stateIndex = 9; };
			if (ConditionSelectable(Lang::T("RM_COND_IS_AIRBORNE"), &aConditions->IsAirborne, Lang::T("RM_TIP_AIRBORNE"))) { stateIndex = 10; };
			
			ImGui::EndTable();
		}

		if (aPreviousReq)
		{
			if (ImGui::Checkbox(Lang::T("RM_CHK_LINK_WITH_PREVIOUS"), aPreviousReq))
			{
				HasChanges = true;
			}
			ImGui::HelpMarker(Lang::T("RM_TIP_LINK_WITH_PREVIOUS"));
		}

		if (!aApplyAllInfo.empty())
		{
			ImGui::TextDisabled(aApplyAllInfo.c_str());
		}

		if (ImGui::Button((std::string(Lang::T("RM_BTN_APPLY_ALL_CONDITIONS")) + "##applyallcondstoall_whataname" + aName).c_str()))
		{
			stateIndex = -99;
		}

		if (ImGui::Button((std::string(Lang::T("RM_BTN_DONE")) + "##ConditionEditor").c_str()))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	return stateIndex;
}

void CRadialContext::CreateDefaultMountRadial()
{
	unsigned int col = ImColor(255, 255, 255, 255);
	unsigned int colHov = ImColor(245, 192, 67, 255);

	CRadialMenu* mounts = this->Add(PacksDirectory / "Mounts.json", "Mounts", ERadialType::Normal);

	/* 预设条目名称走本地化：默认简体中文（国服译名），切换为英文时显示英文名。
	   名称同时作为 AddItemAction 的条目 key，故两处复用同一个变量以保证匹配。 */
	const char* nameRaptor = Lang::T("RM_MOUNT_RAPTOR");
	if (!std::filesystem::exists(IconsDirectory / "ICON_RAPTOR.png")) { Resources::Unpack(SelfModule, IconsDirectory / "ICON_RAPTOR.png", ICON_RAPTOR, "PNG", true); }
	mounts->AddItem(nameRaptor, col, colHov, EIconType::File, "addons\\RadialMenus_CN\\icons\\ICON_RAPTOR.png");
	mounts->AddItemAction(nameRaptor, EActionType::GameInputBind, EGameBinds_SpumoniMAM01);

	const char* nameSpringer = Lang::T("RM_MOUNT_SPRINGER");
	if (!std::filesystem::exists(IconsDirectory / "ICON_SPRINGER.png")) { Resources::Unpack(SelfModule, IconsDirectory / "ICON_SPRINGER.png", ICON_SPRINGER, "PNG", true); }
	mounts->AddItem(nameSpringer, col, colHov, EIconType::File, "addons\\RadialMenus_CN\\icons\\ICON_SPRINGER.png");
	mounts->AddItemAction(nameSpringer, EActionType::GameInputBind, EGameBinds_SpumoniMAM02);

	const char* nameSkimmer = Lang::T("RM_MOUNT_SKIMMER");
	if (!std::filesystem::exists(IconsDirectory / "ICON_SKIMMER.png")) { Resources::Unpack(SelfModule, IconsDirectory / "ICON_SKIMMER.png", ICON_SKIMMER, "PNG", true); }
	mounts->AddItem(nameSkimmer, col, colHov, EIconType::File, "addons\\RadialMenus_CN\\icons\\ICON_SKIMMER.png");
	mounts->AddItemAction(nameSkimmer, EActionType::GameInputBind, EGameBinds_SpumoniMAM03);

	const char* nameJackal = Lang::T("RM_MOUNT_JACKAL");
	if (!std::filesystem::exists(IconsDirectory / "ICON_JACKAL.png")) { Resources::Unpack(SelfModule, IconsDirectory / "ICON_JACKAL.png", ICON_JACKAL, "PNG", true); }
	mounts->AddItem(nameJackal, col, colHov, EIconType::File, "addons\\RadialMenus_CN\\icons\\ICON_JACKAL.png");
	mounts->AddItemAction(nameJackal, EActionType::GameInputBind, EGameBinds_SpumoniMAM04);

	const char* nameGriffon = Lang::T("RM_MOUNT_GRIFFON");
	if (!std::filesystem::exists(IconsDirectory / "ICON_GRIFFON.png")) { Resources::Unpack(SelfModule, IconsDirectory / "ICON_GRIFFON.png", ICON_GRIFFON, "PNG", true); }
	mounts->AddItem(nameGriffon, col, colHov, EIconType::File, "addons\\RadialMenus_CN\\icons\\ICON_GRIFFON.png");
	mounts->AddItemAction(nameGriffon, EActionType::GameInputBind, EGameBinds_SpumoniMAM05);

	const char* nameRollerBeetle = Lang::T("RM_MOUNT_ROLLERBEETLE");
	if (!std::filesystem::exists(IconsDirectory / "ICON_ROLLERBEETLE.png")) { Resources::Unpack(SelfModule, IconsDirectory / "ICON_ROLLERBEETLE.png", ICON_ROLLERBEETLE, "PNG", true); }
	mounts->AddItem(nameRollerBeetle, col, colHov, EIconType::File, "addons\\RadialMenus_CN\\icons\\ICON_ROLLERBEETLE.png");
	mounts->AddItemAction(nameRollerBeetle, EActionType::GameInputBind, EGameBinds_SpumoniMAM06);

	const char* nameWarclaw = Lang::T("RM_MOUNT_WARCLAW");
	if (!std::filesystem::exists(IconsDirectory / "ICON_WARCLAW.png")) { Resources::Unpack(SelfModule, IconsDirectory / "ICON_WARCLAW.png", ICON_WARCLAW, "PNG", true); }
	mounts->AddItem(nameWarclaw, col, colHov, EIconType::File, "addons\\RadialMenus_CN\\icons\\ICON_WARCLAW.png");
	mounts->AddItemAction(nameWarclaw, EActionType::GameInputBind, EGameBinds_SpumoniMAM07);

	const char* nameSkyscale = Lang::T("RM_MOUNT_SKYSCALE");
	if (!std::filesystem::exists(IconsDirectory / "ICON_SKYSCALE.png")) { Resources::Unpack(SelfModule, IconsDirectory / "ICON_SKYSCALE.png", ICON_SKYSCALE, "PNG", true); }
	mounts->AddItem(nameSkyscale, col, colHov, EIconType::File, "addons\\RadialMenus_CN\\icons\\ICON_SKYSCALE.png");
	mounts->AddItemAction(nameSkyscale, EActionType::GameInputBind, EGameBinds_SpumoniMAM08);

	const char* nameSiegeTurtle = Lang::T("RM_MOUNT_SIEGETURTLE");
	if (!std::filesystem::exists(IconsDirectory / "ICON_SIEGETURTLE.png")) { Resources::Unpack(SelfModule, IconsDirectory / "ICON_SIEGETURTLE.png", ICON_SIEGETURTLE, "PNG", true); }
	mounts->AddItem(nameSiegeTurtle, col, colHov, EIconType::File, "addons\\RadialMenus_CN\\icons\\ICON_SIEGETURTLE.png");
	mounts->AddItemAction(nameSiegeTurtle, EActionType::GameInputBind, EGameBinds_SpumoniMAM09);

	this->Save();
}

void CRadialContext::Load()
{
	const std::lock_guard<std::mutex> lock(this->Mutex);
	this->LoadInternal();
}

void CRadialContext::Save()
{
	const std::lock_guard<std::mutex> lock(this->Mutex);
	this->SaveInternal();
}

void CRadialContext::Activate(CRadialMenu* aRadial)
{
	if (!aRadial) { return; }

	if (this->ActiveRadial)
	{
		ESelectionMode mode = this->ActiveRadial->GetSelectionMode();
		/* if there's an active radial and the keybind is pressed again with click mode -> cancel */
		if (mode == ESelectionMode::Click || mode == ESelectionMode::Hover)
		{
			this->Release(ESelectionMode::Escape);
		}
	}
	else if(aRadial->Activate())
	{
		this->ActiveRadial = aRadial;
	}
}

bool CRadialContext::Release(ESelectionMode aMode)
{
	if (!this->ActiveRadial) { return false; }

	ESelectionMode targetMode = this->ActiveRadial->GetSelectionMode();

	switch (aMode)
	{
		default:
		case ESelectionMode::None: { return false; }
		case ESelectionMode::Click:
		{
			if (!(targetMode == ESelectionMode::Click || targetMode == ESelectionMode::ReleaseOrClick)) { return false; }
			this->ActiveRadial->Release();
			break;
		}
		case ESelectionMode::Release:
		{
			if (!(targetMode == ESelectionMode::Release || targetMode == ESelectionMode::ReleaseOrClick)) { return false; }
			this->ActiveRadial->Release();
			break;
		}
		case ESelectionMode::Hover:
		{
			if (targetMode != ESelectionMode::Hover) { return false; }
			this->ActiveRadial->Release();
			break;
		}
		case ESelectionMode::Escape:
		{
			this->ActiveRadial->Release(ESelectionMode::Escape);
			break;
		}
		case ESelectionMode::SingleItem:
		{
			this->ActiveRadial->Release(ESelectionMode::SingleItem);
			break;
		}
	}

	this->ActiveRadial = nullptr;

	return true;
}

void CRadialContext::Render()
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	for (CRadialMenu* radial : this->Radials)
	{
		if (radial->Render())
		{
			this->Release(ESelectionMode::Hover);
		}
	}

	this->ItemProcessor.Render();
}

void CRadialContext::RenderEditorTab()
{
	float width = ImGui::GetWindowContentRegionWidth();

	if (ImGui::BeginTabItem((std::string(Lang::T("RM_TAB_EDITOR")) + "##radialmenus").c_str()))
	{
		if (ImGui::Button(Lang::T("RM_BTN_RELOAD_RADIALS")))
		{
			this->LoadInternal();
		}
		ImGui::SameLine();

		bool showChangeInfo = HasChanges;

		if (showChangeInfo) { ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 0, 1)); }
		if (ImGui::Button(Lang::T("RM_BTN_SAVE_CHANGES")))
		{
			this->SaveInternal();
		}
		if (showChangeInfo) { ImGui::PopStyleColor(); }
		ImGui::SameLine();
		if (ImGui::Button(Lang::T("RM_BTN_OPEN_FOLDER")))
		{
			ShellExecuteA(NULL, "explore", PacksDirectory.string().c_str(), NULL, NULL, SW_SHOW);
		}
		ImGui::SameLine();
		if (ImGui::Button(Lang::T("RM_BTN_NEW_RADIAL")))
		{
			std::thread([this]() {
				OPENFILENAME ofn{};
				char buff[MAX_PATH]{};
				char initialDir[MAX_PATH]{};
				strcpy_s(initialDir, PacksDirectory.string().c_str());

				ofn.lStructSize = sizeof(OPENFILENAME);
				ofn.hwndOwner = 0;
				ofn.lpstrFile = buff;
				ofn.nMaxFile = MAX_PATH;
				ofn.lpstrFilter = "JSON\0*.json";
				ofn.Flags = OFN_PATHMUSTEXIST | OFN_CREATEPROMPT;
				ofn.lpstrInitialDir = initialDir;

				if (GetSaveFileName(&ofn))
				{
					std::filesystem::path path = ofn.lpstrFile != 0 ? ofn.lpstrFile : std::filesystem::path{};

					if (path.extension() != ".json")
					{
						path += ".json";
					}

					if (!path.empty())
					{
						if (std::filesystem::exists(path))
						{
							this->Remove(path);
						}
						this->Add(path, "New Radial " + std::to_string(this->GetLowestUnusedID()));
					}
				}
			}).detach();
		}

		if (showChangeInfo)
		{
			ImGui::TextColored(ImVec4(1, 1, 0, 1), "%s", Lang::T("RM_MSG_UNSAVED_CHANGES"));
		}

		std::lock_guard<std::mutex> uklock(UnboundKeysMutex);
		{
			std::vector<EGameBinds> ukerase;

			/* check all binds again */
			for (auto& [bind, users] : UnboundKeys)
			{
				if (APIDefs->GameBinds.IsBound(bind))
				{
					ukerase.push_back(bind);
				}
			}

			/* erase those that are now tracked */
			for (EGameBinds bind : ukerase)
			{
				UnboundKeys.erase(bind);
			}
		}

		if (UnboundKeys.size() > 0)
		{
			ImGui::TextColored(ImVec4(1, 1, 0, 1), "%s", Lang::T("RM_MSG_MISSING_BINDS"));
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::TextUnformatted(Lang::T("RM_TIP_MISSING_BINDS_1"));
				ImGui::TextUnformatted(Lang::T("RM_TIP_MISSING_BINDS_2"));
				for (auto& [bindId, users] : UnboundKeys)
				{
					ImGui::Text("%s", APIDefs->Localization.Translate(GameBindToString(bindId).c_str()));
					ImGui::SameLine();
					std::string usersStr = Lang::T("RM_TIP_USED_BY");
					for (size_t i = 0; i < users.size(); i++)
					{
						if (i == users.size() - 1)
						{
							usersStr.append(users[i]);
						}
						else
						{
							usersStr.append(users[i] + ", ");
						}
					}
					ImGui::TextDisabled("%s", usersStr.c_str());
				}
				ImGui::EndTooltip();
			}
		}

		ImGui::BeginChild("##radialeditor", ImVec2(width / 6 * 2, 0));

		ImGui::TextDisabled("%s", Lang::T("RM_LBL_SELECT_RADIAL"));
		std::string radDel;
		for (CRadialMenu* radial : this->Radials)
		{
			if (ImGui::TreeNode(radial->GetName().c_str()))
			{
				std::string popupName = "RadialCtxMenu#" + radial->GetName();
				if (ImGui::BeginPopupContextItem(popupName.c_str()))
				{
					if (ImGui::Button(Lang::T("RM_BTN_DELETE")))
					{
						radDel = radial->GetName();
					}
					ImGui::EndPopup();
				}
				ImGui::OpenPopupOnItemClick(popupName.c_str(), 1);

				if (ImGui::Selectable((std::string(Lang::T("RM_LBL_SETTINGS")) + "##" + radial->GetName()).c_str(), this->EditingMenu == radial && this->EditingItem == nullptr))
				{
					this->EditingMenu = radial;
					this->EditingItem = nullptr;
				}
				ImGui::Separator();

				int capacity = radial->GetCapacity();
				std::vector<RadialItem*> items = radial->GetItems();

				int i = 0;
				for (RadialItem* item : items)
				{
					ImGui::TextDisabled("%s", Lang::T("RM_LBL_ITEM"));
					ImGui::SameLine();
					
					float btnSz = ImGui::GetFrameHeight();
					float btnPad = ImGui::GetStyle().ItemSpacing.x;

					if (ImGui::Selectable(item->Identifier.c_str(), this->EditingItem == item, 0, ImVec2(ImGui::GetContentRegionAvailWidth() - ((btnSz * 3) + (btnPad) * 3), 0)))
					{
						this->EditingMenu = radial;
						this->EditingItem = item;
					}
					ImGui::SameLine();
					if (ImGui::ArrowButtonCondDisabled(("up_radialitem##" + std::to_string(i)).c_str(), ImGuiDir_Up, i == 0))
					{
						radial->MoveItemUp(item->Identifier);
						HasChanges = true;
					}
					ImGui::SameLine();
					if (ImGui::ArrowButtonCondDisabled(("dn_radialitem##" + std::to_string(i)).c_str(), ImGuiDir_Down, i == items.size() - 1))
					{
						radial->MoveItemDown(item->Identifier);
						HasChanges = true;
					}
					ImGui::SameLine();
					if (ImGui::CrossButton(("x_radialitem##" + std::to_string(i)).c_str()))
					{
						this->Release(ESelectionMode::Escape);
						radial->RemoveItem(item->Identifier);
						if (this->EditingItem == item)
						{
							this->EditingItem = nullptr;
						}
						HasChanges = true;
					}

					i++;
				}

				if (ImGui::Button((std::string(Lang::T("RM_BTN_ADD_NEW_ITEM")) + "##" + radial->GetName()).c_str(), ImVec2(ImGui::GetWindowContentRegionWidth() - ImGui::GetCursorPosX(), 0)))
				{
					radial->AddItem(std::string(), NORMAL_ITEM_COLOR, NORMAL_ITEM_COLOR_HOVER, EIconType::None, "");
					HasChanges = true;
				}
				if (capacity < items.size())
				{
					ImGui::TextColored(ImColor(255, 255, 0, 255), Lang::T("RM_MSG_ONLY_N_ITEMS"), capacity);
				}

				ImGui::TreePop();
			}
			else
			{
				std::string popupName = "RadialCtxMenu#" + radial->GetName();
				if (ImGui::BeginPopupContextItem(popupName.c_str()))
				{
					if (ImGui::Button(Lang::T("RM_BTN_DELETE")))
					{
						radDel = radial->GetName();
					}
					ImGui::EndPopup();
				}
				ImGui::OpenPopupOnItemClick(popupName.c_str(), 1);
			}
		}

		if (!radDel.empty())
		{
			this->Release(ESelectionMode::Escape);
			this->Remove(radDel);
			this->EditingItem = nullptr;
			this->EditingMenu = nullptr;
		}
		ImGui::EndChild();

		ImGui::SameLine();

		ImGui::BeginChild("##radialitemeditor", ImVec2(width / 6 * 4, 0));
		if (this->EditingMenu && !this->EditingItem) /* editing general menu settings */
		{
			ImGui::TextDisabled(Lang::T("RM_LBL_ID"), this->EditingMenu->GetID());
			ImGui::TextDisabled(Lang::T("RM_LBL_PATH"), this->EditingMenu->GetPath().string().c_str());
			ImGui::TextDisabled("%s", Lang::T("RM_LBL_INPUT_BIND"));
			bool modifiedBinds = false;
			if (ImGui::BeginCombo("##radialinputbind", this->EditingMenu->GetInputBind().c_str()))
			{
				for (auto& [bind, radial] : this->RadialIBMap)
				{
					if (ImGui::Selectable(bind.c_str()))
					{
						if (radial != this->EditingMenu)
						{
							int tmp = radial->GetID();
							radial->SetID(this->EditingMenu->GetID());
							this->EditingMenu->SetID(tmp);

							APIDefs->Localization.Set(radial->GetInputBind().c_str(), "en", radial->GetName().c_str());
							APIDefs->Localization.Set(this->EditingMenu->GetInputBind().c_str(), "en", this->EditingMenu->GetName().c_str());
							modifiedBinds = true;
							HasChanges = true;
						}
					}
				}
				ImGui::EndCombo();
			}
			ImGui::SameLine();
			ImGui::TextDisabled(Lang::T("RM_LBL_SHOWN_AS"), this->EditingMenu->GetName().c_str());

			if (modifiedBinds)
			{
				this->GenerateIBMap();
			}

			ImGui::TextDisabled("%s", Lang::T("RM_LBL_NAME"));
			ImGui::HelpMarker(Lang::T("RM_TIP_NAME_UNIQUE"));

			static bool editingName;
			static char inputBuff[MAX_PATH];
			if (!editingName)
			{
				if (ImGui::Button(this->EditingMenu->GetName().c_str()))
				{
					editingName = true;
					strcpy_s(inputBuff, this->EditingMenu->GetName().c_str());
				}
			}
			else
			{
				if (ImGui::InputText("##radialname", inputBuff, sizeof(inputBuff), ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue))
				{
					std::string newName = this->GetUnusedName(inputBuff, this->EditingMenu);

					this->EditingMenu->SetName(newName);
					APIDefs->Localization.Set(this->EditingMenu->GetInputBind().c_str(), "en", newName.c_str());

					editingName = false;
					HasChanges = true;
				}
			}

			ImGui::TextDisabled("%s", Lang::T("RM_LBL_TYPE"));
			std::string type;
			switch (this->EditingMenu->GetType())
			{
				case ERadialType::Small:
					type = Lang::T("RM_TYPE_SMALL");
					break;
				case ERadialType::Normal:
					type = Lang::T("RM_TYPE_NORMAL");
					break;
			}
			if (ImGui::BeginCombo("##radialtype", type.c_str()))
			{
				if (ImGui::Selectable(Lang::T("RM_TYPE_SMALL")))
				{
					this->EditingMenu->SetType(ERadialType::Small);
					this->EditingMenu->ApplyColorToAll(SMALL_ITEM_COLOR, 1);
					this->EditingMenu->ApplyColorToAll(SMALL_ITEM_COLOR_HOVER, 2);
					HasChanges = true;
				}
				if (ImGui::Selectable(Lang::T("RM_TYPE_NORMAL")))
				{
					this->EditingMenu->SetType(ERadialType::Normal);
					this->EditingMenu->ApplyColorToAll(NORMAL_ITEM_COLOR, 1);
					this->EditingMenu->ApplyColorToAll(NORMAL_ITEM_COLOR_HOVER, 2);
					HasChanges = true;
				}
				ImGui::EndCombo();
			}

			if (this->EditingMenu->GetType() == ERadialType::Normal)
			{
				ImGui::TextDisabled("%s", Lang::T("RM_LBL_INNER_RADIUS"));
				std::string radius;
				switch (this->EditingMenu->GetGetInnerRadius())
				{
					case EInnerRadius::Small:
						radius = Lang::T("RM_RADIUS_SMALL");
						break;
					case EInnerRadius::Medium:
						radius = Lang::T("RM_RADIUS_MEDIUM");
						break;
					case EInnerRadius::Big:
						radius = Lang::T("RM_RADIUS_BIG");
						break;
				}
				if (ImGui::BeginCombo("##radialradius", radius.c_str()))
				{
					if (ImGui::Selectable(Lang::T("RM_RADIUS_SMALL")))
					{
						this->EditingMenu->SetInnerRadius(EInnerRadius::Small);
						HasChanges = true;
					}
					if (ImGui::Selectable(Lang::T("RM_RADIUS_MEDIUM")))
					{
						this->EditingMenu->SetInnerRadius(EInnerRadius::Medium);
						HasChanges = true;
					}
					if (ImGui::Selectable(Lang::T("RM_RADIUS_BIG")))
					{
						this->EditingMenu->SetInnerRadius(EInnerRadius::Big);
						HasChanges = true;
					}
					ImGui::EndCombo();
				}
			}

			ImGui::TextDisabled("%s", Lang::T("RM_LBL_SCALE"));
			if (ImGui::DragFloat("##radialscale", &this->EditingMenu->Scale, 0.01f, 0.1f, 5.0f, "%.2f"))
			{
				this->EditingMenu->Invalidate();
				HasChanges = true;
			}

			ImGui::TextDisabled("%s", Lang::T("RM_LBL_ICON_SCALE"));
			if (ImGui::DragFloat("##radialiconscale", &this->EditingMenu->IconScale, 0.01f, 0.1f, 5.0f, "%.2f"))
			{
				this->EditingMenu->Invalidate();
				HasChanges = true;
			}

			ImGui::TextDisabled("%s", Lang::T("RM_LBL_SELECTION_MODE"));
			std::string selMode;
			ESelectionMode selectionMode = this->EditingMenu->GetSelectionMode();
			switch (selectionMode)
			{
				case ESelectionMode::Click:
					selMode = Lang::T("RM_SELMODE_CLICK");
					break;
				case ESelectionMode::Release:
					selMode = Lang::T("RM_SELMODE_RELEASE");
					break;
				case ESelectionMode::ReleaseOrClick:
					selMode = Lang::T("RM_SELMODE_RELEASE_CLICK");
					break;
				case ESelectionMode::Hover:
					selMode = Lang::T("RM_SELMODE_HOVER");
					break;
			}
			if (ImGui::BeginCombo("##radialselectionmode", selMode.c_str()))
			{
				if (ImGui::Selectable(Lang::T("RM_SELMODE_CLICK")))
				{
					this->EditingMenu->SetSelectionMode(ESelectionMode::Click);
					HasChanges = true;
				}
				ImGui::TooltipGeneric(Lang::T("RM_TIP_SELMODE_CLICK"));

				if (ImGui::Selectable(Lang::T("RM_SELMODE_RELEASE")))
				{
					this->EditingMenu->SetSelectionMode(ESelectionMode::Release);
					HasChanges = true;
				}
				ImGui::TooltipGeneric(Lang::T("RM_TIP_SELMODE_RELEASE"));

				if (ImGui::Selectable(Lang::T("RM_SELMODE_RELEASE_CLICK")))
				{
					this->EditingMenu->SetSelectionMode(ESelectionMode::ReleaseOrClick);
					HasChanges = true;
				}
				ImGui::TooltipGeneric(Lang::T("RM_TIP_SELMODE_RELEASE_CLICK"));

				if (ImGui::Selectable(Lang::T("RM_SELMODE_HOVER")))
				{
					this->EditingMenu->SetSelectionMode(ESelectionMode::Hover);
					HasChanges = true;
				}
				ImGui::TooltipGeneric(Lang::T("RM_TIP_SELMODE_HOVER"));

				ImGui::EndCombo();
			}

			if (selectionMode == ESelectionMode::Hover)
			{
				ImGui::TextDisabled("%s", Lang::T("RM_LBL_HOVER_TIMEOUT"));
				ImGui::SetNextItemWidth(ImGui::CalcItemWidth() / 2);
				if (ImGui::InputInt("##radialhovertimeout", &this->EditingMenu->HoverTimeout))
				{
					HasChanges = true;
				}
				ImGui::HelpMarker(Lang::T("RM_TIP_HOVER_TIMEOUT"));
			}

			ImGui::TextDisabled("%s", Lang::T("RM_LBL_CENTER_BEHAVIOR"));
			ImGui::HelpMarker(Lang::T("RM_TIP_CENTER_BEHAVIOR"));
			std::string centerbehavior;
			switch (this->EditingMenu->GetCenterBehavior())
			{
				case ECenterBehavior::None:
					centerbehavior = Lang::T("RM_CENTER_NONE");
					break;
				case ECenterBehavior::FirstItemMatchingActivation:
					centerbehavior = Lang::T("RM_CENTER_FIRST");
					break;
				case ECenterBehavior::SpecificItem:
					centerbehavior = Lang::T("RM_CENTER_SPECIFIC");
					break;
				case ECenterBehavior::LastUsed:
					centerbehavior = Lang::T("RM_CENTER_LASTUSED");
					break;
			}
			if (ImGui::BeginCombo("##radialcenterbehavior", centerbehavior.c_str()))
			{
				if (ImGui::Selectable(Lang::T("RM_CENTER_NONE")))
				{
					this->EditingMenu->SetCenterBehavior(ECenterBehavior::None);
					HasChanges = true;
				}
				if (ImGui::Selectable(Lang::T("RM_CENTER_FIRST")))
				{
					this->EditingMenu->SetCenterBehavior(ECenterBehavior::FirstItemMatchingActivation);
					HasChanges = true;
				}
				if (ImGui::Selectable(Lang::T("RM_CENTER_SPECIFIC")))
				{
					this->EditingMenu->SetCenterBehavior(ECenterBehavior::SpecificItem);
					HasChanges = true;
				}
				if (ImGui::Selectable(Lang::T("RM_CENTER_LASTUSED")))
				{
					this->EditingMenu->SetCenterBehavior(ECenterBehavior::LastUsed);
					HasChanges = true;
				}
				ImGui::EndCombo();
			}

			if (this->EditingMenu->GetCenterBehavior() == ECenterBehavior::SpecificItem)
			{
				ImGui::TextDisabled("%s", Lang::T("RM_LBL_SPECIFIC_ITEM"));
				ImGui::HelpMarker(Lang::T("RM_TIP_SPECIFIC_ITEM"));

				static bool editingSpecificItemName;
				static char inputBuff[MAX_PATH];
				if (!editingSpecificItemName)
				{
					if (ImGui::Button(this->EditingMenu->SpecificCenterItemName.empty() ? Lang::T("RM_LBL_NONE") : this->EditingMenu->SpecificCenterItemName.c_str()))
					{
						editingSpecificItemName = true;
						strcpy_s(inputBuff, this->EditingMenu->SpecificCenterItemName.c_str());
					}
				}
				else
				{
					if (ImGui::InputText("##radialcenteritemname", inputBuff, sizeof(inputBuff), ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue))
					{
						editingSpecificItemName = false;
						this->EditingMenu->SpecificCenterItemName = inputBuff;
						HasChanges = true;
					}
				}
			}

			ImGui::TextDisabled("%s", Lang::T("RM_LBL_ITEMS_ROTATION"));
			ImGui::HelpMarker(Lang::T("RM_TIP_ITEMS_ROTATION"));
			ImGui::SetNextItemWidth(ImGui::CalcItemWidth() / 2);
			if (ImGui::SliderInt("##radialitemrotation", &this->EditingMenu->ItemRotationDegrees, -180, 180))
			{
				HasChanges = true;
			}

			ImGui::Checkbox(Lang::T("RM_CHK_DRAW_IN_CENTER"), &this->EditingMenu->DrawInCenter);
			if (this->EditingMenu->DrawInCenter)
			{
				ImGui::BeginGroup();
				ImGui::Indent();
				ImGui::Checkbox(Lang::T("RM_CHK_NO_CENTER_CURSOR"), &this->EditingMenu->DoNotCenterCursor);
				ImGui::EndGroup();
			}
			ImGui::Checkbox(Lang::T("RM_CHK_RESTORE_CURSOR"), &this->EditingMenu->RestoreCursor);
			ImGui::Checkbox(Lang::T("RM_CHK_SHOW_TOOLTIPS"), &this->EditingMenu->ShowItemNameTooltip);
		}
		else if (this->EditingItem) /* editing sub item */
		{
			ImGui::TextDisabled("%s", Lang::T("RM_LBL_NAME"));
			ImGui::HelpMarker(Lang::T("RM_TIP_NAME_UNIQUE"));

			static bool editingName;
			static char inputBuff[MAX_PATH];
			if (!editingName)
			{
				if (ImGui::Button(this->EditingItem->Identifier.c_str()))
				{
					editingName = true;
					strcpy_s(inputBuff, this->EditingItem->Identifier.c_str());
				}
			}
			else
			{
				if (ImGui::InputText("##radialitemname", inputBuff, sizeof(inputBuff), ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue))
				{
					editingName = false;
					this->EditingItem->Identifier = inputBuff;
					HasChanges = true;
				}
			}

			ImGui::TextDisabled("%s", Lang::T("RM_LBL_PRIORITY"));
			ImGui::HelpMarker(Lang::T("RM_TIP_PRIORITY"));
			ImGui::SetNextItemWidth(ImGui::CalcItemWidth() / 2);
			if (ImGui::InputInt("##priority", &this->EditingItem->Priority))
			{
				HasChanges = true;
			}

			if (ImGui::BeginTable("##radialcolours", 2, ImGuiTableFlags_SizingFixedFit))
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::TextDisabled("%s", Lang::T("RM_LBL_COLOR"));
				if (ImGui::ColorEdit4U32("Color", &this->EditingItem->Color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel))
				{
					HasChanges = true;
				}
				ImGui::SameLine();
				if (ImGui::Button((std::string(Lang::T("RM_BTN_APPLY_TO_ALL")) + "##color").c_str()))
				{
					this->EditingMenu->ApplyColorToAll(this->EditingItem->Color, 1);
					HasChanges = true;
				}
				ImGui::TooltipGeneric(Lang::T("RM_TIP_APPLY_COLOR_ALL"));

				ImGui::TableSetColumnIndex(1);
				ImGui::TextDisabled("%s", Lang::T("RM_LBL_COLOR_HOVER"));
				if (ImGui::ColorEdit4U32("Color Hover", &this->EditingItem->ColorHover, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel))
				{
					HasChanges = true;
				}
				ImGui::SameLine();
				if (ImGui::Button((std::string(Lang::T("RM_BTN_APPLY_TO_ALL")) + "##colorhover").c_str()))
				{
					this->EditingMenu->ApplyColorToAll(this->EditingItem->ColorHover, 2);
					HasChanges = true;
				}
				ImGui::TooltipGeneric(Lang::T("RM_TIP_APPLY_COLORHOVER_ALL"));

				ImGui::EndTable();
			}

			ImGui::TextDisabled("%s", Lang::T("RM_LBL_ICON"));

			std::string iconType;
			switch (this->EditingItem->Icon.Type)
			{
				case EIconType::File:
					iconType = Lang::T("RM_ICON_FILE");
					break;
				case EIconType::URL:
					iconType = Lang::T("RM_ICON_URL");
					break;
			}
			if (ImGui::BeginCombo("##radialitemicontype", iconType.c_str()))
			{
				if (ImGui::Selectable(Lang::T("RM_ICON_FILE")))
				{
					this->EditingItem->Icon.Type = EIconType::File;
					HasChanges = true;
				}
				if (ImGui::Selectable(Lang::T("RM_ICON_URL")))
				{
					this->EditingItem->Icon.Type = EIconType::URL;
					HasChanges = true;
				}
				ImGui::EndCombo();
			}

			if (this->EditingItem->Icon.Type == EIconType::File)
			{
				if (ImGui::Button(this->EditingItem->Icon.Value.c_str(), ImVec2(ImGui::CalcItemWidth(), 0)))
				{
					std::thread([this]() {
						OPENFILENAME ofn{};
						char buff[MAX_PATH]{};
						char initialDir[MAX_PATH]{};
						strcpy_s(initialDir, IconsDirectory.string().c_str());

						ofn.lStructSize = sizeof(OPENFILENAME);
						ofn.hwndOwner = 0;
						ofn.lpstrFile = buff;
						ofn.nMaxFile = MAX_PATH;
						//ofn.lpstrFilter = "Guild Wars 2 Executable\0*.exe";
						ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
						ofn.lpstrInitialDir = initialDir;

						if (GetOpenFileName(&ofn))
						{
							this->EditingItem->Icon.Value = ofn.lpstrFile != 0 ? ofn.lpstrFile : "";
							if (!this->EditingItem->Icon.Value.empty())
							{
								this->EditingItem->Icon.Value = String::Replace(this->EditingItem->Icon.Value, GW2Root.string() + "\\", "");
								this->EditingItem->Icon.Texture = nullptr;
								std::filesystem::path iconPath = this->EditingItem->Icon.Value;
								if (iconPath.is_relative())
								{
									APIDefs->Textures.LoadFromFile(this->EditingItem->Icon.Value.c_str(), (GW2Root / this->EditingItem->Icon.Value).string().c_str(), nullptr);
								}
								else
								{
									APIDefs->Textures.LoadFromFile(this->EditingItem->Icon.Value.c_str(), this->EditingItem->Icon.Value.c_str(), nullptr);
								}
								HasChanges = true;
							}
						}
					}).detach();
				}
			}
			else if (this->EditingItem->Icon.Type == EIconType::URL)
			{
				static bool editingUrl;
				static char inputBuff[MAX_PATH];
				if (!editingUrl)
				{
					if (ImGui::Button(this->EditingItem->Icon.Value.c_str(), ImVec2(ImGui::CalcItemWidth(), 0)))
					{
						editingUrl = true;
						strcpy_s(inputBuff, this->EditingItem->Icon.Value.c_str());
					}
				}
				else
				{
					if (ImGui::InputText("##radialitemiconurl", inputBuff, sizeof(inputBuff), ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue))
					{
						editingUrl = false;
						this->EditingItem->Icon.Value = inputBuff;
						if (!this->EditingItem->Icon.Value.empty())
						{
							this->EditingItem->Icon.Texture = nullptr;
							APIDefs->Textures.LoadFromURL(this->EditingItem->Icon.Value.c_str(), URL::GetBase(this->EditingItem->Icon.Value).c_str(), URL::GetEndpoint(this->EditingItem->Icon.Value).c_str(), nullptr);
						}
						HasChanges = true;
					}
				}
			}

			ImGui::TextDisabled("%s", Lang::T("RM_LBL_VISIBILITY"));
			ImGui::HelpMarker(Lang::T("RM_TIP_VISIBILITY"));
			int applyVisToAll = ConditionEditor(std::string(Lang::T("RM_BTN_EDIT_CONDITIONS")) + "##visibility", &this->EditingItem->Visibility, Lang::T("RM_TIP_APPLYALL_VISIBILITY"));
			if (applyVisToAll > -1 || applyVisToAll == -99)
			{
				this->EditingMenu->ApplyConditionToAll(&this->EditingItem->Visibility, 1, applyVisToAll);
				HasChanges = true;
			}
			ImGui::TooltipGeneric(ConditionsToString(&this->EditingItem->Visibility).c_str());

			ImGui::TextDisabled("%s", Lang::T("RM_LBL_ACTIVATION"));
			ImGui::HelpMarker(Lang::T("RM_TIP_ACTIVATION"));
			int applyActToAll = ConditionEditor(std::string(Lang::T("RM_BTN_EDIT_CONDITIONS")) + "##activation", &this->EditingItem->Activation, Lang::T("RM_TIP_APPLYALL_ACTIVATION"));
			if (applyActToAll > -1 || applyActToAll == -99)
			{
				this->EditingMenu->ApplyConditionToAll(&this->EditingItem->Activation, 2, applyActToAll);
				HasChanges = true;
			}
			ImGui::TooltipGeneric(ConditionsToString(&this->EditingItem->Activation).c_str());

			ImGui::TextDisabled("%s", Lang::T("RM_LBL_TIMEOUT_SECONDS"));
			ImGui::HelpMarker(Lang::T("RM_TIP_TIMEOUT"));
			ImGui::SetNextItemWidth(ImGui::CalcItemWidth() / 2);
			if (ImGui::InputInt("##activationtimeout", &this->EditingItem->ActivationTimeout))
			{
				HasChanges = true;
			}
			ImGui::SameLine();
			if (ImGui::Button((std::string(Lang::T("RM_BTN_APPLY_TO_ALL")) + "##applyallactivationtimeout").c_str()))
			{
				this->EditingMenu->ApplyActivationTimeoutToAll(this->EditingItem->ActivationTimeout);
				HasChanges = true;
			}

			ImGui::TextDisabled("%s", Lang::T("RM_LBL_ACTIONS"));
			ImGui::HelpMarker(Lang::T("RM_TIP_ACTIONS"));

			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
			ImGui::TextWrapped("%s", Lang::T("RM_MSG_MACRO_POLICY"));
			if (ImGui::TextURL(Lang::T("RM_LBL_MACRO_POLICY_LINK"), false, false))
			{
				ShellExecuteA(0, 0, "https://help.guildwars2.com/hc/en-us/articles/360013762153-Policy-Macros-and-Macro-Use", 0, 0, SW_SHOW);
			}
			ImGui::PopStyleColor();

			int idxDel = -1;
			if (ImGui::BeginTable("##radialitemactions", 4))
			{
				int i = 0;
				bool previousLinked = true;
				for (ActionBase* action : this->EditingItem->Actions)
				{
					if (!action->OnlyExecuteIfPrevious && previousLinked)
					{
						ImGui::Separator();
					}
					previousLinked = action->OnlyExecuteIfPrevious;
					std::string actionType;
					switch (action->Type)
					{
						case EActionType::InputBind: actionType = Lang::T("RM_ACTION_INPUTBIND_NEXUS"); break;
						case EActionType::GameInputBind: actionType = Lang::T("RM_ACTION_INPUTBIND_GAME"); break;
						case EActionType::GameInputBindPress: actionType = Lang::T("RM_ACTION_INPUTBIND_PRESS_GAME"); break;
						case EActionType::GameInputBindRelease: actionType = Lang::T("RM_ACTION_INPUTBIND_RELEASE_GAME"); break;
						case EActionType::Event: actionType = Lang::T("RM_ACTION_EVENT"); break;
						case EActionType::Delay: actionType = Lang::T("RM_ACTION_DELAY"); break;
						case EActionType::Return: actionType = Lang::T("RM_ACTION_RETURN"); break;
					}
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth());
					if (ImGui::BeginCombo(("##radialitemactiontype" + std::to_string(i)).c_str(), actionType.c_str()))
					{
						if (ImGui::Selectable(Lang::T("RM_ACTION_INPUTBIND_NEXUS")))
						{
							if (action->Type != EActionType::InputBind)
							{
								delete this->EditingItem->Actions[i];
								this->EditingItem->Actions[i] = new ActionGeneric();
								this->EditingItem->Actions[i]->Type = EActionType::InputBind;
								HasChanges = true;
							}
						}
						if (ImGui::Selectable(Lang::T("RM_ACTION_INPUTBIND_GAME")))
						{
							if (action->Type != EActionType::GameInputBind)
							{
								EGameBinds persistBind = (EGameBinds)0;

								if (this->EditingItem->Actions[i]->Type == EActionType::GameInputBind ||
									this->EditingItem->Actions[i]->Type == EActionType::GameInputBindPress ||
									this->EditingItem->Actions[i]->Type == EActionType::GameInputBindRelease)
								{
									persistBind = ((ActionGameInputBind*)this->EditingItem->Actions[i])->Identifier;
								}

								delete this->EditingItem->Actions[i];
								this->EditingItem->Actions[i] = new ActionGameInputBind();
								this->EditingItem->Actions[i]->Type = EActionType::GameInputBind;
								((ActionGameInputBind*)this->EditingItem->Actions[i])->Identifier = persistBind;
								HasChanges = true;
							}
						}
						if (ImGui::Selectable(Lang::T("RM_ACTION_INPUTBIND_PRESS_GAME")))
						{
							if (action->Type != EActionType::GameInputBindPress)
							{
								EGameBinds persistBind = (EGameBinds)0;

								if (this->EditingItem->Actions[i]->Type == EActionType::GameInputBind ||
									this->EditingItem->Actions[i]->Type == EActionType::GameInputBindPress ||
									this->EditingItem->Actions[i]->Type == EActionType::GameInputBindRelease)
								{
									persistBind = ((ActionGameInputBind*)this->EditingItem->Actions[i])->Identifier;
								}

								delete this->EditingItem->Actions[i];
								this->EditingItem->Actions[i] = new ActionGameInputBind();
								this->EditingItem->Actions[i]->Type = EActionType::GameInputBindPress;
								((ActionGameInputBind*)this->EditingItem->Actions[i])->Identifier = persistBind;
								HasChanges = true;
							}
						}
						if (ImGui::Selectable(Lang::T("RM_ACTION_INPUTBIND_RELEASE_GAME")))
						{
							if (action->Type != EActionType::GameInputBindRelease)
							{
								EGameBinds persistBind = (EGameBinds)0;

								if (this->EditingItem->Actions[i]->Type == EActionType::GameInputBind ||
									this->EditingItem->Actions[i]->Type == EActionType::GameInputBindPress ||
									this->EditingItem->Actions[i]->Type == EActionType::GameInputBindRelease)
								{
									persistBind = ((ActionGameInputBind*)this->EditingItem->Actions[i])->Identifier;
								}

								delete this->EditingItem->Actions[i];
								this->EditingItem->Actions[i] = new ActionGameInputBind();
								this->EditingItem->Actions[i]->Type = EActionType::GameInputBindRelease;
								((ActionGameInputBind*)this->EditingItem->Actions[i])->Identifier = persistBind;
								HasChanges = true;
							}
						}
						if (ImGui::Selectable(Lang::T("RM_ACTION_EVENT")))
						{
							if (action->Type != EActionType::Event)
							{
								delete this->EditingItem->Actions[i];
								this->EditingItem->Actions[i] = new ActionGeneric();
								this->EditingItem->Actions[i]->Type = EActionType::Event;
								HasChanges = true;
							}
						}
						if (ImGui::Selectable(Lang::T("RM_ACTION_DELAY")))
						{
							if (action->Type != EActionType::Delay)
							{
								delete this->EditingItem->Actions[i];
								this->EditingItem->Actions[i] = new ActionDelay();
								this->EditingItem->Actions[i]->Type = EActionType::Delay;
								HasChanges = true;
							}
						}
						if (ImGui::Selectable(Lang::T("RM_ACTION_RETURN")))
						{
							if (action->Type != EActionType::Return)
							{
								delete this->EditingItem->Actions[i];
								this->EditingItem->Actions[i] = new ActionBase();
								this->EditingItem->Actions[i]->Type = EActionType::Return;
								HasChanges = true;
							}
						}
						ImGui::TooltipGeneric(Lang::T("RM_TIP_ACTION_RETURN"));
						ImGui::EndCombo();
					}

					ImGui::TableSetColumnIndex(1);
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth());
					switch (action->Type)
					{
						case EActionType::InputBind:
						case EActionType::Event:
						{
							static char inputBuff[MAX_PATH];
							strcpy_s(inputBuff, ((ActionGeneric*)action)->Identifier.c_str());

							if (ImGui::InputText(("##actionidentifier" + std::to_string(i)).c_str(), inputBuff, sizeof(inputBuff), ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue))
							{
								((ActionGeneric*)action)->Identifier = _strdup(inputBuff);
								HasChanges = true;
							}
							break;
						}
						case EActionType::GameInputBind:
						case EActionType::GameInputBindPress:
						case EActionType::GameInputBindRelease:
						{
							if (ImGui::BeginCombo(("##actiongamebind" + std::to_string(i)).c_str(), APIDefs->Localization.Translate(GameBindToString(((ActionGameInputBind*)action)->Identifier).c_str())))
							{
								if (ImGui::BeginMenu(APIDefs->Localization.Translate("((Movement))")))
								{
									GameBindSelectable(action, "((MoveForward))", EGameBinds_MoveForward);
									GameBindSelectable(action, "((MoveBackward))", EGameBinds_MoveBackward);
									GameBindSelectable(action, "((MoveLeft))", EGameBinds_MoveLeft);
									GameBindSelectable(action, "((MoveRight))", EGameBinds_MoveRight);
									GameBindSelectable(action, "((MoveTurnLeft))", EGameBinds_MoveTurnLeft);
									GameBindSelectable(action, "((MoveTurnRight))", EGameBinds_MoveTurnRight);
									GameBindSelectable(action, "((MoveDodge))", EGameBinds_MoveDodge);
									GameBindSelectable(action, "((MoveAutoRun))", EGameBinds_MoveAutoRun);
									GameBindSelectable(action, "((MoveWalk))", EGameBinds_MoveWalk);
									GameBindSelectable(action, "((MoveJump))", EGameBinds_MoveJump_SwimUp_FlyUp);
									GameBindSelectable(action, "((MoveSwimDown))", EGameBinds_MoveSwimDown_FlyDown);
									GameBindSelectable(action, "((MoveAboutFace))", EGameBinds_MoveAboutFace);
									ImGui::EndMenu();
								}
								if (ImGui::BeginMenu(APIDefs->Localization.Translate("((Skills))")))
								{
									GameBindSelectable(action, "((SkillWeaponSwap))", EGameBinds_SkillWeaponSwap);
									GameBindSelectable(action, "((SkillWeapon1))", EGameBinds_SkillWeapon1);
									GameBindSelectable(action, "((SkillWeapon2))", EGameBinds_SkillWeapon2);
									GameBindSelectable(action, "((SkillWeapon3))", EGameBinds_SkillWeapon3);
									GameBindSelectable(action, "((SkillWeapon4))", EGameBinds_SkillWeapon4);
									GameBindSelectable(action, "((SkillWeapon5))", EGameBinds_SkillWeapon5);
									GameBindSelectable(action, "((SkillHeal))", EGameBinds_SkillHeal);
									GameBindSelectable(action, "((SkillUtility1))", EGameBinds_SkillUtility1);
									GameBindSelectable(action, "((SkillUtility2))", EGameBinds_SkillUtility2);
									GameBindSelectable(action, "((SkillUtility3))", EGameBinds_SkillUtility3);
									GameBindSelectable(action, "((SkillElite))", EGameBinds_SkillElite);
									GameBindSelectable(action, "((SkillProfession1))", EGameBinds_SkillProfession1);
									GameBindSelectable(action, "((SkillProfession2))", EGameBinds_SkillProfession2);
									GameBindSelectable(action, "((SkillProfession3))", EGameBinds_SkillProfession3);
									GameBindSelectable(action, "((SkillProfession4))", EGameBinds_SkillProfession4);
									GameBindSelectable(action, "((SkillProfession5))", EGameBinds_SkillProfession5);
									GameBindSelectable(action, "((SkillProfession6))", EGameBinds_SkillProfession6);
									GameBindSelectable(action, "((SkillProfession7))", EGameBinds_SkillProfession7);
									GameBindSelectable(action, "((SkillSpecialAction))", EGameBinds_SkillSpecialAction);
									ImGui::EndMenu();
								}
								if (ImGui::BeginMenu(APIDefs->Localization.Translate("((Targeting))")))
								{
									GameBindSelectable(action, "((TargetAlert))", EGameBinds_TargetAlert);
									GameBindSelectable(action, "((TargetCall))", EGameBinds_TargetCall);
									GameBindSelectable(action, "((TargetTake))", EGameBinds_TargetTake);
									GameBindSelectable(action, "((TargetCallLocal))", EGameBinds_TargetCallLocal);
									GameBindSelectable(action, "((TargetTakeLocal))", EGameBinds_TargetTakeLocal);
									GameBindSelectable(action, "((TargetEnemyNearest))", EGameBinds_TargetEnemyNearest);
									GameBindSelectable(action, "((TargetEnemyNext))", EGameBinds_TargetEnemyNext);
									GameBindSelectable(action, "((TargetEnemyPrev))", EGameBinds_TargetEnemyPrev);
									GameBindSelectable(action, "((TargetAllyNearest))", EGameBinds_TargetAllyNearest);
									GameBindSelectable(action, "((TargetAllyNext))", EGameBinds_TargetAllyNext);
									GameBindSelectable(action, "((TargetAllyPrev))", EGameBinds_TargetAllyPrev);
									GameBindSelectable(action, "((TargetLock))", EGameBinds_TargetLock);
									GameBindSelectable(action, "((TargetSnapGroundTarget))", EGameBinds_TargetSnapGroundTarget);
									GameBindSelectable(action, "((TargetSnapGroundTargetToggle))", EGameBinds_TargetSnapGroundTargetToggle);
									GameBindSelectable(action, "((TargetAutoTargetingDisable))", EGameBinds_TargetAutoTargetingDisable);
									GameBindSelectable(action, "((TargetAutoTargetingToggle))", EGameBinds_TargetAutoTargetingToggle);
									GameBindSelectable(action, "((TargetAllyTargetingMode))", EGameBinds_TargetAllyTargetingMode);
									GameBindSelectable(action, "((TargetAllyTargetingModeToggle))", EGameBinds_TargetAllyTargetingModeToggle);
									ImGui::EndMenu();
								}
								if (ImGui::BeginMenu(APIDefs->Localization.Translate("((User Interface))")))
								{
									GameBindSelectable(action, "((UiCommerce))", EGameBinds_UiCommerce);
									GameBindSelectable(action, "((UiContacts))", EGameBinds_UiContacts);
									GameBindSelectable(action, "((UiGuild))", EGameBinds_UiGuild);
									GameBindSelectable(action, "((UiHero))", EGameBinds_UiHero);
									GameBindSelectable(action, "((UiInventory))", EGameBinds_UiInventory);
									GameBindSelectable(action, "((UiKennel))", EGameBinds_UiKennel);
									GameBindSelectable(action, "((UiLogout))", EGameBinds_UiLogout);
									GameBindSelectable(action, "((UiMail))", EGameBinds_UiMail);
									GameBindSelectable(action, "((UiOptions))", EGameBinds_UiOptions);
									GameBindSelectable(action, "((UiParty))", EGameBinds_UiParty);
									GameBindSelectable(action, "((UiPvp))", EGameBinds_UiPvp);
									GameBindSelectable(action, "((UiPvpBuild))", EGameBinds_UiPvpBuild);
									GameBindSelectable(action, "((UiScoreboard))", EGameBinds_UiScoreboard);
									GameBindSelectable(action, "((UiSeasonalObjectivesShop))", EGameBinds_UiSeasonalObjectivesShop);
									GameBindSelectable(action, "((UiInformation))", EGameBinds_UiInformation);
									GameBindSelectable(action, "((UiChatToggle))", EGameBinds_UiChatToggle);
									GameBindSelectable(action, "((UiChatCommand))", EGameBinds_UiChatCommand);
									GameBindSelectable(action, "((UiChatFocus))", EGameBinds_UiChatFocus);
									GameBindSelectable(action, "((UiChatReply))", EGameBinds_UiChatReply);
									GameBindSelectable(action, "((UiToggle))", EGameBinds_UiToggle);
									GameBindSelectable(action, "((UiSquadBroadcastChatToggle))", EGameBinds_UiSquadBroadcastChatToggle);
									GameBindSelectable(action, "((UiSquadBroadcastChatCommand))", EGameBinds_UiSquadBroadcastChatCommand);
									GameBindSelectable(action, "((UiSquadBroadcastChatFocus))", EGameBinds_UiSquadBroadcastChatFocus);
									ImGui::EndMenu();
								}
								if (ImGui::BeginMenu(APIDefs->Localization.Translate("((Camera))")))
								{
									GameBindSelectable(action, "((CameraFree))", EGameBinds_CameraFree);
									GameBindSelectable(action, "((CameraZoomIn))", EGameBinds_CameraZoomIn);
									GameBindSelectable(action, "((CameraZoomOut))", EGameBinds_CameraZoomOut);
									GameBindSelectable(action, "((CameraReverse))", EGameBinds_CameraReverse);
									GameBindSelectable(action, "((CameraActionMode))", EGameBinds_CameraActionMode);
									GameBindSelectable(action, "((CameraActionModeDisable))", EGameBinds_CameraActionModeDisable);
									ImGui::EndMenu();
								}
								if (ImGui::BeginMenu(APIDefs->Localization.Translate("((Screenshot))")))
								{
									GameBindSelectable(action, "((ScreenshotNormal))", EGameBinds_ScreenshotNormal);
									GameBindSelectable(action, "((ScreenshotStereoscopic))", EGameBinds_ScreenshotStereoscopic);
									ImGui::EndMenu();
								}
								if (ImGui::BeginMenu(APIDefs->Localization.Translate("((Map))")))
								{
									GameBindSelectable(action, "((MapToggle))", EGameBinds_MapToggle);
									GameBindSelectable(action, "((MapFocusPlayer))", EGameBinds_MapFocusPlayer);
									GameBindSelectable(action, "((MapFloorDown))", EGameBinds_MapFloorDown);
									GameBindSelectable(action, "((MapFloorUp))", EGameBinds_MapFloorUp);
									GameBindSelectable(action, "((MapZoomIn))", EGameBinds_MapZoomIn);
									GameBindSelectable(action, "((MapZoomOut))", EGameBinds_MapZoomOut);
									ImGui::EndMenu();
								}
								if (ImGui::BeginMenu(APIDefs->Localization.Translate("((Mounts))")))
								{
									GameBindSelectable(action, "((SpumoniToggle))", EGameBinds_SpumoniToggle);
									GameBindSelectable(action, "((SpumoniMovement))", EGameBinds_SpumoniMovement);
									GameBindSelectable(action, "((SpumoniSecondaryMovement))", EGameBinds_SpumoniSecondaryMovement);
									GameBindSelectable(action, "((SpumoniMAM01))", EGameBinds_SpumoniMAM01);
									GameBindSelectable(action, "((SpumoniMAM02))", EGameBinds_SpumoniMAM02);
									GameBindSelectable(action, "((SpumoniMAM03))", EGameBinds_SpumoniMAM03);
									GameBindSelectable(action, "((SpumoniMAM04))", EGameBinds_SpumoniMAM04);
									GameBindSelectable(action, "((SpumoniMAM05))", EGameBinds_SpumoniMAM05);
									GameBindSelectable(action, "((SpumoniMAM06))", EGameBinds_SpumoniMAM06);
									GameBindSelectable(action, "((SpumoniMAM07))", EGameBinds_SpumoniMAM07);
									GameBindSelectable(action, "((SpumoniMAM08))", EGameBinds_SpumoniMAM08);
									GameBindSelectable(action, "((SpumoniMAM09))", EGameBinds_SpumoniMAM09);
									ImGui::EndMenu();
								}
								if (ImGui::BeginMenu(APIDefs->Localization.Translate("((Spectators))")))
								{
									GameBindSelectable(action, "((SpectatorNearestFixed))", EGameBinds_SpectatorNearestFixed);
									GameBindSelectable(action, "((SpectatorNearestPlayer))", EGameBinds_SpectatorNearestPlayer);
									GameBindSelectable(action, "((SpectatorPlayerRed1))", EGameBinds_SpectatorPlayerRed1);
									GameBindSelectable(action, "((SpectatorPlayerRed2))", EGameBinds_SpectatorPlayerRed2);
									GameBindSelectable(action, "((SpectatorPlayerRed3))", EGameBinds_SpectatorPlayerRed3);
									GameBindSelectable(action, "((SpectatorPlayerRed4))", EGameBinds_SpectatorPlayerRed4);
									GameBindSelectable(action, "((SpectatorPlayerRed5))", EGameBinds_SpectatorPlayerRed5);
									GameBindSelectable(action, "((SpectatorPlayerBlue1))", EGameBinds_SpectatorPlayerBlue1);
									GameBindSelectable(action, "((SpectatorPlayerBlue2))", EGameBinds_SpectatorPlayerBlue2);
									GameBindSelectable(action, "((SpectatorPlayerBlue3))", EGameBinds_SpectatorPlayerBlue3);
									GameBindSelectable(action, "((SpectatorPlayerBlue4))", EGameBinds_SpectatorPlayerBlue4);
									GameBindSelectable(action, "((SpectatorPlayerBlue5))", EGameBinds_SpectatorPlayerBlue5);
									GameBindSelectable(action, "((SpectatorFreeCamera))", EGameBinds_SpectatorFreeCamera);
									GameBindSelectable(action, "((SpectatorFreeCameraMode))", EGameBinds_SpectatorFreeCameraMode);
									GameBindSelectable(action, "((SpectatorFreeMoveForward))", EGameBinds_SpectatorFreeMoveForward);
									GameBindSelectable(action, "((SpectatorFreeMoveBackward))", EGameBinds_SpectatorFreeMoveBackward);
									GameBindSelectable(action, "((SpectatorFreeMoveLeft))", EGameBinds_SpectatorFreeMoveLeft);
									GameBindSelectable(action, "((SpectatorFreeMoveRight))", EGameBinds_SpectatorFreeMoveRight);
									GameBindSelectable(action, "((SpectatorFreeMoveUp))", EGameBinds_SpectatorFreeMoveUp);
									GameBindSelectable(action, "((SpectatorFreeMoveDown))", EGameBinds_SpectatorFreeMoveDown);
									ImGui::EndMenu();
								}
								if (ImGui::BeginMenu(APIDefs->Localization.Translate("((Squad))")))
								{
									GameBindSelectable(action, "((SquadMarkerPlaceWorld1))", EGameBinds_SquadMarkerPlaceWorld1);
									GameBindSelectable(action, "((SquadMarkerPlaceWorld2))", EGameBinds_SquadMarkerPlaceWorld2);
									GameBindSelectable(action, "((SquadMarkerPlaceWorld3))", EGameBinds_SquadMarkerPlaceWorld3);
									GameBindSelectable(action, "((SquadMarkerPlaceWorld4))", EGameBinds_SquadMarkerPlaceWorld4);
									GameBindSelectable(action, "((SquadMarkerPlaceWorld5))", EGameBinds_SquadMarkerPlaceWorld5);
									GameBindSelectable(action, "((SquadMarkerPlaceWorld6))", EGameBinds_SquadMarkerPlaceWorld6);
									GameBindSelectable(action, "((SquadMarkerPlaceWorld7))", EGameBinds_SquadMarkerPlaceWorld7);
									GameBindSelectable(action, "((SquadMarkerPlaceWorld8))", EGameBinds_SquadMarkerPlaceWorld8);
									GameBindSelectable(action, "((SquadMarkerClearAllWorld))", EGameBinds_SquadMarkerClearAllWorld);
									GameBindSelectable(action, "((SquadMarkerSetAgent1))", EGameBinds_SquadMarkerSetAgent1);
									GameBindSelectable(action, "((SquadMarkerSetAgent2))", EGameBinds_SquadMarkerSetAgent2);
									GameBindSelectable(action, "((SquadMarkerSetAgent3))", EGameBinds_SquadMarkerSetAgent3);
									GameBindSelectable(action, "((SquadMarkerSetAgent4))", EGameBinds_SquadMarkerSetAgent4);
									GameBindSelectable(action, "((SquadMarkerSetAgent5))", EGameBinds_SquadMarkerSetAgent5);
									GameBindSelectable(action, "((SquadMarkerSetAgent6))", EGameBinds_SquadMarkerSetAgent6);
									GameBindSelectable(action, "((SquadMarkerSetAgent7))", EGameBinds_SquadMarkerSetAgent7);
									GameBindSelectable(action, "((SquadMarkerSetAgent8))", EGameBinds_SquadMarkerSetAgent8);
									GameBindSelectable(action, "((SquadMarkerClearAllAgent))", EGameBinds_SquadMarkerClearAllAgent);
									ImGui::EndMenu();
								}
								if (ImGui::BeginMenu(APIDefs->Localization.Translate("((Mastery Skills))")))
								{
									GameBindSelectable(action, "((MasteryAccess))", EGameBinds_MasteryAccess);
									GameBindSelectable(action, "((MasteryAccess01))", EGameBinds_MasteryAccess01);
									GameBindSelectable(action, "((MasteryAccess02))", EGameBinds_MasteryAccess02);
									GameBindSelectable(action, "((MasteryAccess03))", EGameBinds_MasteryAccess03);
									GameBindSelectable(action, "((MasteryAccess04))", EGameBinds_MasteryAccess04);
									GameBindSelectable(action, "((MasteryAccess05))", EGameBinds_MasteryAccess05);
									GameBindSelectable(action, "((MasteryAccess06))", EGameBinds_MasteryAccess06);
									ImGui::EndMenu();
								}
								if (ImGui::BeginMenu(APIDefs->Localization.Translate("((Miscellaneous))")))
								{
									GameBindSelectable(action, "((MiscAoELoot))", EGameBinds_MiscAoELoot);
									GameBindSelectable(action, "((MiscInteract))", EGameBinds_MiscInteract);
									GameBindSelectable(action, "((MiscShowEnemies))", EGameBinds_MiscShowEnemies);
									GameBindSelectable(action, "((MiscShowAllies))", EGameBinds_MiscShowAllies);
									GameBindSelectable(action, "((MiscCombatStance))", EGameBinds_MiscCombatStance);
									GameBindSelectable(action, "((MiscToggleLanguage))", EGameBinds_MiscToggleLanguage);
									GameBindSelectable(action, "((MiscTogglePetCombat))", EGameBinds_MiscTogglePetCombat);
									GameBindSelectable(action, "((MiscToggleFullScreen))", EGameBinds_MiscToggleFullScreen);
									GameBindSelectable(action, "((MiscToggleDecorationMode))", EGameBinds_MiscToggleDecorationMode);
									GameBindSelectable(action, "((ToyUseDefault))", EGameBinds_ToyUseDefault);
									GameBindSelectable(action, "((ToyUseSlot1))", EGameBinds_ToyUseSlot1);
									GameBindSelectable(action, "((ToyUseSlot2))", EGameBinds_ToyUseSlot2);
									GameBindSelectable(action, "((ToyUseSlot3))", EGameBinds_ToyUseSlot3);
									GameBindSelectable(action, "((ToyUseSlot4))", EGameBinds_ToyUseSlot4);
									GameBindSelectable(action, "((ToyUseSlot5))", EGameBinds_ToyUseSlot5);
									ImGui::EndMenu();
								}
								if (ImGui::BeginMenu(APIDefs->Localization.Translate("((Templates))")))
								{
									GameBindSelectable(action, "((Loadout1))", EGameBinds_Loadout1);
									GameBindSelectable(action, "((Loadout2))", EGameBinds_Loadout2);
									GameBindSelectable(action, "((Loadout3))", EGameBinds_Loadout3);
									GameBindSelectable(action, "((Loadout4))", EGameBinds_Loadout4);
									GameBindSelectable(action, "((Loadout5))", EGameBinds_Loadout5);
									GameBindSelectable(action, "((Loadout6))", EGameBinds_Loadout6);
									GameBindSelectable(action, "((Loadout7))", EGameBinds_Loadout7);
									GameBindSelectable(action, "((Loadout8))", EGameBinds_Loadout8);
									GameBindSelectable(action, "((Loadout9))", EGameBinds_Loadout9);
									GameBindSelectable(action, "((GearLoadout1))", EGameBinds_GearLoadout1);
									GameBindSelectable(action, "((GearLoadout2))", EGameBinds_GearLoadout2);
									GameBindSelectable(action, "((GearLoadout3))", EGameBinds_GearLoadout3);
									GameBindSelectable(action, "((GearLoadout4))", EGameBinds_GearLoadout4);
									GameBindSelectable(action, "((GearLoadout5))", EGameBinds_GearLoadout5);
									GameBindSelectable(action, "((GearLoadout6))", EGameBinds_GearLoadout6);
									GameBindSelectable(action, "((GearLoadout7))", EGameBinds_GearLoadout7);
									GameBindSelectable(action, "((GearLoadout8))", EGameBinds_GearLoadout8);
									GameBindSelectable(action, "((GearLoadout9))", EGameBinds_GearLoadout9);
									ImGui::EndMenu();
								}
								ImGui::EndCombo();
							}

							break;
						}
						case EActionType::Delay:
						{
							if (ImGui::InputInt(("##delay" + std::to_string(i)).c_str(), &((ActionDelay*)action)->Duration, 1, 100))
							{
								HasChanges = true;
							}
							break;
						}
						case EActionType::Return:
						{
							/* there's no parameter */
							break;
						}
					}

					ImGui::TableSetColumnIndex(2);
					ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth());
					int applyActionActToAll = ConditionEditor(std::string(Lang::T("RM_BTN_EDIT_CONDITIONS")) + "##conditionalactivation" + std::to_string(i), &action->Activation, Lang::T("RM_TIP_APPLYALL_ACTION"), &action->OnlyExecuteIfPrevious);
					if (applyActionActToAll > -1 || applyActionActToAll == -99)
					{
						this->EditingMenu->ApplyConditionToAll(this->EditingItem, &action->Activation, applyActionActToAll);
						HasChanges = true;
					}
					ImGui::TooltipGeneric(ConditionsToString(&action->Activation, &action->OnlyExecuteIfPrevious).c_str());

					ImGui::TableSetColumnIndex(3);
					if (ImGui::ArrowButtonCondDisabled(("up_action##" + std::to_string(i)).c_str(), ImGuiDir_Up, i == 0))
					{
						ActionBase* tmp = this->EditingItem->Actions[i - 1];
						this->EditingItem->Actions[i - 1] = this->EditingItem->Actions[i];
						this->EditingItem->Actions[i] = tmp;
						HasChanges = true;
					}
					ImGui::SameLine();
					if (ImGui::ArrowButtonCondDisabled(("dn_action##" + std::to_string(i)).c_str(), ImGuiDir_Down, i == this->EditingItem->Actions.size() - 1))
					{
						ActionBase* tmp = this->EditingItem->Actions[i + 1];
						this->EditingItem->Actions[i + 1] = this->EditingItem->Actions[i];
						this->EditingItem->Actions[i] = tmp;
						HasChanges = true;
					}
					ImGui::SameLine();
					if (ImGui::CrossButton(("x_action##" + std::to_string(i)).c_str()))
					{
						idxDel = i;
						HasChanges = true;
					}

					i++;
				}

				ImGui::EndTable();
			}

			if (ImGui::Button(Lang::T("RM_BTN_ADD_ACTION")))
			{
				this->EditingMenu->AddItemAction(this->EditingItem->Identifier, EActionType::InputBind, "");
				HasChanges = true;
			}

			if (idxDel != -1)
			{
				this->EditingMenu->RemoveItemAction(this->EditingItem->Identifier, idxDel);
			}
		}
		ImGui::EndChild();

		ImGui::EndTabItem();
	}
}

void CRadialContext::RenderSettingsTab()
{
	if (ImGui::BeginTabItem((std::string(Lang::T("RM_TAB_SETTINGS")) + "##radialmenus").c_str()))
	{
		ImGui::TextDisabled("%s", Lang::T("RM_LBL_WIDGET"));
		ImGui::HelpMarker(Lang::T("RM_TIP_WIDGET"));

		ImGui::Checkbox((std::string(Lang::T("RM_CHK_EDIT_POSITION")) + "##radialmenus_widget").c_str(), &this->ItemProcessor.IsEditing);

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		// Interface language. Labels are never translated so the choice stays
		// readable in either language.
		static constexpr const char* kLanguageLabels[] = { "简体中文", "English" };
		int languageIndex = Lang::ToSettingsValue(Lang::Current());
		ImGui::TextDisabled("%s", Lang::T("RM_LBL_LANGUAGE"));
		ImGui::HelpMarker(Lang::T("RM_TIP_LANGUAGE"));
		if (ImGui::Combo("##radialmenus_language", &languageIndex, kLanguageLabels, 2))
		{
			const Lang::RMLanguage chosen = Lang::FromSettingsValue(languageIndex);
			Lang::SetLanguage(chosen);
			Lang::SaveConfig();
		}

		ImGui::EndTabItem();
	}
}

void CRadialContext::RenderOptions()
{
	const std::lock_guard<std::mutex> lock(this->Mutex);

	if (ImGui::BeginTabBar("##radialmenustabbar"))
	{
		RenderEditorTab();
		RenderSettingsTab();

		if (ImGui::BeginTabItem((std::string(Lang::T("RM_TAB_DEBUG")) + "##radialmenus").c_str()))
		{
			StateObserver::RenderDebug();
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}
}

void CRadialContext::OnInputBind(std::string aIdentifier, bool aIsRelease)
{
	auto it = this->RadialIBMap.find(aIdentifier);

	if (it != this->RadialIBMap.end())
	{
		if (!aIsRelease)
		{
			this->Activate(it->second);
		}
		else
		{
			this->Release(ESelectionMode::Release);
		}
	}
}

UINT CRadialContext::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_KEYDOWN:
		{
			if (wParam == VK_ESCAPE)
			{
				return this->Release(ESelectionMode::Escape) ? 0 : 1;
			}
			break;
		}
		case WM_LBUTTONDBLCLK:
		{
			this->IsLeftClickHeld = true;
			break;
		}
		case WM_LBUTTONDOWN:
		{
			this->IsLeftClickHeld = true;
			return this->Release(ESelectionMode::Click) ? 0 : 1;
			break;
		}
		case WM_LBUTTONUP:
		{
			this->IsLeftClickHeld = false;
			break;
		}
		case WM_RBUTTONDBLCLK:
		case WM_RBUTTONDOWN:
		{
			this->IsRightClickHeld = true;
			return this->Release(ESelectionMode::Escape) ? 0 : 1;
			break;
		}
		case WM_RBUTTONUP:
		{
			this->IsRightClickHeld = false;
			break;
		}
	}

	return uMsg;
}

void CRadialContext::QueueItem(RadialItem* aItem)
{
	this->ItemProcessor.QueueItem(aItem);
}

void CRadialContext::LoadInternal()
{
	this->ActiveRadial = nullptr;
	this->EditingMenu = nullptr;
	this->EditingItem = nullptr;
	while (this->Radials.size() > 0) /* clear all, with proper cleanup */
	{
		this->Remove(this->Radials.front()->GetName());
	}
	this->RadialIBMap.clear();

	std::lock_guard<std::mutex> uklock(UnboundKeysMutex);
	UnboundKeys.clear();

	for (const std::filesystem::directory_entry entry : std::filesystem::directory_iterator(PacksDirectory))
	{
		std::filesystem::path filePath = entry.path();

		if (!std::filesystem::is_regular_file(filePath))
		{
			continue;
		}

		if (filePath.extension() != ".json")
		{
			continue;
		}

		bool anyCollision = false;

		try
		{
			std::ifstream file(filePath.string());

			const json radialData = json::parse(file);
			
			auto revision = radialData.value("FormatRevision", 1);

			auto id = radialData.at("ID").get<int>();
			auto name = this->GetUnusedName(radialData.at("Name").get<std::string>());
			auto type = radialData.value("Type", ERadialType::None);
			auto innerRadius = radialData.value("InnerRadius", EInnerRadius::Big);
			auto selMode = radialData.value("SelectionMode", ESelectionMode::None);
			auto centerBehavior = radialData.value("CenterBehavior", ECenterBehavior::None);

			auto drawInCenter = radialData.value("DrawInCenter", false);
			auto doNotCenterCursor = radialData.value("DoNotCenterCursor", false);
			auto restoreCursor = radialData.value("RestoreCursor", false);
			auto scale = radialData.value("Scale", 1.0f);
			auto iconScale = radialData.value("IconScale", 1.0f);
			auto hoverTimeout = radialData.value("HoverTimeout", 0);
			auto itemRotation = radialData.value("ItemRotation", 0);
			auto showTooltip = radialData.value("ShowItemNameTooltip", false);
			auto centerItemName = radialData.value("CenterItemName", "");

			bool idCollision = this->IsIDInUse(id);

			if (idCollision)
			{
				anyCollision = true;
				id = 0;
			}

			CRadialMenu* radial = this->Add(filePath, name, type, selMode, id);
			radial->SetInnerRadius(innerRadius);
			radial->SetCenterBehavior(centerBehavior);
			radial->DrawInCenter = drawInCenter;
			radial->DoNotCenterCursor = doNotCenterCursor;
			radial->RestoreCursor = restoreCursor;
			radial->Scale = scale;
			radial->IconScale = iconScale;
			radial->HoverTimeout = hoverTimeout;
			radial->ItemRotationDegrees = itemRotation;
			radial->ShowItemNameTooltip = showTooltip;
			radial->SpecificCenterItemName = centerItemName;

			for (const json& radialItemData : radialData.value("Items", json::array()))
			{
				if (!radialItemData.contains("Name"))
				{
					continue;
				}

				auto itemId = radialItemData.at("Name").get<std::string>();
				auto priority = radialItemData.value("Priority", 0);
				auto color = radialItemData.value("Color", 0u);
				auto colorHover = radialItemData.value("ColorHover", 0u);
				auto iconType = radialItemData.value("IconType", EIconType::None);
				auto iconValue = radialItemData.value("IconValue", "");
				auto visibility = radialItemData.value("Visibility", Conditions{});
				auto activation = radialItemData.value("Activation", Conditions{});
				auto activationTimeout = radialItemData.value("ActivationTimeout", 0);

				/* correct mount conditions if rev 1*/
				if (revision == 1)
				{
					if ((int)visibility.IsMounted == 0)      { visibility.IsMounted = EObserveMount::Either; }
					else if ((int)visibility.IsMounted == 1) { visibility.IsMounted = EObserveMount::NotMounted; }
					else if ((int)visibility.IsMounted == 2) { visibility.IsMounted = EObserveMount::Any; }

					if ((int)activation.IsMounted == 0)      { activation.IsMounted = EObserveMount::Either; }
					else if ((int)activation.IsMounted == 1) { activation.IsMounted = EObserveMount::NotMounted; }
					else if ((int)activation.IsMounted == 2) { activation.IsMounted = EObserveMount::Any; }
				}

				radial->AddItem(itemId, color, colorHover, iconType, iconValue, priority, visibility, activation, activationTimeout);

				for (const json& radialActionData : radialItemData.value("Actions", json::array()))
				{
					auto actionType = radialActionData.value("Type", EActionType::None);
					auto actionActivation = radialActionData.value("Activation", Conditions{});
					bool execCond = radialActionData.value("OnlyExecuteIfPrevious", false);

					if (revision == 1)
					{
						if ((int)actionActivation.IsMounted == 0)      { actionActivation.IsMounted = EObserveMount::Either; }
						else if ((int)actionActivation.IsMounted == 1) { actionActivation.IsMounted = EObserveMount::NotMounted; }
						else if ((int)actionActivation.IsMounted == 2) { actionActivation.IsMounted = EObserveMount::Any; }
					}

					switch (actionType)
					{
						case EActionType::InputBind:
						case EActionType::Event:
						{
							auto actionIdentifier = radialActionData.value("Identifier", "");

							radial->AddItemAction(itemId, actionType, actionIdentifier, actionActivation, execCond);
							break;
						}
						case EActionType::GameInputBind:
						case EActionType::GameInputBindPress:
						case EActionType::GameInputBindRelease:
						{
							auto actionIdentifier = radialActionData.value("Identifier", (EGameBinds)0);

							/* Migrate legacy gamebind that the game removed. */
							if (actionIdentifier == (EGameBinds)10)
							{
								actionIdentifier = EGameBinds_MoveJump_SwimUp_FlyUp;
							}

							if (!APIDefs->GameBinds.IsBound(actionIdentifier))
							{
								auto& keyUsers = UnboundKeys[actionIdentifier];

								if (std::find(keyUsers.begin(), keyUsers.end(), name) == keyUsers.end())
								{
									keyUsers.push_back(name);
								}
							}

							radial->AddItemAction(itemId, actionType, actionIdentifier, actionActivation, execCond);
							break;
						}
						case EActionType::Delay:
						{
							auto actionDuration = radialActionData.value("Duration", 0);

							radial->AddItemAction(itemId, actionDuration, actionActivation, execCond);
							break;
						}
						case EActionType::Return:
						{
							radial->AddItemAction(itemId, EActionType::Return, actionActivation, execCond);
							break;
						}
					}
				}
			}
		}
		catch (const json::exception& ex)
		{
			APIDefs->Log(ELogLevel_WARNING, "Radial Menus", String::Format("%s could not be parsed. Error: %s", filePath.filename().string().c_str(), ex.what()).c_str());
		}

		if (anyCollision)
		{
			/* after all radials were properly mapped, check all ids that are 0 and reset them to the lowest possible */
			for (CRadialMenu* radial : this->Radials)
			{
				if (radial->GetID() == 0)
				{
					radial->SetID(this->GetLowestUnusedID());
				}
			}
			this->GenerateIBMap();
			this->SaveInternal();
			APIDefs->UI.SendAlert(Lang::T("RM_ALERT_ID_CONFLICT"));
		}
	}

	/* 迁移：早期版本创建预设轮盘时，条目名称是写死的英文（Raptor 等），
	   这些名字会存进 packs/*.json 并直接显示为条目名。此处把仍为英文的
	   预设坐骑条目重命名为当前界面语言（默认简体中文）的译名，并同步更新
	   “指定条目”引用，避免用户此前生成的配置里残留英文。 */
	{
		static const std::map<std::string, const char*> kPresetMountIds = {
			{ "Raptor", "RM_MOUNT_RAPTOR" },
			{ "Springer", "RM_MOUNT_SPRINGER" },
			{ "Skimmer", "RM_MOUNT_SKIMMER" },
			{ "Jackal", "RM_MOUNT_JACKAL" },
			{ "Griffon", "RM_MOUNT_GRIFFON" },
			{ "Roller Beetle", "RM_MOUNT_ROLLERBEETLE" },
			{ "Warclaw", "RM_MOUNT_WARCLAW" },
			{ "Skyscale", "RM_MOUNT_SKYSCALE" },
			{ "Skiff", "RM_MOUNT_SKIFF" },
			{ "Siege Turtle", "RM_MOUNT_SIEGETURTLE" },
		};

		bool migrated = false;

		for (CRadialMenu* radial : this->Radials)
		{
			if (!radial) { continue; }

			for (RadialItem* item : radial->GetItems())
			{
				if (!item) { continue; }

				auto it = kPresetMountIds.find(item->Identifier);
				if (it == kPresetMountIds.end()) { continue; }

				const std::string localized = Lang::T(it->second);
				if (localized.empty() || localized == item->Identifier) { continue; }

				/* “指定条目”可能引用旧名称，同步更新 */
				if (radial->SpecificCenterItemName == item->Identifier)
				{
					radial->SpecificCenterItemName = localized;
				}

				item->Identifier = localized;
				migrated = true;
			}
		}

		if (migrated)
		{
			this->GenerateIBMap();
			this->SaveInternal();
		}
	}

	HasChanges = false;
}
void CRadialContext::SaveInternal()
{
	/* FIXME: save mappings */

	for (CRadialMenu* radial : this->Radials)
	{
		radial->Save();
	}

	HasChanges = false;
}

CRadialMenu* CRadialContext::Add(std::filesystem::path aPath, std::string aIdentifier, ERadialType aRadialMenuType, ESelectionMode aSelectionMode, int aID)
{
	if (aID == -1) { aID = this->GetLowestUnusedID(); }

	CRadialMenu* radial = new CRadialMenu(APIDefs, SelfModule, aPath, aID, aIdentifier, 1.0f, 0, aRadialMenuType, aSelectionMode);
	std::string bind = radial->GetInputBind();
	APIDefs->InputBinds.RegisterWithString(bind.c_str(), Addon::OnInputBind, "(null)");
	APIDefs->Localization.Set(bind.c_str(), "en", aIdentifier.c_str());
	this->Radials.push_back(radial);
	this->GenerateIBMap();

	return radial;
}

void CRadialContext::Remove(std::string aIdentifier)
{
	auto it = std::find_if(this->Radials.begin(), this->Radials.end(), [aIdentifier](CRadialMenu* menu) { return menu->GetName() == aIdentifier; });

	if (it != this->Radials.end())
	{
		if (this->ActiveRadial == (*it))
		{
			this->Release(ESelectionMode::Escape);
		}

		std::string bind = (*it)->GetInputBind();
		APIDefs->InputBinds.Deregister(bind.c_str());
		delete (*it);
		this->Radials.erase(it);
		this->GenerateIBMap();
	}
}
void CRadialContext::Remove(std::filesystem::path aPath)
{
	auto it = std::find_if(this->Radials.begin(), this->Radials.end(), [aPath](CRadialMenu* menu) { return menu->GetPath() == aPath; });

	if (it != this->Radials.end())
	{
		if (this->ActiveRadial == (*it))
		{
			this->Release(ESelectionMode::Escape);
		}

		std::string bind = (*it)->GetInputBind();
		APIDefs->InputBinds.Deregister(bind.c_str());
		delete (*it);
		this->Radials.erase(it);
		this->GenerateIBMap();
	}
}

int CRadialContext::GetLowestUnusedID()
{
	/* iterate over radials until an unused ID is found */
	int lowestUnusedId = 1;
	bool collision = false;
	do
	{
		/* reset collision check */
		collision = false;

		for (CRadialMenu* r : this->Radials)
		{
			if (r->GetID() == lowestUnusedId)
			{
				/* id is used, set collision to true to loop again and increment id */
				collision = true;
				lowestUnusedId++;
			}
		}
	} while (collision);

	return lowestUnusedId;
}
bool CRadialContext::IsIDInUse(int aID, CRadialMenu* aRadialSkip)
{
	for (CRadialMenu* r : this->Radials)
	{
		if (aRadialSkip != nullptr && aRadialSkip == r) { continue; }

		if (r->GetID() == aID)
		{
			return true;
		}
	}

	return false;
}

std::string CRadialContext::GetUnusedName(std::string aName, CRadialMenu* aRadialSkip)
{
	// if the "name" is in use get "name (2)", "name (3)" etc
	std::string newName = aName;
	int i = 1;
	while (this->IsNameInUse(newName, this->EditingMenu))
	{
		i++;
		newName = aName;
		newName.append(" (" + std::to_string(i) + ")");
	}

	return newName;
}
bool CRadialContext::IsNameInUse(std::string aName, CRadialMenu* aRadialSkip)
{
	for (CRadialMenu* r : this->Radials)
	{
		if (aRadialSkip != nullptr && aRadialSkip == r) { continue; }

		if (r->GetName() == aName)
		{
			return true;
		}
	}

	return false;
}

void CRadialContext::GenerateIBMap()
{
	this->RadialIBMap.clear();

	for (CRadialMenu* r : this->Radials)
	{
		this->RadialIBMap[r->GetInputBind()] = r;
	}
}
