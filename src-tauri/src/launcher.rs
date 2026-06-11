use serde::{Deserialize, Serialize};
use sha1::{Digest, Sha1};
use std::fs;
use std::path::PathBuf;
use std::process::Command;
use tauri::Emitter;

// ==================== ТИПЫ ====================

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct GameVersion {
    pub id: String,
    #[serde(rename = "type")]
    pub version_type: String,
    pub url: String,
    #[serde(rename = "releaseTime", default)]
    pub release_time: String,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct LauncherConfig {
    pub ram_mb: u32,
    pub java_path: String,
    pub game_dir: String,
    pub resolution_width: u32,
    pub resolution_height: u32,
    pub jvm_args: String,
}

impl Default for LauncherConfig {
    fn default() -> Self {
        let game_dir = get_game_dir().to_string_lossy().to_string();
        Self {
            ram_mb: 4096,
            java_path: detect_java().unwrap_or_else(|| "java".into()),
            game_dir,
            resolution_width: 1920,
            resolution_height: 1080,
            jvm_args: "-XX:+UseG1GC -XX:+UnlockExperimentalVMOptions -XX:G1NewSizePercent=20 -XX:G1ReservePercent=20".into(),
        }
    }
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct DownloadProgress {
    pub stage: String,
    pub progress: f64,
    pub detail: String,
}

// ==================== Mojang API structs ====================

#[derive(Debug, Deserialize)]
struct VersionManifest {
    versions: Vec<ManifestVersion>,
}

#[derive(Debug, Deserialize)]
struct ManifestVersion {
    id: String,
    #[serde(rename = "type")]
    version_type: String,
    url: String,
    #[serde(rename = "releaseTime", default)]
    release_time: String,
}

#[derive(Debug, Deserialize)]
struct VersionMeta {
    #[allow(dead_code)]
    id: String,
    downloads: Downloads,
    libraries: Vec<Library>,
    #[serde(rename = "mainClass")]
    main_class: String,
    #[serde(rename = "minecraftArguments", default)]
    minecraft_arguments: Option<String>,
    arguments: Option<Arguments>,
    #[serde(rename = "assetIndex")]
    asset_index: Option<AssetIndex>,
    assets: Option<String>,
}

#[derive(Debug, Deserialize)]
struct Downloads {
    client: DownloadInfo,
}

#[derive(Debug, Deserialize)]
struct DownloadInfo {
    url: String,
    sha1: String,
    #[allow(dead_code)]
    size: u64,
}

#[derive(Debug, Deserialize)]
struct Library {
    name: String,
    downloads: Option<LibraryDownloads>,
    rules: Option<Vec<Rule>>,
}

#[derive(Debug, Deserialize)]
struct LibraryDownloads {
    artifact: Option<LibraryArtifact>,
}

#[derive(Debug, Deserialize)]
struct LibraryArtifact {
    path: String,
    url: String,
    sha1: String,
    #[allow(dead_code)]
    size: u64,
}

#[derive(Debug, Deserialize)]
struct Rule {
    action: String,
    os: Option<OsRule>,
}

#[derive(Debug, Deserialize)]
struct OsRule {
    name: Option<String>,
}

#[derive(Debug, Deserialize)]
struct Arguments {
    game: Option<Vec<serde_json::Value>>,
    #[allow(dead_code)]
    jvm: Option<Vec<serde_json::Value>>,
}

#[derive(Debug, Deserialize)]
struct AssetIndex {
    id: String,
    url: String,
    #[allow(dead_code)]
    #[serde(rename = "totalSize", default)]
    total_size: u64,
}

#[derive(Debug, Deserialize)]
struct AssetIndexFile {
    objects: std::collections::HashMap<String, AssetObject>,
}

#[derive(Debug, Deserialize)]
struct AssetObject {
    hash: String,
    #[allow(dead_code)]
    size: u64,
}

// ==================== КОМАНДЫ TAURI ====================

/// Получить список версий с Mojang API
#[tauri::command]
pub async fn fetch_versions() -> Result<Vec<GameVersion>, String> {
    let client = reqwest::Client::new();
    let manifest: VersionManifest = client
        .get("https://piston-meta.mojang.com/mc/game/version_manifest_v2.json")
        .send()
        .await
        .map_err(|e| format!("Ошибка загрузки: {}", e))?
        .json()
        .await
        .map_err(|e| format!("Ошибка парсинга: {}", e))?;

    let versions: Vec<GameVersion> = manifest
        .versions
        .into_iter()
        .filter(|v| v.version_type == "release" || v.version_type == "snapshot")
        .take(100)
        .map(|v| GameVersion {
            id: v.id,
            version_type: v.version_type,
            url: v.url,
            release_time: v.release_time,
        })
        .collect();

    Ok(versions)
}

/// Получить список установленных версий
#[tauri::command]
pub fn get_installed_versions() -> Vec<String> {
    let versions_dir = get_game_dir().join("versions");
    if !versions_dir.exists() {
        return vec![];
    }

    fs::read_dir(&versions_dir)
        .map(|entries| {
            entries
                .filter_map(|e| e.ok())
                .filter(|e| e.path().is_dir())
                .filter(|e| {
                    let jar = e
                        .path()
                        .join(format!("{}.jar", e.file_name().to_string_lossy()));
                    jar.exists()
                })
                .map(|e| e.file_name().to_string_lossy().to_string())
                .collect()
        })
        .unwrap_or_default()
}

/// Скачать и установить версию
#[tauri::command]
pub async fn install_version(
    version_id: String,
    version_url: String,
    app_handle: tauri::AppHandle,
) -> Result<String, String> {
    let client = reqwest::Client::new();
    let game_dir = get_game_dir();
    let version_dir = game_dir.join("versions").join(&version_id);
    fs::create_dir_all(&version_dir)
        .map_err(|e| format!("Ошибка создания директории: {}", e))?;

    // 1. Скачиваем метаданные версии
    emit_progress(&app_handle, "Загрузка метаданных", 0.0, &version_id);

    let meta_text = client
        .get(&version_url)
        .send()
        .await
        .map_err(|e| format!("Ошибка загрузки метаданных: {}", e))?
        .text()
        .await
        .map_err(|e| format!("Ошибка чтения: {}", e))?;

    let meta: VersionMeta = serde_json::from_str(&meta_text)
        .map_err(|e| format!("Ошибка парсинга метаданных: {}", e))?;

    // Сохраняем JSON метаданных
    fs::write(
        version_dir.join(format!("{}.json", version_id)),
        &meta_text,
    )
    .map_err(|e| format!("Ошибка сохранения: {}", e))?;

    // 2. Скачиваем клиент .jar
    emit_progress(
        &app_handle,
        "Загрузка клиента",
        0.05,
        &format!("{}.jar", version_id),
    );

    let jar_path = version_dir.join(format!("{}.jar", version_id));
    download_file(&client, &meta.downloads.client.url, &jar_path).await?;

    // 3. Скачиваем библиотеки
    let libs_dir = game_dir.join("libraries");
    let total_libs = meta.libraries.len();

    for (i, lib) in meta.libraries.iter().enumerate() {
        if !should_use_library(lib) {
            continue;
        }

        let progress = 0.1 + 0.6 * (i as f64 / total_libs as f64);
        emit_progress(&app_handle, "Загрузка библиотек", progress, &lib.name);

        if let Some(downloads) = &lib.downloads {
            if let Some(artifact) = &downloads.artifact {
                let lib_path = libs_dir.join(&artifact.path);
                if !lib_path.exists() || !verify_sha1(&lib_path, &artifact.sha1) {
                    if let Some(parent) = lib_path.parent() {
                        fs::create_dir_all(parent).ok();
                    }
                    // Пропускаем ошибки отдельных библиотек
                    let _ = download_file(&client, &artifact.url, &lib_path).await;
                }
            }
        }
    }

    // 4. Скачиваем ассеты
    if let Some(asset_index) = &meta.asset_index {
        emit_progress(&app_handle, "Загрузка ассетов", 0.7, &asset_index.id);

        let assets_dir = game_dir.join("assets");
        let indexes_dir = assets_dir.join("indexes");
        fs::create_dir_all(&indexes_dir).ok();

        let index_path = indexes_dir.join(format!("{}.json", asset_index.id));
        let index_text = client
            .get(&asset_index.url)
            .send()
            .await
            .map_err(|e| format!("Ошибка загрузки asset index: {}", e))?
            .text()
            .await
            .map_err(|e| e.to_string())?;

        fs::write(&index_path, &index_text).ok();

        if let Ok(asset_index_file) = serde_json::from_str::<AssetIndexFile>(&index_text) {
            let objects_dir = assets_dir.join("objects");
            let total_assets = asset_index_file.objects.len();
            let mut downloaded = 0;

            for (_name, obj) in &asset_index_file.objects {
                let prefix = &obj.hash[..2];
                let obj_dir = objects_dir.join(prefix);
                let obj_path = obj_dir.join(&obj.hash);

                if !obj_path.exists() {
                    fs::create_dir_all(&obj_dir).ok();
                    let url = format!(
                        "https://resources.download.minecraft.net/{}/{}",
                        prefix, obj.hash
                    );
                    let _ = download_file(&client, &url, &obj_path).await;
                }

                downloaded += 1;
                if downloaded % 100 == 0 {
                    let progress = 0.7 + 0.25 * (downloaded as f64 / total_assets as f64);
                    emit_progress(
                        &app_handle,
                        "Загрузка ассетов",
                        progress,
                        &format!("{}/{}", downloaded, total_assets),
                    );
                }
            }
        }
    }

    emit_progress(
        &app_handle,
        "Готово",
        1.0,
        &format!("Версия {} установлена!", version_id),
    );

    Ok(format!("Версия {} успешно установлена", version_id))
}

/// Удалить установленную версию
#[tauri::command]
pub fn delete_version(version_id: String) -> Result<(), String> {
    let version_dir = get_game_dir().join("versions").join(&version_id);
    if version_dir.exists() {
        fs::remove_dir_all(&version_dir)
            .map_err(|e| format!("Ошибка удаления: {}", e))?;
    }
    Ok(())
}

/// Запустить Minecraft
#[tauri::command]
pub async fn launch_game(
    version_id: String,
    username: String,
    uuid: String,
    access_token: String,
    ram_mb: u32,
    java_path: String,
    app_handle: tauri::AppHandle,
) -> Result<String, String> {
    let game_dir = get_game_dir();
    let version_dir = game_dir.join("versions").join(&version_id);
    let jar_path = version_dir.join(format!("{}.jar", version_id));

    if !jar_path.exists() {
        return Err("Версия не установлена! Сначала скачайте её.".into());
    }

    // Читаем метаданные версии
    let meta_path = version_dir.join(format!("{}.json", version_id));
    let meta_text = fs::read_to_string(&meta_path)
        .map_err(|_| "Метаданные версии не найдены".to_string())?;
    let meta: VersionMeta =
        serde_json::from_str(&meta_text).map_err(|e| format!("Ошибка парсинга: {}", e))?;

    // Собираем classpath
    let libs_dir = game_dir.join("libraries");
    let separator = if cfg!(target_os = "windows") {
        ";"
    } else {
        ":"
    };
    let mut classpath_parts: Vec<String> = Vec::new();

    for lib in &meta.libraries {
        if !should_use_library(lib) {
            continue;
        }
        if let Some(downloads) = &lib.downloads {
            if let Some(artifact) = &downloads.artifact {
                let lib_path = libs_dir.join(&artifact.path);
                if lib_path.exists() {
                    classpath_parts.push(lib_path.to_string_lossy().to_string());
                }
            }
        }
    }
    classpath_parts.push(jar_path.to_string_lossy().to_string());
    let classpath = classpath_parts.join(separator);

    // Assets
    let assets_dir = game_dir.join("assets");
    let asset_id = meta.assets.unwrap_or_else(|| "legacy".into());

    // Natives dir
    let natives_dir = version_dir.join("natives");
    fs::create_dir_all(&natives_dir).ok();

    // JVM аргументы
    let java_cmd = if java_path.is_empty() {
        "java".to_string()
    } else {
        java_path
    };

    let mut args: Vec<String> = vec![
        // ely.by authserver
        "-Dminecraft.api.auth.host=https://authserver.ely.by/auth".into(),
        "-Dminecraft.api.account.host=https://authserver.ely.by/account".into(),
        "-Dminecraft.api.session.host=https://authserver.ely.by/session".into(),
        "-Dminecraft.api.services.host=https://authserver.ely.by".into(),
        // Память
        format!("-Xmx{}M", ram_mb),
        format!("-Xms{}M", ram_mb / 2),
        // Оптимизация
        "-XX:+UseG1GC".into(),
        "-XX:+UnlockExperimentalVMOptions".into(),
        "-XX:G1NewSizePercent=20".into(),
        "-XX:G1ReservePercent=20".into(),
        // Paths
        format!(
            "-Djava.library.path={}",
            natives_dir.to_string_lossy()
        ),
        "-cp".into(),
        classpath,
        // Main class
        meta.main_class.clone(),
    ];

    // Game аргументы
    if let Some(mc_args) = &meta.minecraft_arguments {
        // Старый формат (1.12 и ниже)
        let game_args = mc_args
            .replace("${auth_player_name}", &username)
            .replace("${version_name}", &version_id)
            .replace("${game_directory}", &game_dir.to_string_lossy())
            .replace("${assets_root}", &assets_dir.to_string_lossy())
            .replace("${assets_index_name}", &asset_id)
            .replace("${auth_uuid}", &uuid)
            .replace("${auth_access_token}", &access_token)
            .replace("${user_type}", "mojang")
            .replace("${version_type}", "WenzLauncher")
            .replace("${user_properties}", "{}");

        args.extend(game_args.split_whitespace().map(|s| s.to_string()));
    } else if let Some(arguments) = &meta.arguments {
        // Новый формат (1.13+)
        if let Some(game_args) = &arguments.game {
            for arg in game_args {
                if let Some(s) = arg.as_str() {
                    let replaced = s
                        .replace("${auth_player_name}", &username)
                        .replace("${version_name}", &version_id)
                        .replace("${game_directory}", &game_dir.to_string_lossy())
                        .replace("${assets_root}", &assets_dir.to_string_lossy())
                        .replace("${assets_index_name}", &asset_id)
                        .replace("${auth_uuid}", &uuid)
                        .replace("${auth_access_token}", &access_token)
                        .replace("${user_type}", "mojang")
                        .replace("${version_type}", "WenzLauncher")
                        .replace("${clientid}", "")
                        .replace("${auth_xuid}", "");
                    args.push(replaced);
                }
            }
        }
    }

    emit_progress(
        &app_handle,
        "Запуск",
        1.0,
        &format!("Minecraft {} для {}", version_id, username),
    );

    // Запускаем процесс
    let mut cmd = Command::new(&java_cmd);
    cmd.args(&args);
    cmd.current_dir(&game_dir);

    match cmd.spawn() {
        Ok(_) => Ok(format!("Minecraft {} запущен для {}", version_id, username)),
        Err(e) => Err(format!(
            "Ошибка запуска: {}. Проверьте путь к Java: {}",
            e, java_cmd
        )),
    }
}

/// Получить конфиг лаунчера
#[tauri::command]
pub fn get_config() -> LauncherConfig {
    let config_path = get_game_dir().join("launcher_config.json");
    if config_path.exists() {
        if let Ok(data) = fs::read_to_string(&config_path) {
            if let Ok(config) = serde_json::from_str(&data) {
                return config;
            }
        }
    }
    LauncherConfig::default()
}

/// Сохранить конфиг лаунчера
#[tauri::command]
pub fn save_config(config: LauncherConfig) -> Result<(), String> {
    let dir = get_game_dir();
    fs::create_dir_all(&dir).ok();
    let config_path = dir.join("launcher_config.json");
    let json = serde_json::to_string_pretty(&config).map_err(|e| e.to_string())?;
    fs::write(&config_path, json).map_err(|e| format!("Ошибка сохранения: {}", e))?;
    Ok(())
}

/// Определение Java
#[tauri::command]
pub fn get_java_info() -> String {
    detect_java().unwrap_or_else(|| "Java не найдена".into())
}

// ==================== УТИЛИТЫ ====================

fn emit_progress(handle: &tauri::AppHandle, stage: &str, progress: f64, detail: &str) {
    let _ = handle.emit(
        "download-progress",
        DownloadProgress {
            stage: stage.into(),
            progress,
            detail: detail.into(),
        },
    );
}

async fn download_file(
    client: &reqwest::Client,
    url: &str,
    path: &PathBuf,
) -> Result<(), String> {
    if let Some(parent) = path.parent() {
        fs::create_dir_all(parent).ok();
    }

    let resp = client
        .get(url)
        .send()
        .await
        .map_err(|e| format!("Ошибка загрузки {}: {}", url, e))?;

    if !resp.status().is_success() {
        return Err(format!("HTTP {} для {}", resp.status(), url));
    }

    let bytes = resp
        .bytes()
        .await
        .map_err(|e| format!("Ошибка чтения: {}", e))?;

    fs::write(path, &bytes).map_err(|e| format!("Ошибка записи: {}", e))?;
    Ok(())
}

fn verify_sha1(path: &PathBuf, expected: &str) -> bool {
    if let Ok(data) = fs::read(path) {
        let mut hasher = Sha1::new();
        hasher.update(&data);
        let result = format!("{:x}", hasher.finalize());
        result == expected
    } else {
        false
    }
}

fn should_use_library(lib: &Library) -> bool {
    if let Some(rules) = &lib.rules {
        let current_os = if cfg!(target_os = "windows") {
            "windows"
        } else if cfg!(target_os = "macos") {
            "osx"
        } else {
            "linux"
        };

        let mut dominated_allow = false;
        let mut dominated_disallow = false;

        for rule in rules {
            if let Some(os) = &rule.os {
                if let Some(name) = &os.name {
                    if name == current_os {
                        if rule.action == "allow" {
                            dominated_allow = true;
                        } else {
                            dominated_disallow = true;
                        }
                    }
                }
            } else if rule.action == "allow" {
                dominated_allow = true;
            } else {
                dominated_disallow = true;
            }
        }

        if dominated_disallow {
            return false;
        }
        return dominated_allow;
    }
    true
}

pub fn detect_java() -> Option<String> {
    let paths = if cfg!(target_os = "windows") {
        vec![
            "C:\\Program Files\\Java\\jdk-21\\bin\\java.exe",
            "C:\\Program Files\\Eclipse Adoptium\\jdk-21\\bin\\java.exe",
            "C:\\Program Files\\Java\\jdk-17\\bin\\java.exe",
            "C:\\Program Files\\Eclipse Adoptium\\jdk-17\\bin\\java.exe",
            "C:\\Program Files (x86)\\Java\\jre-1.8\\bin\\java.exe",
        ]
    } else if cfg!(target_os = "macos") {
        vec![
            "/usr/bin/java",
            "/Library/Java/JavaVirtualMachines/jdk-21.jdk/Contents/Home/bin/java",
        ]
    } else {
        vec![
            "/usr/bin/java",
            "/usr/lib/jvm/java-21/bin/java",
            "/usr/lib/jvm/java-17/bin/java",
        ]
    };

    for path in paths {
        if std::path::Path::new(path).exists() {
            return Some(path.to_string());
        }
    }

    Command::new("java")
        .arg("-version")
        .output()
        .ok()
        .map(|_| "java".to_string())
}

pub fn get_game_dir() -> PathBuf {
    let base = dirs::data_dir().unwrap_or_else(|| PathBuf::from("."));
    base.join("wenzlauncher").join("minecraft")
}
