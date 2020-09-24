#include "beatsaber-hook/shared/utils/utils.h"
#include "beatsaber-hook/shared/utils/logging.hpp"
#include "modloader/shared/modloader.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-functions.hpp"
#include "beatsaber-hook/shared/utils/il2cpp-utils.hpp" 
#include "beatsaber-hook/shared/utils/typedefs.h"

#include <string>

#include "UnityEngine/Vector3.hpp"
#include "UnityEngine/GameObject.hpp"
#include "UnityEngine/Transform.hpp"
#include "UnityEngine/SceneManagement/Scene.hpp"

#include "GlobalNamespace/ConditionalMaterialSwitcher.hpp"
#include "GlobalNamespace/ObstacleController.hpp"
#include "GlobalNamespace/NoteController.hpp"
#include "GlobalNamespace/NoteData.hpp"
#include "GlobalNamespace/GameplayModifiers.hpp"
#include "GlobalNamespace/MenuTransitionsHelper.hpp"
#include "GlobalNamespace/NoteDebris.hpp"
#include "GlobalNamespace/BombNoteController.hpp"
#include "GlobalNamespace/PlayerSpecificSettings.hpp"
#include "GlobalNamespace/SettingsFlowCoordinator.hpp"
#include "GlobalNamespace/SettingsNavigationController.hpp"

#include "Qosmetic/QuestSaber.hpp"
#include "Qosmetic/QuestNote.hpp"
#include "Qosmetic/QuestWall.hpp"

#include "Logging/WallLogger.hpp"
#include "Logging/NoteLogger.hpp"
#include "Logging/SaberLogger.hpp"
#include "Logging/GenericLogger.hpp"

#include "Data/CustomTrail.hpp"

#include "Utils/MaterialUtils.hpp"

bool getSceneName(Scene scene, std::string& output);

std::string sceneLoadedName;

static ModInfo modInfo;

const Logger& getLogger() {
  static const Logger& logger(modInfo);
  return logger;
}

bool wallsEnabled = true;
bool notesEnabled = true;
bool sabersEnabled = true;
bool shaderWarmupFirst = true;
std::string shaderWarmup = "ShaderWarmup";
std::string healthWarning = "HealthWarning";
std::string gameCore = "GameCore";
std::string menuViewControllers = "MenuViewControllers";
std::string emptyTransition = "EmptyTransition";

MAKE_HOOK_OFFSETLESS(MenuTransitionsHelper_StartStandardLevel, void, GlobalNamespace::MenuTransitionsHelper* self, Il2CppObject* difficultyBeatmap, Il2CppObject* overrideEnvironmentSettings, Il2CppObject* overrideColorScheme, GlobalNamespace::GameplayModifiers* gameplayModifiers, GlobalNamespace::PlayerSpecificSettings* playerSpecificSettings, Il2CppObject* practiceSettings, Il2CppString* backButtonText, bool useTestNoteCutSoundEffects, Il2CppObject* beforeSceneSwitchCallback, Il2CppObject* afterSceneSwitchCallback, Il2CppObject* levelFinishedCallback)
{
    if (notesEnabled) 
    {
        Qosmetics::QuestNote::ModifierScoreDisableCheck(gameplayModifiers);
        Qosmetics::QuestNote::ReducedDebrisDisableCheck(playerSpecificSettings);
    }
    MenuTransitionsHelper_StartStandardLevel(self, difficultyBeatmap, overrideEnvironmentSettings, overrideColorScheme, gameplayModifiers, playerSpecificSettings, practiceSettings, backButtonText, useTestNoteCutSoundEffects, beforeSceneSwitchCallback, afterSceneSwitchCallback, levelFinishedCallback);
}

MAKE_HOOK_OFFSETLESS(NoteDebris_Init, void, GlobalNamespace::NoteDebris* self, GlobalNamespace::NoteType noteType, UnityEngine::Transform* initTransform, UnityEngine::Vector3 cutPoint, UnityEngine::Vector3 cutNormal, UnityEngine::Vector3 force, UnityEngine::Vector3 torque, float lifeTime)
{
    
    NoteDebris_Init(self, noteType, initTransform, cutPoint, cutNormal, force, torque, lifeTime);
    if (notesEnabled) Qosmetics::QuestNote::NoteDebris_Init_Post(self, noteType, initTransform, cutPoint, cutNormal);
}

MAKE_HOOK_OFFSETLESS(BombNoteController_Init, void, GlobalNamespace::BombNoteController* self, GlobalNamespace::NoteData* noteData, float worldRotation, UnityEngine::Vector3 moveStartPos, UnityEngine::Vector3 moveEndPos, UnityEngine::Vector3 jumpEndPos, float moveDuration, float jumpDuration, float jumpGravity, float cutDirectionAngleOffset)
{
    noteData->set_cutDirection(GlobalNamespace::NoteCutDirection::_get_Down());
    BombNoteController_Init(self, noteData, worldRotation, moveStartPos, moveEndPos, jumpEndPos, moveDuration, jumpDuration, jumpGravity, cutDirectionAngleOffset);
    if (notesEnabled) Qosmetics::QuestNote::BombController_Init_Post(self);
}

MAKE_HOOK_OFFSETLESS(NoteController_Init, void, GlobalNamespace::NoteController* self, GlobalNamespace::NoteData* noteData, float worldRotation, Vector3 moveStartPos, Vector3 moveEndPos, Vector3 jumpEndPos, float moveDuration, float jumpDuration, float jumpGravity, float cutDirectionAngleOffset)
{
    NoteController_Init(self, noteData, worldRotation, moveStartPos, moveEndPos, jumpEndPos, moveDuration, jumpDuration, jumpGravity, cutDirectionAngleOffset);
    if (notesEnabled) Qosmetics::QuestNote::NoteController_Init_Post(self);
}

MAKE_HOOK_OFFSETLESS(ObstacleController_set_hide, void, GlobalNamespace::ObstacleController* self, bool value)
{
    ObstacleController_set_hide(self, value);
    if (wallsEnabled) Qosmetics::QuestWall::ObstacleController_Set_Hide(self, value);
}

MAKE_HOOK_OFFSETLESS(ObstacleController_Init, void, GlobalNamespace::ObstacleController* self, Il2CppObject* obstacleData, float worldRotation, Vector3 startPos, Vector3 midPos, Vector3 endPos, float move1Duration, float move2Duration, float singleLineWidth, float height)
{
    if (wallsEnabled) Qosmetics::QuestWall::ObstacleController_Init_Pre(self);
    ObstacleController_Init(self, obstacleData, worldRotation, startPos, midPos, endPos, move1Duration, move2Duration, singleLineWidth, height);
}

MAKE_HOOK_OFFSETLESS(SceneManager_ActiveSceneChanged, void, Scene previousActiveScene, Scene nextActiveScene)
{
    getSceneName(nextActiveScene, sceneLoadedName);
    std::string oldScene;
    if (!getSceneName(previousActiveScene, oldScene)) oldScene = "";
    getLogger().info("Found scene %s, handle is %d", sceneLoadedName.c_str(), nextActiveScene.m_Handle);

    if(sceneLoadedName == shaderWarmup && shaderWarmupFirst)
    {
        if (sabersEnabled) sabersEnabled = Qosmetics::QuestSaber::ShaderWarmup();
        if (wallsEnabled) wallsEnabled = Qosmetics::QuestWall::ShaderWarmup();
        if (notesEnabled) notesEnabled = Qosmetics::QuestNote::ShaderWarmup();
        shaderWarmupFirst = false;
    }

    if(sceneLoadedName == healthWarning)
    {
        if (sabersEnabled) Qosmetics::QuestSaber::HealthWarning();
        if (wallsEnabled) Qosmetics::QuestWall::HealthWarning();
        if (notesEnabled) Qosmetics::QuestNote::HealthWarning();
    }

    if(sceneLoadedName == gameCore)
    {
        if (sabersEnabled) Qosmetics::QuestSaber::GameCore();
        if (wallsEnabled) Qosmetics::QuestWall::GameCore();
        if (notesEnabled) Qosmetics::QuestNote::GameCore();
    }

    SceneManager_ActiveSceneChanged(previousActiveScene, nextActiveScene);
    
    if (sceneLoadedName == menuViewControllers)
    {
        if (sabersEnabled) Qosmetics::QuestSaber::MenuViewControllers();
        if (wallsEnabled) Qosmetics::QuestWall::MenuViewControllers();
        if (notesEnabled) Qosmetics::QuestNote::MenuViewControllers();
    }
}

MAKE_HOOK_OFFSETLESS(Saber_Start, void, GlobalNamespace::Saber* self)
{
    Saber_Start(self);
    if (sabersEnabled) Qosmetics::QuestSaber::SaberStart(self);
}

MAKE_HOOK_OFFSETLESS(ConditionalMaterialSwitcher_Awake, void, GlobalNamespace::ConditionalMaterialSwitcher* self)
{
    // basically QuestTrailOverlap is forced with this
    UnityEngine::Transform* trailTransform = self->get_transform();
    std::string thisName = to_utf8(csstrtostr(trailTransform->get_gameObject()->get_name()));
    if (thisName == "Trail(Clone)") return;
    ConditionalMaterialSwitcher_Awake(self);
}

extern "C" void setup(ModInfo& info) 
{
    info.id = ID;
    info.version = VERSION;
    modInfo = info;
    Qosmetics::GenericLogger::modInfo = info;

    Qosmetics::QuestSaber::modInfo.id = string_format("%s Sabers", ID);
    Qosmetics::QuestSaber::modInfo.version = info.version;
    Qosmetics::SaberLogger::modInfo.id = string_format("%s Sabers", ID);
    Qosmetics::SaberLogger::modInfo.version = info.version;

    Qosmetics::QuestNote::modInfo.id = string_format("%s Notes", ID);
    Qosmetics::QuestNote::modInfo.version = info.version;
    Qosmetics::NoteLogger::modInfo.id = string_format("%s Notes", ID);
    Qosmetics::NoteLogger::modInfo.version = info.version;

    Qosmetics::QuestWall::modInfo.id = string_format("%s Walls", ID);
    Qosmetics::QuestWall::modInfo.version = info.version;
    Qosmetics::WallLogger::modInfo.id = string_format("%s Walls", ID);
    Qosmetics::WallLogger::modInfo.version = info.version;
    getLogger().info("If you are reading this then idk what you're doing, go play the game, don't read logs");
    getLogger().info(info.version);
    getLogger().info(info.id);
}

extern "C" void load() 
{
    il2cpp_functions::Init();
    getLogger().info("Installing hooks");
    INSTALL_HOOK_OFFSETLESS(Saber_Start, il2cpp_utils::FindMethodUnsafe("", "Saber", "Start", 0));
    INSTALL_HOOK_OFFSETLESS(ConditionalMaterialSwitcher_Awake, il2cpp_utils::FindMethodUnsafe("", "ConditionalMaterialSwitcher", "Awake", 0)); 
    INSTALL_HOOK_OFFSETLESS(SceneManager_ActiveSceneChanged, il2cpp_utils::FindMethodUnsafe("UnityEngine.SceneManagement", "SceneManager", "Internal_ActiveSceneChanged", 2));
    INSTALL_HOOK_OFFSETLESS(ObstacleController_Init, il2cpp_utils::FindMethodUnsafe("", "ObstacleController", "Init", 9));
    INSTALL_HOOK_OFFSETLESS(ObstacleController_set_hide, il2cpp_utils::FindMethodUnsafe("", "ObstacleController", "set_hide", 1));
    INSTALL_HOOK_OFFSETLESS(NoteController_Init, il2cpp_utils::FindMethodUnsafe("", "NoteController", "Init", 9));
    INSTALL_HOOK_OFFSETLESS(MenuTransitionsHelper_StartStandardLevel, il2cpp_utils::FindMethodUnsafe("", "MenuTransitionsHelper", "StartStandardLevel", 11));
    INSTALL_HOOK_OFFSETLESS(NoteDebris_Init, il2cpp_utils::FindMethodUnsafe("", "NoteDebris", "Init", 7));
    INSTALL_HOOK_OFFSETLESS(BombNoteController_Init, il2cpp_utils::FindMethodUnsafe("", "BombNoteController", "Init", 9));
    
    getLogger().info("Hooks installed");
}

bool getSceneName(Scene scene, std::string& output)
{
    Il2CppString* csString = RET_0_UNLESS(il2cpp_utils::RunMethod<Il2CppString*>("UnityEngine.SceneManagement", "Scene", "GetNameInternal", scene.m_Handle));
    RET_0_UNLESS(csString);
    output = to_utf8(csstrtostr(csString));
    return true; 
}