import { Plus, Play, Settings, Trash2, FolderOpen, Clock } from "lucide-react";

interface Instance {
  id: string;
  name: string;
  version: string;
  modLoader: string;
  modsCount: number;
  lastPlayed: string;
  icon: string;
}

const instances: Instance[] = [
  {
    id: "1",
    name: "PvP Оптимизация",
    version: "1.8.9",
    modLoader: "Forge",
    modsCount: 12,
    lastPlayed: "2 часа назад",
    icon: "⚔️",
  },
  {
    id: "2",
    name: "Выживание 1.21",
    version: "1.21.4",
    modLoader: "Fabric",
    modsCount: 34,
    lastPlayed: "Вчера",
    icon: "🏕️",
  },
  {
    id: "3",
    name: "Техно-сборка",
    version: "1.20.1",
    modLoader: "Forge",
    modsCount: 156,
    lastPlayed: "3 дня назад",
    icon: "⚙️",
  },
  {
    id: "4",
    name: "SkyBlock",
    version: "1.21.4",
    modLoader: "Fabric",
    modsCount: 8,
    lastPlayed: "Неделю назад",
    icon: "🏝️",
  },
  {
    id: "5",
    name: "Vanilla+",
    version: "1.21.4",
    modLoader: "Нет",
    modsCount: 0,
    lastPlayed: "2 недели назад",
    icon: "🧊",
  },
];

export function InstancesPage() {
  return (
    <div className="animate-fade-in space-y-6">
      <div className="flex items-center justify-between">
        <div>
          <h1 className="text-2xl font-bold text-wenz-text">Сборки</h1>
          <p className="mt-1 text-sm text-wenz-muted">
            Управляй своими игровыми сборками
          </p>
        </div>
        <button className="flex items-center gap-2 rounded-lg bg-wenz-accent px-4 py-2.5 text-sm font-semibold text-wenz-bg transition-colors hover:bg-wenz-accent-hover">
          <Plus className="h-4 w-4" />
          Новая сборка
        </button>
      </div>

      <div className="grid grid-cols-2 gap-4 xl:grid-cols-3">
        {instances.map((inst) => (
          <div
            key={inst.id}
            className="group relative overflow-hidden rounded-xl border border-wenz-border bg-wenz-card transition-all hover:border-wenz-accent/30 hover:shadow-lg hover:shadow-wenz-accent/5"
          >
            {/* Header with icon */}
            <div className="flex items-start justify-between p-5">
              <div className="flex items-center gap-3">
                <div className="flex h-12 w-12 items-center justify-center rounded-lg bg-wenz-bg text-2xl">
                  {inst.icon}
                </div>
                <div>
                  <h3 className="font-semibold text-wenz-text">{inst.name}</h3>
                  <div className="mt-0.5 flex items-center gap-2">
                    <span className="font-mono text-xs text-wenz-accent">
                      {inst.version}
                    </span>
                    <span className="text-wenz-muted">·</span>
                    <span className="text-xs text-wenz-muted">{inst.modLoader}</span>
                  </div>
                </div>
              </div>
            </div>

            {/* Stats */}
            <div className="flex items-center gap-4 border-t border-wenz-border px-5 py-3">
              <div className="flex items-center gap-1.5 text-xs text-wenz-muted">
                <FolderOpen className="h-3 w-3" />
                {inst.modsCount} модов
              </div>
              <div className="flex items-center gap-1.5 text-xs text-wenz-muted">
                <Clock className="h-3 w-3" />
                {inst.lastPlayed}
              </div>
            </div>

            {/* Actions */}
            <div className="flex border-t border-wenz-border">
              <button className="flex flex-1 items-center justify-center gap-2 py-2.5 text-xs font-medium text-wenz-accent transition-colors hover:bg-wenz-accent/10">
                <Play className="h-3.5 w-3.5" fill="currentColor" />
                Играть
              </button>
              <div className="w-px bg-wenz-border" />
              <button className="flex flex-1 items-center justify-center gap-2 py-2.5 text-xs font-medium text-wenz-muted transition-colors hover:bg-white/5 hover:text-wenz-text">
                <Settings className="h-3.5 w-3.5" />
                Настроить
              </button>
              <div className="w-px bg-wenz-border" />
              <button className="flex flex-1 items-center justify-center gap-2 py-2.5 text-xs font-medium text-wenz-muted transition-colors hover:bg-red-500/10 hover:text-wenz-danger">
                <Trash2 className="h-3.5 w-3.5" />
                Удалить
              </button>
            </div>
          </div>
        ))}

        {/* Add New Card */}
        <button className="flex flex-col items-center justify-center gap-3 rounded-xl border-2 border-dashed border-wenz-border bg-wenz-card/50 p-8 transition-all hover:border-wenz-accent/40 hover:bg-wenz-card">
          <div className="flex h-12 w-12 items-center justify-center rounded-full bg-wenz-accent/10">
            <Plus className="h-6 w-6 text-wenz-accent" />
          </div>
          <span className="text-sm font-medium text-wenz-muted">
            Создать сборку
          </span>
        </button>
      </div>
    </div>
  );
}
