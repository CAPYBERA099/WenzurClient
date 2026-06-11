use serde::{Deserialize, Serialize};
use std::fs;
use std::path::PathBuf;

/// Аккаунт пользователя (ely.by)
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Account {
    pub username: String,
    pub uuid: String,
    pub access_token: String,
    pub skin_url: Option<String>,
}

/// Ответ от ely.by auth сервера
#[derive(Debug, Deserialize)]
#[serde(rename_all = "camelCase")]
struct AuthResponse {
    access_token: String,
    selected_profile: Option<SelectedProfile>,
}

#[derive(Debug, Deserialize)]
struct SelectedProfile {
    id: String,
    name: String,
}

/// Ответ ошибки от ely.by
#[derive(Debug, Deserialize)]
#[serde(rename_all = "camelCase")]
struct AuthError {
    error_message: Option<String>,
    error: Option<String>,
}

/// Авторизация через ely.by (Yggdrasil API)
#[tauri::command]
pub async fn login_elyby(username: String, password: String) -> Result<Account, String> {
    let client = reqwest::Client::new();

    let payload = serde_json::json!({
        "agent": {
            "name": "Minecraft",
            "version": 1
        },
        "username": username,
        "password": password,
        "clientToken": uuid::Uuid::new_v4().to_string(),
        "requestUser": true
    });

    let resp = client
        .post("https://authserver.ely.by/auth/authenticate")
        .json(&payload)
        .send()
        .await
        .map_err(|e| format!("Ошибка соединения: {}", e))?;

    let status = resp.status();
    let body = resp.text().await.map_err(|e| format!("Ошибка чтения ответа: {}", e))?;

    if !status.is_success() {
        if let Ok(err) = serde_json::from_str::<AuthError>(&body) {
            return Err(err.error_message.unwrap_or_else(|| err.error.unwrap_or("Неизвестная ошибка".into())));
        }
        return Err(format!("Ошибка авторизации ({})", status));
    }

    let auth: AuthResponse =
        serde_json::from_str(&body).map_err(|e| format!("Ошибка парсинга: {}", e))?;

    let profile = auth
        .selected_profile
        .ok_or("Нет профиля Minecraft на этом аккаунте")?;

    let skin_url = format!("https://skinsystem.ely.by/skins/{}.png", profile.name);

    let account = Account {
        username: profile.name,
        uuid: profile.id,
        access_token: auth.access_token,
        skin_url: Some(skin_url),
    };

    // Сохраняем сессию на диск
    save_session(&account);

    Ok(account)
}

/// Проверяем сохранённую сессию
#[tauri::command]
pub fn get_saved_session() -> Option<Account> {
    let path = session_path();
    if path.exists() {
        let data = fs::read_to_string(&path).ok()?;
        serde_json::from_str(&data).ok()
    } else {
        None
    }
}

/// Выход из аккаунта
#[tauri::command]
pub fn logout() -> Result<(), String> {
    let path = session_path();
    if path.exists() {
        fs::remove_file(&path).map_err(|e| format!("Ошибка: {}", e))?;
    }
    Ok(())
}

/// Получить URL скина
#[tauri::command]
pub fn get_skin_url(username: String) -> String {
    format!("https://skinsystem.ely.by/skins/{}.png", username)
}

/// Получить URL кейпа
#[tauri::command]
pub fn get_cape_url(username: String) -> String {
    format!("https://skinsystem.ely.by/cloaks/{}.png", username)
}

fn session_path() -> PathBuf {
    let base = dirs::data_dir().unwrap_or_else(|| PathBuf::from("."));
    let dir = base.join("wenzlauncher");
    fs::create_dir_all(&dir).ok();
    dir.join("session.json")
}

fn save_session(account: &Account) {
    let path = session_path();
    if let Ok(json) = serde_json::to_string_pretty(account) {
        fs::write(&path, json).ok();
    }
}
