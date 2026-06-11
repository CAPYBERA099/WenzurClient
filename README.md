# WenzGuard — Монитор безопасности сессий v2.0

Следит за файлами сессий и токенами **всех** установленных программ на Windows. Если стилер пытается украсть ваши данные — WenzGuard немедленно предупредит.

## Что отслеживается

### 🌐 Браузеры
Chrome, Edge, Firefox, Opera, Opera GX, Brave, Yandex Browser, Vivaldi
- Cookies, Login Data, Local State (мастер-ключ), сессии

### 💬 Мессенджеры
- **Telegram Desktop** — tdata (key_data, map, сессии)
- **Discord** / Canary / PTB — токены (LevelDB)
- **WhatsApp Desktop** — профиль, базы данных
- **Signal** — config.json, db.sqlite
- **Viber** — viber.db, config
- **Skype** — Local Storage
- **Element (Matrix)** — Local Storage
- **ICQ** — auth_token

### 🎮 Игровые платформы
- **Steam** — ssfn файлы, loginusers.vdf, config.vdf
- **Epic Games** — настройки аккаунта
- **Minecraft** — launcher_accounts.json
- **Riot Games** (Valorant/LoL) — приватные настройки
- **Battle.net** — config
- **Ubisoft Connect** — user.dat
- **EA App** — user config

### 💰 Крипто-кошельки
- **Exodus** — seed.seco, passphrase.json
- **Atomic Wallet** — LevelDB
- **Electrum** — файлы кошельков
- **MetaMask** (расширение Chrome) — LevelDB

### 🔒 VPN
- **NordVPN**, **ProtonVPN** — user.config
- **OpenVPN** — .ovpn профили

### 📁 Другие
- **FileZilla** — сохранённые FTP серверы и пароли
- **WinSCP** — SSH/SCP пароли
- **Outlook** / **Thunderbird** — почтовые базы
- **Authy** — 2FA токены
- **OBS Studio** — stream key
- **Git** — .git-credentials

## Как работает

1. **Обнаружение** — автоматически находит все профили
2. **Мониторинг** — `ReadDirectoryChangesW` следит за изменениями в реальном времени
3. **Сканирование** — каждые 30 сек проверяет процессы на стилеры
4. **Оповещение** — звук + всплывающее окно + лог

## Сборка

```cmd
cmake -B build
cmake --build build --config Release
```

## Запуск

```cmd
WenzGuard.exe              :: полный мониторинг
WenzGuard.exe --silent     :: без всплывающих окон
WenzGuard.exe --scan-only  :: только сканирование процессов
```

Автозапуск: `Win+R` → `shell:startup` → ярлык на `WenzGuard.exe`
