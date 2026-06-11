mod auth;
mod launcher;

#[cfg_attr(mobile, tauri::mobile_entry_point)]
pub fn run() {
    tauri::Builder::default()
        .plugin(tauri_plugin_shell::init())
        .invoke_handler(tauri::generate_handler![
            // Auth (ely.by)
            auth::login_elyby,
            auth::get_saved_session,
            auth::logout,
            auth::get_skin_url,
            auth::get_cape_url,
            // Launcher
            launcher::fetch_versions,
            launcher::get_installed_versions,
            launcher::install_version,
            launcher::delete_version,
            launcher::launch_game,
            launcher::get_config,
            launcher::save_config,
            launcher::get_java_info,
        ])
        .run(tauri::generate_context!())
        .expect("error while running WenzLauncher");
}
