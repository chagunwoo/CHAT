"""Run once with UE 5.5 PythonScript commandlet after compiling CHATEditor."""
import unreal
import gc

library = unreal.EditorAssetLibrary
gm = unreal.load_class(None, '/Script/CHAT.CHATGameModeBase')
pc = unreal.load_class(None, '/Script/CHAT.CHATPlayerController')
ps = unreal.load_class(None, '/Script/CHAT.CHATPlayerState')
gs = unreal.load_class(None, '/Script/CHAT.CHATGameState')
assert all((gm, pc, ps, gs)), 'CHAT native classes failed to load'

for path in library.list_assets('/Game/ChatX', recursive=True, include_folder=False):
    if '/Maps/' in path:
        continue
    asset = library.load_asset(path)
    assert asset, path
    if isinstance(asset, unreal.Blueprint):
        unreal.BlueprintEditorLibrary.compile_blueprint(asset)
    assert library.save_loaded_asset(asset, only_if_is_dirty=False), path
    del asset

if not library.does_asset_exist('/Game/CHAT/Maps/L_Chat'):
    assert library.rename_directory('/Game/ChatX', '/Game/CHAT')

bp = library.load_asset('/Game/CHAT/Blueprint/Game/BP_GameModeBase')
assert bp
defaults = unreal.get_default_object(bp.generated_class())
defaults.set_editor_property('player_controller_class', pc)
defaults.set_editor_property('player_state_class', ps)
defaults.set_editor_property('game_state_class', gs)
defaults.set_editor_property('default_pawn_class', None)
unreal.BlueprintEditorLibrary.compile_blueprint(bp)
library.save_loaded_asset(bp, only_if_is_dirty=False)

level = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
gc.collect()
unreal.SystemLibrary.collect_garbage()
assert level.load_level('/Game/CHAT/Maps/L_Chat')
world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
world.get_world_settings().set_editor_property('default_game_mode', gm)
assert level.save_current_level()
library.save_directory('/Game/CHAT', only_if_is_dirty=False, recursive=True)
unreal.log('CHAT_ASSET_MIGRATION_OK')
