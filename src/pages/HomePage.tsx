import { Play, Download, Cpu, HardDrive, Wifi, Gamepad2, Zap, Star } from "lucide-react";

const recentVersions = [
  { version: "1.21.4", type: "Release", date: "2024-12-03", active: true },
  { version: "1.20.4", type: "Release", date: "2024-01-25", active: false },
  { version: "1.8.9", type: "Release", date: "2015-12-09", active: false },
];

const quickStats = [
  { icon: Cpu, label: "Java", value: "21.0.2", color: "text-wenz-accent" },
  { icon: HardDrive, label: "Диск", value: "2.4 GB", color: "text-wenz-purple" },
  { icon: Wifi, label: "Пинг", value: "24 ms", color: "text-wenz-success" },
  { icon: Gamepad2, label: "Сессии", value: "142", color: "text-wenz-warning" },
];

const features = [
  { icon: Zap, title: "Оптимизация FPS", desc: "Встроенные моды для максимальной производительности" },
  { icon: Star, title: "Космётики", desc: "Уникальные крылья, шапки и анимации" },
  { icon: Gamepad2, title: "Анти-чит", desc: "Собственная система античита для честной игры" },
];

export function HomePage() {
  return (
    <div className="animate-fade-in space-y-6">
      {/* Hero / Play Section */}
      <div className="relative overflow-hidden rounded-xl border border-wenz-border bg-gradient-to-br from-wenz-card via-wenz-card to-wenz-accent/5 p-8">
        <div className="absolute -right-20 -top-20 h-64 w-64 rounded-full bg-wenz-accent/5 blur-3xl" />
        <div className="absolute -bottom-10 -left-10 h-40 w-40 rounded-full bg-wenz-purple/5 blur-3xl" />

        <div className="relative flex items-center justify-between">
          <div>
            <div className="flex items-center gap-2">
              <span className="rounded bg-wenz-accent/20 px-2 py-0.5 text-xs font-semibold text-wenz-accent">
                ГОТОВО К ЗАПУСКУ
              </span>
            </div>
            <h1 className="mt-3 text-4xl font-extrabold tracking-tight text-wenz-text">
              Minecraft <span className="text-wenz-accent">1.21.4</span>
            </h1>
            <p className="mt-2 max-w-md text-sm text-wenz-muted">
              Последняя версия с оптимизацией WenzLauncher. Включены моды для
              FPS, улучшенные шейдеры и античит.
            </p>
          </div>

          <button className="animate-pulse-glow group flex h-20 w-20 items-center justify-center rounded-2xl bg-wenz-accent transition-all hover:scale-105 hover:bg-wenz-accent-hover">
            <Play className="h-8 w-8 text-wenz-bg transition-transform group-hover:scale-110" fill="currentColor" />
          </button>
        </div>

        {/* Quick Stats */}
        <div className="relative mt-8 grid grid-cols-4 gap-4">
          {quickStats.map((stat) => (
            <div
              key={stat.label}
              className="flex items-center gap-3 rounded-lg border border-wenz-border bg-wenz-bg/50 px-4 py-3"
            >
              <stat.icon className={`h-4 w-4 ${stat.color}`} />
              <div>
                <p className="text-xs text-wenz-muted">{stat.label}</p>
                <p className="font-mono text-sm font-semibold text-wenz-text">
                  {stat.value}
                </p>
              </div>
            </div>
          ))}
        </div>
      </div>

      <div className="grid grid-cols-3 gap-6">
        {/* Recent Versions */}
        <div className="col-span-2 rounded-xl border border-wenz-border bg-wenz-card p-5">
          <h2 className="mb-4 text-sm font-semibold uppercase tracking-wider text-wenz-muted">
            Установленные версии
          </h2>
          <div className="space-y-2">
            {recentVersions.map((ver) => (
              <div
                key={ver.version}
                className="flex items-center justify-between rounded-lg border border-wenz-border bg-wenz-bg/30 px-4 py-3 transition-colors hover:border-wenz-accent/30"
              >
                <div className="flex items-center gap-3">
                  <div
                    className={`h-2 w-2 rounded-full ${
                      ver.active ? "bg-wenz-success" : "bg-wenz-muted/40"
                    }`}
                  />
                  <div>
                    <span className="font-mono text-sm font-semibold text-wenz-text">
                      {ver.version}
                    </span>
                    <span className="ml-2 text-xs text-wenz-muted">{ver.type}</span>
                  </div>
                </div>
                <div className="flex items-center gap-3">
                  <span className="text-xs text-wenz-muted">{ver.date}</span>
                  {ver.active ? (
                    <span className="rounded bg-wenz-accent/20 px-2 py-0.5 text-xs font-medium text-wenz-accent">
                      Активная
                    </span>
                  ) : (
                    <button className="rounded bg-white/5 px-2 py-0.5 text-xs font-medium text-wenz-muted transition-colors hover:bg-white/10 hover:text-wenz-text">
                      Выбрать
                    </button>
                  )}
                </div>
              </div>
            ))}
          </div>
          <button className="mt-3 flex w-full items-center justify-center gap-2 rounded-lg border border-dashed border-wenz-border py-2.5 text-xs text-wenz-muted transition-colors hover:border-wenz-accent/30 hover:text-wenz-accent">
            <Download className="h-3.5 w-3.5" />
            Скачать другую версию
          </button>
        </div>

        {/* Features */}
        <div className="space-y-3">
          <h2 className="mb-1 text-sm font-semibold uppercase tracking-wider text-wenz-muted">
            Возможности
          </h2>
          {features.map((feat) => (
            <div
              key={feat.title}
              className="rounded-xl border border-wenz-border bg-wenz-card p-4 transition-colors hover:border-wenz-accent/20"
            >
              <div className="flex items-center gap-2">
                <feat.icon className="h-4 w-4 text-wenz-accent" />
                <h3 className="text-sm font-semibold text-wenz-text">
                  {feat.title}
                </h3>
              </div>
              <p className="mt-1.5 text-xs leading-relaxed text-wenz-muted">
                {feat.desc}
              </p>
            </div>
          ))}
        </div>
      </div>
    </div>
  );
}
