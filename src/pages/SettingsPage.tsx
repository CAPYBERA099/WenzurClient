import { Monitor, Cpu, HardDrive, FolderOpen, Globe, Palette } from "lucide-react";

interface SettingGroup {
  title: string;
  icon: React.ElementType;
  settings: Setting[];
}

interface Setting {
  label: string;
  description: string;
  type: "slider" | "select" | "toggle" | "input";
  value: string;
}

const settingGroups: SettingGroup[] = [
  {
    title: "Производительность",
    icon: Cpu,
    settings: [
      { label: "Выделенная RAM", description: "Оперативная память для Minecraft", type: "slider", value: "4096 MB" },
      { label: "Аргументы JVM", description: "Дополнительные параметры Java", type: "input", value: "-XX:+UseG1GC -XX:+UnlockExperimentalVMOptions" },
      { label: "Разрешение окна", description: "Размер окна при запуске", type: "select", value: "1920x1080" },
    ],
  },
  {
    title: "Java",
    icon: Monitor,
    settings: [
      { label: "Путь к Java", description: "Авто-определение или ручной путь", type: "input", value: "Авто (Java 21.0.2)" },
      { label: "Версия Java", description: "Предпочитаемая версия", type: "select", value: "Java 21 (рекомендуется)" },
    ],
  },
  {
    title: "Хранилище",
    icon: HardDrive,
    settings: [
      { label: "Папка игры", description: "Где хранятся файлы Minecraft", type: "input", value: "~/.wenzlauncher/minecraft" },
      { label: "Папка сборок", description: "Где хранятся сборки", type: "input", value: "~/.wenzlauncher/instances" },
    ],
  },
  {
    title: "Интерфейс",
    icon: Palette,
    settings: [
      { label: "Тема", description: "Оформление лаунчера", type: "select", value: "Тёмная" },
      { label: "Язык", description: "Язык интерфейса", type: "select", value: "Русский" },
      { label: "Анимации", description: "Плавные переходы и эффекты", type: "toggle", value: "on" },
    ],
  },
];

export function SettingsPage() {
  return (
    <div className="animate-fade-in space-y-6">
      <div>
        <h1 className="text-2xl font-bold text-wenz-text">Настройки</h1>
        <p className="mt-1 text-sm text-wenz-muted">
          Настрой лаунчер под себя
        </p>
      </div>

      <div className="space-y-6">
        {settingGroups.map((group) => (
          <div
            key={group.title}
            className="rounded-xl border border-wenz-border bg-wenz-card"
          >
            <div className="flex items-center gap-3 border-b border-wenz-border px-5 py-4">
              <group.icon className="h-4 w-4 text-wenz-accent" />
              <h2 className="text-sm font-semibold text-wenz-text">
                {group.title}
              </h2>
            </div>
            <div className="divide-y divide-wenz-border">
              {group.settings.map((setting) => (
                <div
                  key={setting.label}
                  className="flex items-center justify-between px-5 py-4"
                >
                  <div>
                    <p className="text-sm font-medium text-wenz-text">
                      {setting.label}
                    </p>
                    <p className="mt-0.5 text-xs text-wenz-muted">
                      {setting.description}
                    </p>
                  </div>
                  <div className="flex-shrink-0">
                    {setting.type === "toggle" ? (
                      <button className="h-6 w-11 rounded-full bg-wenz-accent p-0.5 transition-colors">
                        <div className="h-5 w-5 translate-x-5 rounded-full bg-white shadow transition-transform" />
                      </button>
                    ) : setting.type === "slider" ? (
                      <div className="flex items-center gap-3">
                        <input
                          type="range"
                          min="1024"
                          max="16384"
                          defaultValue="4096"
                          step="512"
                          className="h-1 w-32 cursor-pointer appearance-none rounded-full bg-wenz-border accent-wenz-accent"
                        />
                        <span className="w-20 text-right font-mono text-xs text-wenz-accent">
                          {setting.value}
                        </span>
                      </div>
                    ) : setting.type === "select" ? (
                      <select className="rounded-lg border border-wenz-border bg-wenz-bg px-3 py-1.5 text-xs text-wenz-text outline-none focus:border-wenz-accent/50">
                        <option>{setting.value}</option>
                      </select>
                    ) : (
                      <input
                        type="text"
                        defaultValue={setting.value}
                        className="w-72 rounded-lg border border-wenz-border bg-wenz-bg px-3 py-1.5 text-xs text-wenz-text outline-none transition-colors focus:border-wenz-accent/50"
                      />
                    )}
                  </div>
                </div>
              ))}
            </div>
          </div>
        ))}
      </div>

      {/* Save */}
      <div className="flex justify-end gap-3">
        <button className="rounded-lg border border-wenz-border px-4 py-2 text-sm text-wenz-muted transition-colors hover:border-wenz-accent/30 hover:text-wenz-text">
          Сбросить
        </button>
        <button className="rounded-lg bg-wenz-accent px-6 py-2 text-sm font-semibold text-wenz-bg transition-colors hover:bg-wenz-accent-hover">
          Сохранить
        </button>
      </div>
    </div>
  );
}
