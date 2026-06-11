mod launcher;

use launcher::{GameVersion, LauncherConfig};

#[tauri::command]
fn get_versions() -> Vec<GameVersion> {
    launcher::get_available_versions()
}

#[tauri::command]
fn get_config() -> LauncherConfig {
    LauncherConfig::default()
}

#[tauri::command]
async fn launch_game(version: String, username: String) -> Result<String, String> {
    launcher::launch(version, username)
        .await
        .map_err(|e| e.to_string())
}

#[tauri::command]
fn get_java_info() -> String {
    launcher::detect_java().unwrap_or_else(|| "Java не найдена".to_string())
}

#[cfg_attr(mobile, tauri::mobile_entry_point)]
pub fn run() {
    tauri::Builder::default()
        .plugin(tauri_plugin_shell::init())
        .invoke_handler(tauri::generate_handler![
            get_versions,
            get_config,
            launch_game,
            get_java_info,
        ])
        .run(tauri::generate_context!())
        .expect("error while running WenzLauncher");
}
