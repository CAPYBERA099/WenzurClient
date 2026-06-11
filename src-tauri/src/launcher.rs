use serde::{Deserialize, Serialize};
use std::path::PathBuf;

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct GameVersion {
    pub id: String,
    pub version_type: String, // "release", "snapshot"
    pub url: String,
    pub release_date: String,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct LauncherConfig {
    pub ram_mb: u32,
    pub java_path: Option<String>,
    pub game_dir: String,
    pub resolution_width: u32,
    pub resolution_height: u32,
    pub jvm_args: String,
    pub username: String,
}

impl Default for LauncherConfig {
    fn default() -> Self {
        let game_dir = get_default_game_dir()
            .to_string_lossy()
            .to_string();

        Self {
            ram_mb: 4096,
            java_path: None,
            game_dir,
            resolution_width: 1920,
            resolution_height: 1080,
            jvm_args: "-XX:+UseG1GC -XX:+UnlockExperimentalVMOptions".to_string(),
            username: "WenzPlayer".to_string(),
        }
    }
}

/// Возвращает список доступных версий (заглушка — потом подключим Mojang API)
pub fn get_available_versions() -> Vec<GameVersion> {
    vec![
        GameVersion {
            id: "1.21.4".to_string(),
            version_type: "release".to_string(),
            url: "https://piston-meta.mojang.com/v1/packages/1.21.4.json".to_string(),
            release_date: "2024-12-03".to_string(),
        },
        GameVersion {
            id: "1.20.4".to_string(),
            version_type: "release".to_string(),
            url: "https://piston-meta.mojang.com/v1/packages/1.20.4.json".to_string(),
            release_date: "2024-01-25".to_string(),
        },
        GameVersion {
            id: "1.8.9".to_string(),
            version_type: "release".to_string(),
            url: "https://piston-meta.mojang.com/v1/packages/1.8.9.json".to_string(),
            release_date: "2015-12-09".to_string(),
        },
    ]
}

/// Запуск игры
pub async fn launch(version: String, username: String) -> Result<String, Box<dyn std::error::Error>> {
    // TODO: Реализовать:
    // 1. Скачать version manifest с Mojang API
    // 2. Скачать клиент (.jar) и ассеты
    // 3. Скачать библиотеки
    // 4. Сформировать classpath
    // 5. Запустить java с правильными аргументами

    let config = LauncherConfig::default();
    let java = detect_java().unwrap_or_else(|| "java".to_string());

    println!("🎮 Запуск Minecraft {} для {}", version, username);
    println!("   Java: {}", java);
    println!("   RAM: {} MB", config.ram_mb);
    println!("   Директория: {}", config.game_dir);

    Ok(format!("Minecraft {} запущен для {}", version, username))
}

/// Определение Java на системе
pub fn detect_java() -> Option<String> {
    // Проверяем стандартные пути
    let paths = if cfg!(target_os = "windows") {
        vec![
            "C:\\Program Files\\Java\\jdk-21\\bin\\java.exe",
            "C:\\Program Files\\Eclipse Adoptium\\jdk-21\\bin\\java.exe",
            "C:\\Program Files\\Java\\jdk-17\\bin\\java.exe",
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

    // Пробуем найти через PATH
    std::process::Command::new("java")
        .arg("-version")
        .output()
        .ok()
        .map(|_| "java".to_string())
}

fn get_default_game_dir() -> PathBuf {
    let base = dirs::data_dir().unwrap_or_else(|| PathBuf::from("."));
    base.join("wenzlauncher").join("minecraft")
}
