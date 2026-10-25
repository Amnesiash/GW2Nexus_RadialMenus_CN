#ifndef LANGUAGE_H
#define LANGUAGE_H

#include "Nexus/Nexus.h"

extern AddonAPI* APIDefs;

///----------------------------------------------------------------------------------------------------
/// Lang Namespace
/// 	Localization for the Radial Menus addon.
/// 	Translations are registered through the Nexus localization API on load.
/// 	"en" is the fallback language, "cn" is Simplified Chinese (Nexus language id).
///----------------------------------------------------------------------------------------------------
namespace Lang
{
	///----------------------------------------------------------------------------------------------------
	/// T:
	/// 	Translates an identifier into the user's current Nexus language.
	///----------------------------------------------------------------------------------------------------
	inline const char* T(const char* aIdentifier)
	{
		if (aIdentifier == nullptr) { return ""; }
		if (APIDefs == nullptr || APIDefs->Localization.Translate == nullptr) { return aIdentifier; }

		/* Nexus 未注册该标识符时 Translate 可能返回 nullptr，
		   直接传给 ImGui::Text 或 std::string 会触发空指针崩溃，故兜底返回标识符本身 */
		const char* translated = APIDefs->Localization.Translate(aIdentifier);
		return translated != nullptr ? translated : aIdentifier;
	}

	///----------------------------------------------------------------------------------------------------
	/// Init:
	/// 	Registers all English and Simplified Chinese texts.
	///----------------------------------------------------------------------------------------------------
	inline void Init(LOCALIZATION_SET aSetterFunc)
	{
		if (aSetterFunc == nullptr) { return; }

		struct Entry
		{
			const char* Id;
			const char* En;
			const char* Cn;
		};

		static const Entry Entries[] =
		{
			/* keybinds */
			{ "KB_RADIALS", "All Radials", "全部轮盘" },

			/* tabs */
			{ "RM_TAB_EDITOR", "Editor", "编辑器" },
			{ "RM_TAB_SETTINGS", "Settings", "设置" },
			{ "RM_TAB_DEBUG", "Debug", "调试" },

			/* editor tab: toolbar */
			{ "RM_BTN_RELOAD_RADIALS", "Reload radials", "重新加载轮盘" },
			{ "RM_BTN_SAVE_CHANGES", "Save changes", "保存更改" },
			{ "RM_BTN_OPEN_FOLDER", "Open folder", "打开文件夹" },
			{ "RM_BTN_NEW_RADIAL", "New radial", "新建轮盘" },
			{ "RM_MSG_UNSAVED_CHANGES", "You might have unsaved changes.", "可能存在未保存的更改。" },
			{ "RM_MSG_MISSING_BINDS", "Some radial menus may not function as intended. Some binds are missing. (?)", "部分轮盘菜单可能无法正常工作：缺少按键绑定。(?)" },
			{ "RM_TIP_MISSING_BINDS_1", "Set the missing binds in the Nexus Game Keybinds settings.\nThey should match the bind you are using in-game.", "请在 Nexus 的「游戏按键绑定」设置中配置缺失的绑定。\n它们需要与游戏内实际使用的按键一致。" },
			{ "RM_TIP_MISSING_BINDS_2", "Missing binds:", "缺失的绑定：" },
			{ "RM_TIP_USED_BY", "used by ", "使用者：" },

			/* editor tab: radial list */
			{ "RM_LBL_SELECT_RADIAL", "Select Radial Menu:", "选择轮盘菜单：" },
			{ "RM_BTN_DELETE", "Delete", "删除" },
			{ "RM_LBL_SETTINGS", "Settings", "设置" },
			{ "RM_LBL_ITEM", "Item", "条目" },
			{ "RM_BTN_ADD_NEW_ITEM", "Add New Item", "添加新条目" },
			{ "RM_MSG_ONLY_N_ITEMS", "Only %d items will be drawn.", "仅会绘制 %d 个条目。" },

			/* editor tab: menu settings */
			{ "RM_LBL_ID", "ID: %d", "ID：%d" },
			{ "RM_LBL_PATH", "Path: %s", "路径：%s" },
			{ "RM_LBL_INPUT_BIND", "Input Bind", "按键绑定" },
			{ "RM_LBL_SHOWN_AS", "shown as \"%s\"", "显示为 \"%s\"" },
			{ "RM_LBL_NAME", "Name", "名称" },
			{ "RM_TIP_NAME_UNIQUE", "This name has to be unique.", "名称必须唯一。" },
			{ "RM_LBL_TYPE", "Type", "类型" },
			{ "RM_TYPE_SMALL", "Small", "小型" },
			{ "RM_TYPE_NORMAL", "Normal", "普通" },
			{ "RM_LBL_INNER_RADIUS", "Inner Radius", "内半径" },
			{ "RM_RADIUS_SMALL", "Small", "小" },
			{ "RM_RADIUS_MEDIUM", "Medium", "中" },
			{ "RM_RADIUS_BIG", "Big", "大" },
			{ "RM_LBL_SCALE", "Scale", "缩放" },
			{ "RM_LBL_ICON_SCALE", "Icon Scale", "图标缩放" },
			{ "RM_LBL_SELECTION_MODE", "Selection Mode", "选取模式" },
			{ "RM_SELMODE_CLICK", "Click", "点击" },
			{ "RM_TIP_SELMODE_CLICK", "Left mouse button will select the element.", "按下鼠标左键选中元素。" },
			{ "RM_SELMODE_RELEASE", "Release", "松开" },
			{ "RM_TIP_SELMODE_RELEASE", "Releasing the input bind will select the element.", "松开按键绑定时选中元素。" },
			{ "RM_SELMODE_RELEASE_CLICK", "Release / Click", "松开 / 点击" },
			{ "RM_TIP_SELMODE_RELEASE_CLICK", "Releasing the input bind or left mouse button will select the element.", "松开按键绑定或鼠标左键时选中元素。" },
			{ "RM_SELMODE_HOVER", "Hover", "悬停" },
			{ "RM_TIP_SELMODE_HOVER", "Hovering over an element will immediately select it.", "悬停在元素上会立即选中。" },
			{ "RM_LBL_HOVER_TIMEOUT", "Hover Timeout (milliseconds)", "悬停超时（毫秒）" },
			{ "RM_TIP_HOVER_TIMEOUT", "This timeout controls how long an item needs to be hovered before it gets activated.", "该超时控制条目需要被悬停多久后才会被激活。" },
			{ "RM_LBL_CENTER_BEHAVIOR", "Center Behavior", "中心行为" },
			{ "RM_TIP_CENTER_BEHAVIOR", "This setting controls what to do if no item is selected.", "该设置控制未选中任何条目时的行为。" },
			{ "RM_CENTER_NONE", "Do nothing.", "不做任何操作。" },
			{ "RM_CENTER_FIRST", "Activate the first item, matching conditions.", "激活第一个满足条件的条目。" },
			{ "RM_CENTER_SPECIFIC", "Execute a specific item.", "执行指定条目。" },
			{ "RM_CENTER_LASTUSED", "Execute a last activated item.", "执行上次激活的条目。" },
			{ "RM_LBL_SPECIFIC_ITEM", "Specific Item Name", "指定条目名称" },
			{ "RM_TIP_SPECIFIC_ITEM", "This is the item that will be executed when not selecting any.", "未选中任何条目时将执行该条目。" },
			{ "RM_LBL_NONE", "(none)", "（无）" },
			{ "RM_LBL_ITEMS_ROTATION", "Items Rotation", "条目旋转" },
			{ "RM_TIP_ITEMS_ROTATION", "This controls the location of the items by rotating them.\nControl-Click to manually edit.", "通过旋转控制条目的位置。\n按住 Ctrl 点击可手动编辑。" },
			{ "RM_CHK_DRAW_IN_CENTER", "Draw in Center", "在屏幕中心绘制" },
			{ "RM_CHK_NO_CENTER_CURSOR", "Do not set cursor to center on activation", "激活时不将光标移至中心" },
			{ "RM_CHK_RESTORE_CURSOR", "Restore Cursor Position", "恢复光标位置" },
			{ "RM_CHK_SHOW_TOOLTIPS", "Show Item Name Tooltips", "显示条目名称提示" },

			/* editor tab: item settings */
			{ "RM_LBL_PRIORITY", "Priority", "优先级" },
			{ "RM_TIP_PRIORITY", "This setting controls which item to activate on center, if multiple items match the conditions. Higher means higher priority.", "当多个条目满足条件时，该设置决定中心激活哪一个。数值越大优先级越高。" },
			{ "RM_LBL_COLOR", "Color", "颜色" },
			{ "RM_BTN_APPLY_TO_ALL", "Apply to all", "应用到全部" },
			{ "RM_TIP_APPLY_COLOR_ALL", "Applies this item's Color to all items Color in this Radial Menu.", "将该条目的颜色应用到本轮盘所有条目的颜色。" },
			{ "RM_LBL_COLOR_HOVER", "Color Hover", "悬停颜色" },
			{ "RM_TIP_APPLY_COLORHOVER_ALL", "Applies this item's Hover Color to all items Hover Color in this Radial Menu.", "将该条目的悬停颜色应用到本轮盘所有条目的悬停颜色。" },
			{ "RM_LBL_ICON", "Icon", "图标" },
			{ "RM_ICON_FILE", "File", "文件" },
			{ "RM_ICON_URL", "URL", "网址" },
			{ "RM_LBL_VISIBILITY", "Visibility", "可见性" },
			{ "RM_TIP_VISIBILITY", "These conditions control when this item is visible.", "这些条件控制该条目何时可见。" },
			{ "RM_LBL_ACTIVATION", "Activation", "激活" },
			{ "RM_TIP_ACTIVATION", "These conditions control whether to queue the item until they are met or if it can be immediately activated.", "这些条件控制该条目是排队等待条件满足，还是可以立即激活。" },
			{ "RM_LBL_TIMEOUT_SECONDS", "Timeout (seconds)", "超时（秒）" },
			{ "RM_TIP_TIMEOUT", "This timeout controls how long to wait for until the conditions are met before aborting.", "该超时控制等待条件满足的最长时间，超时后放弃。" },
			{ "RM_LBL_ACTIONS", "Actions", "动作" },
			{ "RM_TIP_ACTIONS", "This sequence of actions will be executed in order when selecting the item.\nSelecting another item cancels execution.", "选中该条目时将按顺序执行这一系列动作。\n选择其他条目会取消执行。" },
			{ "RM_MSG_MACRO_POLICY", "By using this editor you acknowledge to have read and understood the official Macro Policy.", "使用本编辑器即表示你已阅读并理解官方宏政策。" },
			{ "RM_LBL_MACRO_POLICY_LINK", "You can read it here.", "可在此阅读。" },

			/* action types */
			{ "RM_ACTION_INPUTBIND_NEXUS", "InputBind (Nexus)", "按键绑定（Nexus）" },
			{ "RM_ACTION_INPUTBIND_GAME", "InputBind (Game)", "按键绑定（游戏）" },
			{ "RM_ACTION_INPUTBIND_PRESS_GAME", "InputBind Press (Game)", "按键按下（游戏）" },
			{ "RM_ACTION_INPUTBIND_RELEASE_GAME", "InputBind Release (Game)", "按键松开（游戏）" },
			{ "RM_ACTION_EVENT", "Event", "事件" },
			{ "RM_ACTION_DELAY", "Delay", "延迟" },
			{ "RM_ACTION_RETURN", "Return", "返回" },
			{ "RM_TIP_ACTION_RETURN", "Return prevents the execution of any further actions.\nUseful when linked with previous conditional actions.", "「返回」会阻止后续动作的执行。\n与「与上一个动作链接」配合使用时很有用。" },
			{ "RM_BTN_ADD_ACTION", "Add Action", "添加动作" },

			/* conditions editor */
			{ "RM_BTN_EDIT_CONDITIONS", "Edit Conditions", "编辑条件" },
			{ "RM_TIP_APPLYALL_VISIBILITY", "\"Apply to all\" will affect the visibility of all items in this radial.", "「应用到全部」将影响本轮盘中所有条目的可见性。" },
			{ "RM_TIP_APPLYALL_ACTIVATION", "\"Apply to all\" will affect the activation of all items in this radial.", "「应用到全部」将影响本轮盘中所有条目的激活条件。" },
			{ "RM_TIP_APPLYALL_ACTION", "\"Apply to all\" will affect of all actions in this item.", "「应用到全部」将影响该条目中的所有动作。" },
			{ "RM_TIP_UNBOUND_GAMEBIND", "Bind this Game InputBind via the Nexus options in order to be able to use it.\nIt must match the game.", "请在 Nexus 选项中绑定该游戏按键后才能使用。\n它必须与游戏内绑定一致。" },
			{ "RM_STATE_EITHER", "Either", "不限" },
			{ "RM_STATE_MUST_FALSE", "Must be false", "必须为否" },
			{ "RM_STATE_MUST_TRUE", "Must be true", "必须为是" },
			{ "RM_COND_IS_IN_COMBAT", "Is in combat", "处于战斗中" },
			{ "RM_COND_IS_MOUNTED", "Is mounted", "骑乘状态" },
			{ "RM_COND_IS_COMMANDER", "Is commander", "指挥官状态" },
			{ "RM_COND_IS_PVPWVW", "Is in PvP/WvW", "处于 PvP/WvW" },
			{ "RM_COND_IS_MAP_OPEN", "Is map open", "地图打开状态" },
			{ "RM_COND_IS_TEXTBOX_ACTIVE", "Is textbox active", "输入框聚焦状态" },
			{ "RM_COND_IS_INSTANCE", "Is in instance", "处于副本中" },
			{ "RM_COND_IS_GAMEPLAY", "Is gameplay", "游戏进行状态" },
			{ "RM_COND_IS_UNDERWATER", "Is underwater", "水下状态" },
			{ "RM_COND_IS_ON_WATER_SURFACE", "Is on water surface", "水面状态" },
			{ "RM_COND_IS_AIRBORNE", "Is airborne", "滞空状态" },
			{ "RM_TIP_AIRBORNE", "Will be true when falling, gliding using an updraft, or after jumping.", "下落、借助上升气流滑翔或跳跃后为真。" },
			{ "RM_CHK_LINK_WITH_PREVIOUS", "Link with previous action", "与上一个动作链接" },
			{ "RM_TIP_LINK_WITH_PREVIOUS", "Besides the set conditions, setting this means that the previous action must have executed for this one to be able to.", "除已设置的条件外，勾选后还要求上一个动作已执行，本动作才能执行。" },
			{ "RM_BTN_APPLY_ALL_CONDITIONS", "Apply all conditions to all", "将全部条件应用到全部" },
			{ "RM_BTN_DONE", "Done", "完成" },

			/* condition summary lines */
			{ "RM_COND_HEADER", "Required conditions:\n", "所需条件：\n" },
			{ "RM_COND_LINKED", "Linked with previous.", "与上一个动作链接。" },
			{ "RM_COND_LINE_IN_COMBAT", "- Is in combat\n", "- 处于战斗中\n" },
			{ "RM_COND_LINE_OUT_COMBAT", "- Is out of combat\n", "- 处于战斗外\n" },
			{ "RM_COND_LINE_MOUNTED", "- Is mounted\n", "- 已骑乘\n" },
			{ "RM_COND_LINE_NOT_MOUNTED", "- Is not mounted\n", "- 未骑乘\n" },
			{ "RM_COND_LINE_ON_MOUNT", "- Is on ", "- 骑乘：" },
			{ "RM_COND_LINE_CMD", "- Has commander tag\n", "- 拥有指挥官标记\n" },
			{ "RM_COND_LINE_NO_CMD", "- Does not have commander tag\n", "- 没有指挥官标记\n" },
			{ "RM_COND_LINE_COMP", "- Is in WvW/PvP\n", "- 处于 WvW/PvP\n" },
			{ "RM_COND_LINE_PVE", "- Is in PvE\n", "- 处于 PvE\n" },
			{ "RM_COND_LINE_MAP_OPEN", "- Map is open\n", "- 地图已打开\n" },
			{ "RM_COND_LINE_MAP_CLOSED", "- Map is closed\n", "- 地图已关闭\n" },
			{ "RM_COND_LINE_TEXTBOX", "- Textbox focused\n", "- 输入框已聚焦\n" },
			{ "RM_COND_LINE_NO_TEXTBOX", "- Textbox not focused\n", "- 输入框未聚焦\n" },
			{ "RM_COND_LINE_INSTANCE", "- Is in instance\n", "- 处于副本中\n" },
			{ "RM_COND_LINE_NO_INSTANCE", "- Is not in instance\n", "- 不处于副本中\n" },
			{ "RM_COND_LINE_GAMEPLAY", "- Is gameplay\n", "- 处于游戏画面\n" },
			{ "RM_COND_LINE_NOT_GAMEPLAY", "- Is loading screen/cutscene/character select\n", "- 处于加载画面/过场动画/角色选择\n" },
			{ "RM_COND_LINE_UNDERWATER", "- Is underwater\n", "- 处于水下\n" },
			{ "RM_COND_LINE_TERRESTRIAL", "- Is terrestrial\n", "- 处于陆地\n" },
			{ "RM_COND_LINE_ON_WATER", "- Is on water surface\n", "- 处于水面\n" },
			{ "RM_COND_LINE_NOT_ON_WATER", "- Is not on water surface\n", "- 不处于水面\n" },
			{ "RM_COND_LINE_AIRBORNE", "- Is airborne", "- 处于滞空" },
			{ "RM_COND_LINE_NOT_AIRBORNE", "- Is not airborne", "- 未滞空" },

			/* mounts */
			{ "RM_MOUNT_ANY", "Any Mount", "任意坐骑" },
			{ "RM_MOUNT_NOT", "Not Mounted", "未骑乘" },
			{ "RM_MOUNT_RAPTOR", "肉食鸟", "肉食鸟" },
			{ "RM_MOUNT_SPRINGER", "弹跳兔", "弹跳兔" },
			{ "RM_MOUNT_SKIMMER", "飞鱼", "飞鱼" },
			{ "RM_MOUNT_JACKAL", "胡狼", "胡狼" },
			{ "RM_MOUNT_GRIFFON", "狮鹫", "狮鹫" },
			{ "RM_MOUNT_ROLLERBEETLE", "翻滚甲虫", "翻滚甲虫" },
			{ "RM_MOUNT_WARCLAW", "战爪", "战爪" },
			{ "RM_MOUNT_SKYSCALE", "飞天鳞龙", "飞天鳞龙" },
			{ "RM_MOUNT_SKIFF", "小船", "小船" },
			{ "RM_MOUNT_SIEGETURTLE", "攻城龟", "攻城龟" },

			/* alerts */
			{ "RM_ALERT_ID_CONFLICT", "Some Radial bindings might have changed due to ID conflicts.", "由于 ID 冲突，部分轮盘绑定可能已发生变化。" },

			/* settings tab */
			{ "RM_LBL_WIDGET", "Widget", "小部件" },
			{ "RM_TIP_WIDGET", "The widget indicates the currently queued action.\nIt has a circular countdown, waiting for the action to start execution.\n\nYou can left-click to refresh the countdown or right-click to cancel the action.", "小部件指示当前排队中的动作。\n它带有一个环形倒计时，等待动作开始执行。\n\n左键可刷新倒计时，右键可取消该动作。" },
			{ "RM_CHK_EDIT_POSITION", "Edit position", "编辑位置" },

			/* debug tab */
			{ "RM_DBG_FALLING", "Falling: %s", "下落：%s" },
			{ "RM_DBG_GLIDING", "Gliding: %s", "滑翔：%s" },
			{ "RM_DBG_ASCENDING", "Ascending: %s", "上升：%s" },
			{ "RM_DBG_JUMPING", "Jumping: %s", "跳跃：%s" },
			{ "RM_DBG_DISMOUNTING", "Dismounting: %s", "下坐骑：%s" },
			{ "RM_DBG_ISCOMBAT", "IsCombat: %s", "战斗中：%s" },
			{ "RM_DBG_ISMOUNTED", "IsMounted: %s", "骑乘：%s" },
			{ "RM_DBG_ISCOMMANDER", "IsCommander: %s", "指挥官：%s" },
			{ "RM_DBG_ISCOMPETITIVE", "IsCompetitive: %s", "竞技模式：%s" },
			{ "RM_DBG_ISMAPOPEN", "IsMapOpen: %s", "地图打开：%s" },
			{ "RM_DBG_ISTEXTBOX", "IsTextboxActive: %s", "输入框聚焦：%s" },
			{ "RM_DBG_ISINSTANCE", "IsInstance: %s", "副本中：%s" },
			{ "RM_DBG_ISGAMEPLAY", "IsGameplay: %s", "游戏画面：%s" },
			{ "RM_DBG_ISUNDERWATER", "IsUnderwater: %s", "水下：%s" },
			{ "RM_DBG_ISONWATER", "IsOnWaterSurface: %s", "水面：%s" },
			{ "RM_DBG_ISAIRBORNE", "IsAirborne: %s", "滞空：%s" }
		};

		for (const Entry& e : Entries)
		{
			aSetterFunc(e.Id, "en", e.En);
			aSetterFunc(e.Id, "cn", e.Cn);
		}
	}
}

#endif
