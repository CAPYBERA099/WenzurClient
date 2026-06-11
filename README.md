# 🎮 WenzLauncher

Кастомный Minecraft лаунчер в стиле Lunar Client — построен на **Tauri 2** (Rust) + **React** + **TypeScript** + **Tailwind CSS**.

![WenzLauncher](https://img.shields.io/badge/Tauri-2.0-blue?style=flat-square) ![React](https://img.shields.io/badge/React-18-61dafb?style=flat-square) ![License](https://img.shields.io/badge/License-MIT-green?style=flat-square)

## ✨ Возможности

- 🚀 **Быстрый запуск** — нативное приложение на Rust, минимальное потребление ресурсов
- 📦 **Управление сборками** — создавай и переключай игровые конфигурации
- 🧩 **Менеджер модов** — устанавливай моды в один клик
- 👕 **Скины и косметика** — управляй внешним видом персонажа
- ⚙️ **Гибкие настройки** — RAM, JVM аргументы, Java, разрешение
- 🎨 **Современный UI** — тёмная тема в стиле Lunar Client

## 🛠 Стек

| Слой | Технология |
|------|------------|
| Фреймворк | [Tauri 2](https://v2.tauri.app/) |
| Backend | Rust |
| Frontend | React 18 + TypeScript |
| Стили | Tailwind CSS |
| Сборка | Vite |
| Иконки | Lucide React |

## 📦 Установка

### Зависимости

1. [Node.js](https://nodejs.org/) >= 18
2. [Rust](https://www.rust-lang.org/tools/install) (через rustup)
3. [Tauri Prerequisites](https://v2.tauri.app/start/prerequisites/)

### Запуск в режиме разработки

```bash
# Установка зависимостей
npm install

# Запуск Tauri + Vite
npm run tauri dev
```

### Сборка

```bash
# Создание установщика
npm run tauri build
```

Готовый установщик будет в `src-tauri/target/release/bundle/`.

## 📁 Структура проекта

```
WenzLauncher/
├── src/                    # React frontend
│   ├── components/         # UI компоненты
│   │   ├── Sidebar.tsx     # Боковая навигация
│   │   └── TitleBar.tsx    # Кастомный заголовок окна
│   ├── pages/              # Страницы
│   │   ├── HomePage.tsx    # Главная + запуск
│   │   ├── InstancesPage.tsx # Управление сборками
│   │   ├── ModsPage.tsx    # Менеджер модов
│   │   ├── SkinsPage.tsx   # Скины и косметика
│   │   └── SettingsPage.tsx # Настройки
│   ├── lib/                # Утилиты
│   ├── styles/             # CSS
│   ├── App.tsx             # Роутинг
│   └── main.tsx            # Точка входа
├── src-tauri/              # Rust backend
│   ├── src/
│   │   ├── main.rs         # Точка входа Tauri
│   │   ├── lib.rs          # Команды Tauri
│   │   └── launcher.rs     # Логика лаунчера
│   ├── Cargo.toml
│   └── tauri.conf.json     # Конфиг Tauri
├── package.json
├── tailwind.config.js
├── vite.config.ts
└── README.md
```

## 🗺 Роадмап

- [x] UI лаунчера (главная, сборки, моды, скины, настройки)
- [x] Tauri backend с командами
- [ ] Авторизация через Microsoft (OAuth2)
- [ ] Скачивание версий через Mojang API
- [ ] Запуск Minecraft с правильным classpath
- [ ] Скачивание и установка модов (Modrinth API)
- [ ] Скачивание Java автоматически
- [ ] Система обновлений лаунчера
- [ ] Мультиплеер — список серверов

## 📄 Лицензия

MIT — делай что хочешь.

---

*Сделано с 💚 для WenzPlay*
